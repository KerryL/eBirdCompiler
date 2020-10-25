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
	static bool Parse(const std::string& html, ChecklistInfo& info);
	
private:
};

#endif// EBIRD_CHECKLIST_PARSER_H_
