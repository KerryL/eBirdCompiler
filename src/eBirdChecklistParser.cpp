// File:  eBirdChecklistParser.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Function for parsing checklist HTML data.

// Local headers
#include "eBirdChecklistParser.h"

#include <fstream>
bool EBirdChecklistParser::Parse(const std::string& html, ChecklistInfo& info)
{
	std::ofstream f("test.html");
	f << html;
	return true;
}
