// File:  eBirdCompiler.h
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Class for compiling summary data from separate eBird checklists.

#ifndef EBIRD_COMPILER_H_
#define EBIRD_COMPILER_H_

// Standard C++ headers
#include <string>
#include <vector>

// Local forward declarations
struct ChecklistInfo;

struct SpeciesInfo
{
	std::string name;
	unsigned int count;
	unsigned int taxonomicOrder;
};

class EBirdCompiler
{
public:
	bool Update(const std::string& checklistString);
	
	std::string GetErrorString() const { return errorString; }
	std::string GetSummaryString() const;

private:
	static const std::string userAgent;

	std::string errorString;
	std::vector<std::string> checklistURLs;
	
	struct SummaryInfo
	{
		std::vector<std::string> participants;
		bool includesMoreThanOneAnonymousUser = false;
		double totalDistance = 0.0;// [km]
		double totalTime = 0.0;// [min]
		unsigned int locationCount = 0;
		
		std::vector<SpeciesInfo> species;
	};
	
	SummaryInfo summary;
	
	static unsigned int GetDateCode(const ChecklistInfo& info);
};

#endif// EBIRD_COMPILER_H_
