// File:  eBirdChecklistParser.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Function for parsing checklist HTML data.

// Local headers
#include "eBirdChecklistParser.h"

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
	const auto locationTagPosition(html.find(locationTag, position));
	if (locationTagPosition == std::string::npos)
		return false;
		
	position = locationTagPosition;
	const std::string spanTag("<span>");
	const std::string spanEndTag("</span>");
	return ExtractTextBetweenTags(html, spanTag, spanEndTag, location, position);
}

bool EBirdChecklistParser::ExtractBirders(const std::string& html, std::string::size_type& position, std::vector<std::string>& birders)
{
	const std::string ownerTag("<span class=\"is-visuallyHidden\">Owner</span>");
	const auto ownerTagPosition(html.find(ownerTag, position));
	if (ownerTagPosition == std::string::npos)
		return false;
		
	position = ownerTagPosition;
	const std::string spanTag("<span>");
	const std::string spanEndTag("</span>");
	std::string token;
	if (!ExtractTextBetweenTags(html, spanTag, spanEndTag, token, position))
		return false;
	birders.push_back(token);
	
	// Check to see if we have additional birders
	const std::string additionalBirdersTag("<span class=\"Heading-main is-visuallyHidden\">Other participating eBirders</span>");
	const auto additionalBirdersPosition(html.find(additionalBirdersTag, position));
	if (additionalBirdersPosition == std::string::npos)
		return true;// not an error
		
	const std::string divEndTag("</div>");
	auto divEndPosition(html.find(divEndTag, additionalBirdersPosition));
	if (divEndPosition == std::string::npos)
		return false;
	
	const std::string smallSpanTag("<span class=\"u-inline-xs\">");
	while (ExtractTextBetweenTags(html, spanTag, spanEndTag, token, position, divEndPosition))
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
	// in min
	// TODO:  Implement
	return true;
}

bool EBirdChecklistParser::ExtractDistance(const std::string& html, std::string::size_type& position, double& distance)
{
	// in km
	// TODO:  Implement
	return true;
}

bool EBirdChecklistParser::ExtractTextBetweenTags(const std::string& html, const std::string& startTag, const std::string& endTag, std::string& token, std::string::size_type& position, const std::string::size_type& maxPosition)
{
	const auto startPosition(html.find(startTag, position));
	if (startPosition == std::string::npos)
		return false;

	const auto endPosition(html.find(endTag, startPosition));
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
	const auto listStartPosition(html.find(listStartTag, position));
	if (listStartPosition == std::string::npos)
		return false;
		
	const std::string listEndTag("</main>");
	const auto listEndPosition(html.find(listEndTag, listStartPosition));
	if (listEndPosition == std::string::npos)
		return false;
		
	position = listStartPosition;
	SpeciesInfo info;
	while (ExtractSpeciesInfo(html, position, info, listEndPosition))
		species.push_back(info);
	
	return true;
}

bool EBirdChecklistParser::ExtractSpeciesInfo(const std::string& html, std::string::size_type& position, SpeciesInfo& info, const std::string::size_type& maxPosition)
{
	info.taxonomicOrder = 0;// TODO:  Implement taxonomic order
	// Go to "<section"
	// Extract between "<span class=\"Heading-main\"  >" and "</span>"
	// Go to "<span class=\"is-visuallyHidden\">Number observed: </span>"
	// Extract between "<span>" and "</span>"
	// Go to "</section>"
	
	return true;
}
