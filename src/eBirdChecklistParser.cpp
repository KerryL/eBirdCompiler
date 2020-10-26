// File:  eBirdChecklistParser.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Function for parsing checklist HTML data.

// Local headers
#include "eBirdChecklistParser.h"
#include "taxonomyOrder.h"

// Standard C++ headers
#include <sstream>

bool EBirdChecklistParser::Parse(const std::string& html, ChecklistInfo& info)
{
	std::string::size_type position(0);
	if (!ExtractDate(html, position, info))
	{
		errorString = "Failed to find date";
		return false;
	}
	
	if (!ExtractLocation(html, position, info.location))
	{
		errorString = "Failed to find location";
		return false;
	}
	
	if (!ExtractBirders(html, position, info.birders))
	{
		errorString = "Failed to find birder names";
		return false;
	}
	
	Protocol protocol;
	if (!ExtractProtocol(html, position, protocol))
	{
		errorString = "Failed to find protocol";
		return false;
	}
	
	// TODO:  Consider also getting the "number of birders" field?  Currently we count only birders who have checklist shared with them...

	if (protocol == Protocol::Traveling || protocol == Protocol::Stationary)
	{
		if (!ExtractDuration(html, position, info.duration))
		{
			errorString = "Failed to find duration";
			return false;
		}
	}
	else
		info.duration = 0.0;
	
	if (protocol == Protocol::Traveling)
	{
		if (!ExtractDistance(html, position, info.distance))
		{
			errorString = "Failed to find distance";
			return false;
		}
	}
	else
		info.distance = 0.0;
	
	if (!ExtractSpeciesList(html, position, info.species))
	{
		errorString = "Failed to find species list";
		return false;
	}

	return true;
}

bool EBirdChecklistParser::ExtractDate(const std::string& html, std::string::size_type& position, ChecklistInfo& info)
{
	const std::string dateTagStart("<time datetime=\"");
	const std::string tagEnd("\">");
	std::string token;
	if (!ExtractTextBetweenTags(html, dateTagStart, tagEnd, token, position))
		return false;
		
	std::istringstream ss(token);
	bool failed((ss >> info.year).fail());
	ss.ignore();
	failed = failed || (ss >> info.month).fail();
	ss.ignore();
	failed = failed || (ss >> info.day).fail();

	return !failed;
}

bool EBirdChecklistParser::ExtractLocation(const std::string& html, std::string::size_type& position, std::string& location)
{
	const std::string locationTag("<h6 class=\"is-visuallyHidden\">Location</h6>");
	if (!MoveToEndOfTag(html, locationTag, position))
		return false;

	const std::string spanTag("<span>");
	const std::string spanEndTag("</span>");
	return ExtractTextBetweenTags(html, spanTag, spanEndTag, location, position);
}

bool EBirdChecklistParser::ExtractBirders(const std::string& html, std::string::size_type& position, std::vector<std::string>& birders)
{
	const std::string ownerTag("<span class=\"is-visuallyHidden\">Owner</span>");
	if (!MoveToEndOfTag(html, ownerTag, position))
		return false;

	const std::string spanTag("<span>");
	const std::string spanEndTag("</span>");
	std::string token;
	if (!ExtractTextBetweenTags(html, spanTag, spanEndTag, token, position))
		return false;
	birders.push_back(token);
	
	// Check to see if we have additional birders
	const std::string additionalBirdersTag("<span class=\"Heading-main is-visuallyHidden\">Other participating eBirders</span>");
	if (!MoveToEndOfTag(html, additionalBirdersTag, position))
		return true;// Not an error
		
	const std::string breadcrumbsTag("<div class=\"Breadcrumbs Breadcrumbs--small Breadcrumbs--comma\">");
	if (!MoveToEndOfTag(html, breadcrumbsTag, position))
		return false;
		
	const std::string divEndTag("</div>");
	auto divEndPosition(html.find(divEndTag, position));
	if (divEndPosition == std::string::npos)
		return false;

	const std::string smallSpanTag("<span class=\"u-inline-xs\">");
	while (ExtractTextBetweenTags(html, smallSpanTag, spanEndTag, token, position, divEndPosition))
		birders.push_back(token);
	
	return true;
}

bool EBirdChecklistParser::ExtractProtocol(const std::string& html, std::string::size_type& position, Protocol& protocol)
{
	const std::string protocolStartTag("<span class=\"Heading-main u-inline-sm\" title=\"Protocol: ");
	const std::string endTag("\">");
	std::string token;
	if (!ExtractTextBetweenTags(html, protocolStartTag, endTag, token, position))
		return false;
		
	if (token == "Traveling")
		protocol = Protocol::Traveling;
	else if (token == "Stationary")
		protocol = Protocol::Stationary;
	else if (token == "Incidential")
		protocol = Protocol::Incidential;
	else
		protocol = Protocol::Other;
		
	return true;
}

bool EBirdChecklistParser::ExtractDuration(const std::string& html, std::string::size_type& position, double& duration)
{
	const std::string durationStartTag("<span class=\"Badge Badge--plain Badge--icon\" title=\"Duration: ");
	const std::string durationEndTag("\"");
	std::string token;
	if (!ExtractTextBetweenTags(html, durationStartTag, durationEndTag, token, position))
		return false;
		
	double value;
	std::istringstream ss(token);
	if ((ss >> value).fail())
		return false;
	ss.ignore();

	if (ss.peek() == 'h')
	{
		duration = value * 60;
		std::string::size_type temp(0);
		if (MoveToEndOfTag(token, ", ", temp))
		{
			ss.str(token.substr(temp));
			if ((ss >> value).fail())
				return false;
			duration += value;
		}
	}
	else if (ss.peek() == 'm')
		duration = value;
	else
		return false;// TODO:  Better messaging?

	return true;
}

