// File:  eBirdChecklistParser.h
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Function for parsing checklist HTML data.

#ifndef EBIRD_CHECKLIST_PARSER_H_
#define EBIRD_CHECKLIST_PARSER_H_

// Local headers
#include "eBirdCompiler.h"

// Standard C++ headers
#include <vector>
#include <string>

// Local forward declarations
class TaxonomyOrder;

struct ChecklistInfo
{
	std::vector<std::string> birders;
	std::string location;
	double distance;// [km]
	double duration;// [min]
	
	unsigned int day;
	unsigned int month;
	unsigned int year;
	
	std::vector<SpeciesInfo> species;
};

class EBirdChecklistParser
{
public:
	EBirdChecklistParser(TaxonomyOrder& taxonomy) : taxonomy(taxonomy) {}
	
	bool Parse(const std::string& html, ChecklistInfo& info);
	std::string GetErrorString() const { return errorString; }
	
private:
	std::string errorString;

	TaxonomyOrder& taxonomy;
	
	enum class Protocol
	{
		Traveling,
		Stationary,
		Incidential,
		Other
	};
	
	bool ExtractDate(const std::string& html, std::string::size_type& position, ChecklistInfo& info);
	bool ExtractLocation(const std::string& html, std::string::size_type& position, std::string& location);
	bool ExtractBirders(const std::string& html, std::string::size_type& position, std::vector<std::string>& birders);
	bool ExtractProtocol(const std::string& html, std::string::size_type& position, Protocol& protocol);
	bool ExtractDuration(const std::string& html, std::string::size_type& position, double& duration);
	bool ExtractDistance(const std::string& html, std::string::size_type& position, double& distance);
	bool ExtractSpeciesList(const std::string& html, std::string::size_type& position, std::vector<SpeciesInfo>& species);
	bool ExtractSpeciesInfo(const std::string& html, std::string::size_type& position, SpeciesInfo& info, const std::string::size_type& maxPosition);
	
	static bool ExtractTextBetweenTags(const std::string& html, const std::string& startTag, const std::string& endTag, std::string& token, std::string::size_type& position, const std::string::size_type& maxPosition = std::string::npos);
	static bool MoveToEndOfTag(const std::string& html, const std::string& tag, std::string::size_type& position, const std::string::size_type& maxPosition = std::string::npos);
	
	static std::vector<SpeciesInfo> MergeLists(const std::vector<std::vector<SpeciesInfo>>& lists);
};

#endif// EBIRD_CHECKLIST_PARSER_H_
