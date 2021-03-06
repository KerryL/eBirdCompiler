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
#include <map>

const std::string EBirdCompiler::userAgent("eBird Compiler");
const std::string EBirdCompiler::taxonFileName("eBird_Taxonomy_v2019.csv");

#include <iostream>
bool EBirdCompiler::Update(const std::string& checklistString)
{
	errorString.clear();
	summary = SummaryInfo();
	
	std::set<std::string> urlList;// Use set to avoid duplicates
	std::string url;
	std::istringstream ss(checklistString);
	while (ss >> url)
	{
		if (url.find("ebird.org/") == std::string::npos && url.front() == 'S')// Allow checkilist IDs to be used and generate full URL automatically
			urlList.insert("https://ebird.org/checklist/" + url);
		else if (!url.empty())
			urlList.insert(url);
	}
		
	if (urlList.empty())
	{
		errorString = "Failed to find any URLs";
		return false;
	}
	
	TaxonomyOrder taxonomicOrder(userAgent);
	if (!taxonomicOrder.Parse(taxonFileName))
	{
		errorString = taxonomicOrder.GetErrorString();
		return false;
	}
	
	HTMLRetriever htmlClient(userAgent);
	
	const auto baseURL(RobotsParser::GetBaseURL(*urlList.begin()));
	RobotsParser robotsTxtParser(htmlClient, baseURL);
	std::chrono::steady_clock::duration crawlDelay;
	if (robotsTxtParser.RetrieveRobotsTxt())
		crawlDelay = robotsTxtParser.GetCrawlDelay();
	else
		crawlDelay = std::chrono::seconds(1);// Default value
	
	htmlClient.SetCrawlDelay(crawlDelay);
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
	std::map<unsigned int, std::vector<std::string>> checklistsByDateCode;
	for (const auto& ci : checklistInfo)
	{
		summary.totalDistance += ci.distance;
		summary.totalTime += ci.duration;
		
		const auto dateCode(GetDateCode(ci));
		checklistsByDateCode[dateCode].push_back(ci.identifier);
		
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

	RemoveSubspeciesFromSummary();
	SortTaxonomically(summary.species);
	
	summary.includesMoreThanOneAnonymousUser = anonUserCount > 1;
	summary.locationCount = locationSet.size();
	
	if (checklistsByDateCode.size() > 1)
	{
		// Try to be helpful about reporting these potential errors:
		// - If there is a date code that includes > 80% of the checklists, identify the checklists that make up the 20%
		// - Otherwise, report the number of checklists given for each date
		for (const auto& cl : checklistsByDateCode)
		{
			if (cl.second.size() > 0.8 * checklistInfo.size())
			{
				std::ostringstream ss;
				ss << "The following checklists are not from the same date as the others:\n";
				for (const auto& cl2 : checklistsByDateCode)
				{
					if (cl2.first != cl.first)
					{
						for (const auto &id : cl2.second)
							ss << id << '\n';
					}
				}
				
				errorString = ss.str();
				break;
			}
		}
		
		if (errorString.empty())
		{
			std::ostringstream ss;
			ss << "Not all checklists are from the same date:\n";
			for (const auto& cl : checklistsByDateCode)
				ss << GetDateFromCode(cl.first) << " - " << cl.second.size() << " checklists\n";
			errorString = ss.str();
		}
	}
	
	return true;
}

std::string EBirdCompiler::GetSummaryString() const
{
	unsigned int totalIndividuals(0);
	for (const auto& s : summary.species)
		totalIndividuals += s.count;
		
	const unsigned int timeHour(static_cast<unsigned int>(floor(summary.totalTime / 60.0)));
	const unsigned int timeMin(static_cast<unsigned int>(summary.totalTime - timeHour * 60.0));
		
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
		
	const unsigned int extraSpace([this]()
	{
		unsigned int maxLength(0);
		for (const auto& s : summary.species)
		{
			const unsigned int length(log10(s.count) + 1);
			if (length > maxLength)
				maxLength = length;
		}
		return maxLength + 3;// Three extra spaces to make it look nice
	}());
	
	ss << "Species list:\n";
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

std::string EBirdCompiler::GetDateFromCode(const unsigned int& code)
{
	const unsigned int day(code / 100000);
	const unsigned int month((code - day * 100000) / 1000);
	const unsigned int year(code - day * 100000 - month * 1000 + 1700);
	std::ostringstream ss;
	ss << month << '/' << day << '/' << year;
	return ss.str();
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

void EBirdCompiler::RemoveSubspeciesFromSummary()
{
	for (auto& s : summary.species)
		s.name = StripSubspecies(s.name);
		
	for (unsigned int i = 0; i < summary.species.size(); ++i)
	{
		for (unsigned int j = i + 1; j < summary.species.size(); ++j)
		{
			if (summary.species[i].name == summary.species[j].name)
			{
				summary.species[i].count += summary.species[j].count;
				summary.species.erase(summary.species.begin() + j);
				--j;
			}
		}
	}
}
