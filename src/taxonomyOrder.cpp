// File:  taxonomyOrder.cpp
// Date:  10/26/2020
// Auth:  K. Loux
// Desc:  Object for looking up integers to represent eBird taxonomy order.
//        The taxonomy .csv file can be downloaded from:
//        https://www.birds.cornell.edu/clementschecklist/download/.

// Local headers
#include "taxonomyOrder.h"

// Standard C++ headers
#include <fstream>
#include <algorithm>

bool TaxonomyOrder::Parse(const std::string& fileName)
{
	std::ifstream file(fileName);
	if (!file.good())
	{
		errorString = "Failed to open file at '" + fileName + "'";
		return false;
	}
		
	std::string line;
	if (!std::getline(file, line))
	{
		errorString = "Failed to read taxonomy file header line";
		return false;
	}
	else if (!HeaderMatches(line))
	{
		errorString = "Unexpected taxonomy file header format";
		return false;
	}

	while (std::getline(file, line))
	{
		TaxaInfo info;
		if (!ParseLine(line, info))
		{
			errorString = "Failed to parse taxonomy file";
			return false;
		}
			
		taxaInfo.push_back(info);
	}
	
	return true;
}
	
bool TaxonomyOrder::GetTaxonomicSequence(const std::string& commonName, unsigned int& sequence) const
{
	for (const auto& t : taxaInfo)
	{
		if (t.commonName != commonName)
			continue;
		sequence = t.sequence;
		return true;
	}
	
	return false;
}

bool TaxonomyOrder::ParseLine(std::string line, TaxaInfo& info)
{
	Trim(line);
	std::istringstream ss(line);
	std::string token;
	if (!std::getline(ss, token, ',') || !ParseToken(token, info.sequence))
		return false;
		
	if (!std::getline(ss, token, ',') || !ParseToken(token, info.category))
		return false;
		
	if (!std::getline(ss, token, ',') || !ParseToken(token, info.speciesCode))
		return false;
		
	if (!std::getline(ss, token, ',') || !ParseToken(token, info.commonName))
		return false;

	// TODO:  Implement the rest of this - species group can have commas, so need to also parse the quotes
	/*if (!std::getline(ss, token, ',') || !ParseToken(token, info.scientificName))
		return false;
		
	if (!std::getline(ss, token, ',') || !ParseToken(token, info.order))
		return false;
		
	if (!std::getline(ss, token, ',') || !ParseToken(token, info.family))
		return false;
		
	if (!std::getline(ss, token, ',') || !ParseToken(token, info.speciesGroup))
		return false;
		
	if (!std::getline(ss, token, ',') || !ParseToken(token, info.reportAs))
		return false;*/
		
	return true;
}

bool TaxonomyOrder::HeaderMatches(const std::string& headerLine)
{
	std::string trimmedHeader(headerLine);
	Trim(trimmedHeader);
	trimmedHeader = trimmedHeader.substr(3);// TODO:  Fix this - need a real solution
	// This didn't seem helpful, but I wonder if it's on the right track:
	// //#include <codecvt>std::locale::global(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	const std::string expectedHeader("TAXON_ORDER,CATEGORY,SPECIES_CODE,PRIMARY_COM_NAME,SCI_NAME,ORDER1,FAMILY,SPECIES_GROUP,REPORT_AS");
	return trimmedHeader == expectedHeader;
}

void TaxonomyOrder::Trim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }).base(), s.end());
}

bool TaxonomyOrder::ParseToken(const std::string& s, std::string& value)
{
	value = s;
	return true;
}