bool EBirdChecklistParser::ExtractDistance(const std::string& html, std::string::size_type& position, double& distance)
{
	const std::string distanceStartTag("<span class=\"Badge Badge--plain Badge--icon\" title=\"Distance: ");
	const std::string distanceEndTag("\"");
	std::string token;
	if (!ExtractTextBetweenTags(html, distanceStartTag, distanceEndTag, token, position))
		return false;

	double value;
	std::istringstream ss(token);
	if ((ss >> value).fail())
		return false;
	ss.ignore();
	
	if (ss.peek() == 'm')
		distance = value * 1.609344;
	else if (ss.peek() == 'k')
		distance = value;
	else
		return false;// TODO:  Better messaging?

	return true;
}

bool EBirdChecklistParser::ExtractTextBetweenTags(const std::string& html, const std::string& startTag, const std::string& endTag, std::string& token, std::string::size_type& position, const std::string::size_type& maxPosition)
{
	const auto startPosition(html.find(startTag, position));
	if (startPosition == std::string::npos)
		return false;

	const auto endPosition(html.find(endTag, startPosition + startTag.length()));
	if (endPosition == std::string::npos)
		return false;
		
	if (endPosition > maxPosition)
		return false;
		
	token = html.substr(startPosition + startTag.length(), endPosition - startPosition - startTag.length());
	position = endPosition + endTag.length();
	return true;
}

bool EBirdChecklistParser::ExtractSpeciesList(const std::string& html, std::string::size_type& position, std::vector<SpeciesInfo>& species)
{
	const std::string listStartTag("<main id=\"list\">");
	if (!MoveToEndOfTag(html, listStartTag, position))
		return false;

	// TODO:  Would be good to have a check for the same event being entered as multiple checklists (i.e. participant A + particpant B)
		
	const std::string listEndTag("</main>");
	const auto listEndPosition(html.find(listEndTag, position));
	if (listEndPosition == std::string::npos)
		return false;
		
	std::vector<std::vector<SpeciesInfo>> lists;
	const std::string additionalSpeciesTag("<h5 class=\"Heading Heading--h5 Heading--minor\" data-observationheading>Additional species");
	std::string::size_type nextListStart;

	do
	{
		nextListStart = html.find(additionalSpeciesTag, position);
		lists.push_back(std::vector<SpeciesInfo>());
		SpeciesInfo info;
		while (ExtractSpeciesInfo(html, position, info, std::min(nextListStart, listEndPosition)))
			lists.back().push_back(info);
			
		if (nextListStart != std::string::npos)
			position = nextListStart + additionalSpeciesTag.length();
	} while (nextListStart < listEndPosition);
	
	species = MergeLists(lists);
	
	return true;
}

bool EBirdChecklistParser::ExtractSpeciesInfo(const std::string& html, std::string::size_type& position, SpeciesInfo& info, const std::string::size_type& maxPosition)
{
	const std::string sectionStartTag("<section");
	if (!MoveToEndOfTag(html, sectionStartTag, position, maxPosition))
		return false;

	const std::string speciesNameStartTag("<span class=\"Heading-main\" ");
	if (!MoveToEndOfTag(html, speciesNameStartTag, position, maxPosition))
		return false;
		
	const std::string nameStartTag(">");
	const std::string spanEndTag("</span>");
	if (!ExtractTextBetweenTags(html, nameStartTag, spanEndTag, info.name, position, maxPosition))
		return false;
		
	if (!taxonomy.GetTaxonomicSequence(info.name, info.taxonomicOrder))
		return false;
		
	const std::string countStartTag("<span class=\"is-visuallyHidden\">Number observed: </span>");
	if (!MoveToEndOfTag(html, countStartTag, position, maxPosition))
		return false;

	const std::string spanStartTag("<span>");
	std::string countToken;
	if (!ExtractTextBetweenTags(html, spanStartTag, spanEndTag, countToken, position, maxPosition))
		return false;
	if (countToken == "X")
		info.count = 0;
	else
	{
		std::istringstream ss(countToken);
		if ((ss >> info.count).fail())
			return false;
	}
	
	const std::string sectionEndTag("</section>");
	if (!MoveToEndOfTag(html, sectionEndTag, position, maxPosition))
		return false;
	
	return true;
}

bool EBirdChecklistParser::MoveToEndOfTag(const std::string& html, const std::string& tag, std::string::size_type& position, const std::string::size_type& maxPosition)
{
	const auto tagPosition(html.find(tag, position));
	if (tagPosition == std::string::npos)
		return false;

	if (tagPosition + tag.length() > maxPosition)
		return false;
		
	position = tagPosition + tag.length();
	return true;
}

std::vector<SpeciesInfo> EBirdChecklistParser::MergeLists(const std::vector<std::vector<SpeciesInfo>>& lists)
{
	std::vector<SpeciesInfo> mergedList(lists.front());
	for (unsigned int i = 1; i < lists.size(); ++i)
	{
		for (const auto& s : lists[i])
		{
			bool found(false);
			for (auto& m : mergedList)
			{
				if (s.name == m.name)
				{
					// NOTE:  If a pair of shared checklists both contian an entry for a species, but have
					// different counts, only the count for the checklist whose link is being viewed is shown
					// (i.e. no additional counts are shown in "additional species" lists at the bottom of the page).
					m.count = std::max(m.count, s.count);
					found = true;
					break;
				}
			}
			
			if (!found)
				mergedList.push_back(s);
		}
	}
	
	return mergedList;
}
