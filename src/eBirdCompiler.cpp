// File:  eBirdCompiler.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Class for compiling summary data from separate eBird checklists.

// Local headers
#include "eBirdCompiler.h"
#include "eBirdChecklistParser.h"
#include "htmlRetriever.h"
#include "robotsParser.h"

// Standard C++ headers
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>

const std::string EBirdCompiler::userAgent("eBird Compiler");

bool EBirdCompiler::Update(const std::string& checklistString)
{
	errorString.clear();
	summary = SummaryInfo();
	
	std::vector<std::string> urlList;
	std::string url;
	std::istringstream ss(checklistString);
	while (std::getline(ss, url))
		urlList.push_back(url);
		
	if (urlList.empty())
	{
		errorString = "Failed to find any URLs";
		return false;
	}
	
	const auto baseURL(RobotsParser::GetBaseURL(urlList.front()));
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
		if (!EBirdChecklistParser::Parse(html, checklistInfo.back()))
		{
			// TODO:  Set error string to something more meaningful
			errorString = "Failed to parse checklist";
			return false;
		}
	}

	std::set<std::string> locationSet;
	unsigned int anonUserCount(0);
	for (const auto& ci : checklistInfo)
	{
		summary.totalDistance += ci.distance;
		summary.totalTime += ci.duration;
		
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
	
	summary.includesMoreThanOneAnonymousUser = anonUserCount > 1;
	summary.locationCount = locationSet.size();
	
	// Need to decide if we should store info from each checklist separately, so we only need to re-parse pages for which URLs were added (and remove stored data for URLs that were removed), or if we should re-parse everything each time.  What happens if the URL didn't change but the checklist was updated?
	// Should add a warning if all checklists are not from same date.  If there are many checklists and only a small number are from a different date, identify those checklists.
	// Should this be threaded?  Need to have rate limiter and robots.txt reader (with or without threading)?
	// TODO:  Warn if dates don't match
	
	return true;
}

std::string EBirdCompiler::GetSummaryString() const
{
	unsigned int totalIndividuals(0);
	for (const auto& s : summary.species)
		totalIndividuals += s.count;
		
	std::ostringstream ss;
	ss << "Summary of observations:"
		<< "\n  Participants:     " << summary.participants.size();
	if (summary.includesMoreThanOneAnonymousUser)
		ss << " (participant count may be inexact due to anonymous checklists)";
	ss << "\n  Total distance:  " << summary.totalDistance * 0.621371 << " miles"
		<< "\n  Total time:      " << summary.totalTime << " min"
		<< "\n  # Locations:     " << summary.locationCount
		<< "\n  # Species:       " << summary.species.size()
		<< "\n  # Individuals:   " << totalIndividuals << "\n\n";
		
	// TODO:  Sort list by taxonmic order
	
	std::string::size_type maxNameLength(0);
	for (const auto& s : summary.species)
		maxNameLength = std::max(maxNameLength, s.name.length());
		
	ss << "  Species list:\n";
	for (const auto& s : summary.species)
		ss << "    " << s.name << std::setw(maxNameLength + 2 - s.name.length()) << std::setfill(' ') << s.count << '\n';
	
	return ss.str();
}
