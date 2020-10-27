// File:  eBirdCompiler.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Class for compiling summary data from separate eBird checklists.

// Local headers
#include "eBirdCompiler.h"
#include "eBirdChecklistParser.h"
#include "htmlRetriever.h"
#include "robotsParser.h"
#include "taxonomyOrder.h"

// Standard C++ headers
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <cassert>
#include <cmath>

const std::string EBirdCompiler::userAgent("eBird Compiler");
const std::string EBirdCompiler::taxonFileName("eBird_Taxonomy_v2019.csv");

bool EBirdCompiler::Update(const std::string& checklistString)
{
	errorString.clear();
	summary = SummaryInfo();
	
	std::set<std::string> urlList;
	std::string url;
	std::istringstream ss(checklistString);
	while (std::getline(ss, url))
		urlList.insert(url);
		
	if (urlList.empty())
	{
		errorString = "Failed to find any URLs";
		return false;
	}
	
	TaxonomyOrder taxonomicOrder;
	if (!taxonomicOrder.Parse(taxonFileName))
	{
		errorString = taxonomicOrder.GetErrorString();
		return false;
	}
	
	const auto baseURL(RobotsParser::GetBaseURL(*urlList.begin()));
	RobotsParser robotsTxtParser(userAgent, baseURL);
	std::chrono::steady_clock::duration crawlDelay;
	if (robotsTxtParser.RetrieveRobotsTxt())
		crawlDelay = robotsTxtParser.GetCrawlDelay();
	else
		crawlDelay = std::chrono::seconds(1);// Default value
	
	HTMLRetriever htmlClient(userAgent, crawlDelay);
	std::vector<ChecklistInfo> checklistInfo;
	for (const auto& u : urlList)
	{
		std::string html;
		if (!htmlClient.GetHTML(u, html))
		{
			errorString = "Failed to download checklist from " + u;
			return false;
		}
		
		checklistInfo.push_back(ChecklistInfo());
		EBirdChecklistParser parser(taxonomicOrder);
		if (!parser.Parse(html, checklistInfo.back()))
		{
			errorString = parser.GetErrorString();
			return false;
		}
	}

	std::set<std::string> locationSet;
	unsigned int anonUserCount(0);
	const auto dateCode(GetDateCode(checklistInfo.front()));
	bool allSameDate(true);
	for (const auto& ci : checklistInfo)
	{
		summary.totalDistance += ci.distance;
		summary.totalTime += ci.duration;
		
		if (GetDateCode(ci) != dateCode)
			allSameDate = false;
		
		for (const auto& b : ci.birders)
		{
			if (std::find(summary.participants.begin(), summary.participants.end(), b) == summary.participants.end())
				summary.participants.push_back(b);
		}
		
		if (std::find(ci.birders.begin(), ci.birders.end(), std::string("Anonymous eBirder")) != ci.birders.end())
			++anonUserCount;
			
		locationSet.insert(ci.location);
		
		for (const auto& checklistSpecies : ci.species)
		{
			bool found(false);
			for (auto& summarySpecies : summary.species)
			{
				if (summarySpecies.name == checklistSpecies.name)
				{
					summarySpecies.count += checklistSpecies.count;
					found = true;
					break;
				}
			}
			
			if (!found)
				summary.species.push_back(checklistSpecies);
		}
	}
	
	SortTaxonomically(summary.species);
	
	summary.includesMoreThanOneAnonymousUser = anonUserCount > 1;
	summary.locationCount = locationSet.size();
	
	if (!allSameDate)
		errorString = "Not all checklists are from the same date";

	// TODO:  If there are many checklists and only a small number are from a different date, identify those checklists.
	
	return true;
}

std::string EBirdCompiler::GetSummaryString() const
{
	unsigned int totalIndividuals(0);
	for (const auto& s : summary.species)
		totalIndividuals += s.count;
		
	const unsigned int timeHour(floor(summary.totalTime / 60.0));
	const unsigned int timeMin(summary.totalTime - timeHour * 60.0);
		
	std::ostringstream ss;
	ss.precision(1);
	ss << "\nParticipants:    " << summary.participants.size();
	if (summary.includesMoreThanOneAnonymousUser)
		ss << " (participant count may be inexact due to anonymous checklists)";
	ss << "\nTotal distance:  " << std::fixed << summary.totalDistance * 0.621371 << " miles"
		<< "\nTotal time:      ";
	if (timeHour > 0)
	{
		ss << timeHour << " hr";
		if (timeMin > 0)
			ss << ", " << timeMin << " min";
	}
	else
		ss << timeMin << " min";
		
	unsigned int speciesCount, otherTaxaCount;
	CountSpecies(summary.species, speciesCount, otherTaxaCount);
	ss<< "\n# Locations:     " << summary.locationCount
		<< "\n# Species:       " << speciesCount;
	if (otherTaxaCount > 0)
		ss << " (+ " << otherTaxaCount << " other taxa.)";
	ss << "\n# Individuals:   " << totalIndividuals << "\n\n";
	
	std::string::size_type maxNameLength(0);
	for (const auto& s : summary.species)
		maxNameLength = std::max(maxNameLength, s.name.length());
		
	const unsigned int extraSpace(7);// Extra space must be longer than max expected count size (TODO:  Automate this)
	ss << "Species list:\n";
	// TODO:  When we print the final list, should we remove subspecies info again?
	for (const auto& s : summary.species)
	{
		ss << "  " << s.name << std::setw(maxNameLength + extraSpace - s.name.length()) << std::setfill(' ');
		if (s.count == 0)
			ss << 'X';
		else
			ss << s.count;
		ss << '\n';
	}
	
	return ss.str();
}

unsigned int EBirdCompiler::GetDateCode(const ChecklistInfo& info)
{
	assert(info.month > 0 && info.month <= 12);
	assert(info.day > 0 && info.day <= 31);
	assert(info.year > 1700);
	return (info.year - 1700) + info.month * 1000 + info.day * 100000;
}

void EBirdCompiler::CountSpecies(const std::vector<SpeciesInfo>& species, unsigned int& speciesCount, unsigned int& otherTaxaCount)
{
	std::set<std::string> fullSpecies;
	std::set<std::string> otherTaxa;
	
	for (const auto& s : species)
	{
		const auto cleanedName(StripSubspecies(s.name));
		if (IsSpuhOrSlash(cleanedName))
			otherTaxa.insert(cleanedName);
		else
			fullSpecies.insert(cleanedName);
	}
	
	speciesCount = fullSpecies.size();
	otherTaxaCount = otherTaxa.size();
}

std::string EBirdCompiler::StripSubspecies(const std::string& name)
{
	const auto parenStart(name.find('('));
	if (parenStart == std::string::npos)
		return name;
		
	return name.substr(0, parenStart - 1);
}

bool EBirdCompiler::IsSpuhOrSlash(const std::string& name)
{
	const std::string spuh("sp.");
	if (name.find(spuh) != std::string::npos)
		return true;
	else if (name.find('/') != std::string::npos)
		return true;
		
	return false;
}

void EBirdCompiler::SortTaxonomically(std::vector<SpeciesInfo>& species)
{
	auto sortPredicate([](const SpeciesInfo& a, const SpeciesInfo& b)
	{
		return a.taxonomicOrder < b.taxonomicOrder;
	});
	
	std::sort(species.begin(), species.end(), sortPredicate);
}
