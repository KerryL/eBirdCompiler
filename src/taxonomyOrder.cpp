// File:  taxonomyOrder.cpp
// Date:  10/26/2020
// Auth:  K. Loux
// Desc:  Object for looking up integers to represent eBird taxonomy order.
//        The taxonomy .csv file can be downloaded from:
//        https://www.birds.cornell.edu/clementschecklist/download/.

// Local headers
#include "taxonomyOrder.h"
#include "htmlRetriever.h"

// Standard C++ headers
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <cctype>
#include <iomanip>

#if defined(_MSC_VER) && _MSC_VER < 1914
#define filesystem experimental::filesystem
#endif

const std::string TaxonomyOrder::taxonomyFileURL("https://www.birds.cornell.edu/clementschecklist/wp-content/uploads/2019/08/eBird_Taxonomy_v2019.csv");

bool TaxonomyOrder::Parse(const std::string& fileName)
{
	if (!std::filesystem::exists(fileName))
	{
		if (!DownloadTaxonomyFile(fileName))
		{
			errorString = "Failed to download the taxonomy file";
			return false;
		}
	}
	
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
	if (!GetNextToken(ss, token) || !ParseToken(token, info.sequence))
		return false;
		
	if (!GetNextToken(ss, token) || !ParseToken(token, info.category))
		return false;
		
	if (!GetNextToken(ss, token) || !ParseToken(token, info.speciesCode))
		return false;
		
	if (!GetNextToken(ss, token) || !ParseToken(token, info.commonName))
		return false;

	if (!GetNextToken(ss, token) || !ParseToken(token, info.scientificName))
		return false;
		
	if (!GetNextToken(ss, token) || !ParseToken(token, info.order))
		return false;
		
	if (!GetNextToken(ss, token) || !ParseToken(token, info.family))
		return false;
		
	if (!GetNextToken(ss, token) || !ParseToken(token, info.speciesGroup))
		return false;
		
	GetNextToken(ss, token);// Don't return if the last one fails - it just means the token is blank
	if (!ParseToken(token, info.reportAs))
		return false;
		
	return true;
}

bool TaxonomyOrder::GetNextToken(std::istringstream& ss, std::string& token)
{
	ss >> std::ws;// Discard leading whitespace
	if (ss.peek() == '"')// Process quoted string
	{
		ss >> std::quoted(token);
	}
	
	return !std::getline(ss, token, ',').fail();
}

bool TaxonomyOrder::HeaderMatches(std::string& headerLine)
{
	// For some reason, the eBird taxonomy file starts with three negative-valued characters, which seem to be ignored by text editors.
	// We'll ignore them here, too, along with any whitespace characters (this works to remove trailing whitespace, like \r, which is
	// included in the line, but we can only use it here because we know the expected line doesn't contain any whitespace).
	headerLine.erase(std::remove_if(headerLine.begin(), headerLine.end(), [](const std::string::value_type& c)
	{
		if (static_cast<int>(c) < 0 || std::isspace(c))
			return true;
		return false;
	}), headerLine.end());
	
	const std::string expectedHeader("TAXON_ORDER,CATEGORY,SPECIES_CODE,PRIMARY_COM_NAME,SCI_NAME,ORDER1,FAMILY,SPECIES_GROUP,REPORT_AS");
	return headerLine == expectedHeader;
}

bool TaxonomyOrder::ParseToken(const std::string& s, std::string& value)
{
	value = s;
	return true;
}

void TaxonomyOrder::Trim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const unsigned char& ch)
	{
		return !std::isspace(ch);
	}));
	
	s.erase(std::find_if(s.rbegin(), s.rend(), [](const unsigned int &ch)
	{
		return !std::isspace(ch);
	}).base(), s.end());
}

bool TaxonomyOrder::ParseToken(const std::string& s, TaxaInfo::Category& value)
{
	if (s == "species")
		value = TaxaInfo::Category::Species;
	else if (s == "hybrid")
		value = TaxaInfo::Category::Hybrid;
	else if (s == "spuh")
		value = TaxaInfo::Category::Spuh;
	else if (s == "slash")
		value = TaxaInfo::Category::Slash;
	else if (s == "issf")
		value = TaxaInfo::Category::IdentifiableSubSpecificGroup;
	else if (s == "intergrade")
		value = TaxaInfo::Category::Intergrade;
	else if (s == "domestic")
		value = TaxaInfo::Category::Domestic;
	else if (s == "form")
		value = TaxaInfo::Category::Form;
	else
		return false;

	return true;
}

bool TaxonomyOrder::DownloadTaxonomyFile(const std::string& saveTo)
{
	HTMLRetriever retriever(userAgent, std::chrono::steady_clock::duration(0));
	std::string fileContents;
	if (!retriever.GetHTML(taxonomyFileURL, fileContents))
		return false;

	std::ofstream file(saveTo);
	if (!file.good())
		return false;
		
	file << fileContents;

	return true;
}
