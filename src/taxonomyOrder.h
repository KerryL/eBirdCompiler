// File:  taxonomyOrder.h
// Date:  10/26/2020
// Auth:  K. Loux
// Desc:  Object for looking up integers to represent eBird taxonomy order.
//        The taxonomy .csv file can be downloaded from:
//        https://www.birds.cornell.edu/clementschecklist/download/.

#ifndef TAXONOMY_ORDER_H_
#define TAXONOMY_ORDER_H_

// Standard C++ headers
#include <string>
#include <vector>
#include <sstream>

class TaxonomyOrder
{
public:
	explicit TaxonomyOrder(const std::string& userAgent) : userAgent(userAgent) {}
	bool Parse(const std::string& fileName);
	
	bool GetTaxonomicSequence(const std::string& commonName, unsigned int& sequence) const;
	
	std::string GetErrorString() const { return errorString; }

private:
	static const std::string taxonomyFileURL;
	const std::string userAgent;
	
	std::string errorString;

	struct TaxaInfo
	{
		unsigned int sequence;
		std::string category;// TODO:  Make enum
		std::string speciesCode;
		std::string commonName;
		std::string scientificName;
		std::string order;
		std::string family;
		std::string speciesGroup;
		std::string reportAs;
	};
	
	std::vector<TaxaInfo> taxaInfo;
	
	bool ParseLine(std::string line, TaxaInfo& info);
	bool HeaderMatches(const std::string& headerLine);
	
	template <typename T>
	bool ParseToken(const std::string& s, T& value);
	bool ParseToken(const std::string& s, std::string& value);
	
	static void Trim(std::string& s);
	
	bool DownloadTaxonomyFile(const std::string& saveTo);
};

template <typename T>
bool TaxonomyOrder::ParseToken(const std::string& s, T& value)
{
	// Fields are allowed to be blank
	if (s.empty())
		return true;
		
	std::istringstream ss(s);
	return !(ss >> value).fail();
}

#endif// TAXONOMY_ORDER_H_
