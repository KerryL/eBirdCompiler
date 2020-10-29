// File:  robotsParser.h
// Date:  7/19/2019
// Auth:  K. Loux
// Desc:  Object for parsing robots.txt files.

#ifndef ROBOTS_PARSER_H_
#define ROBOTS_PARSER_H_

// Standard C++ headers
#include <chrono>
#include <string>

// Local forward declarations
class HTMLRetriever;

class RobotsParser
{
public:
	RobotsParser(HTMLRetriever& htmlRetriever, const std::string& baseURL);
	bool RetrieveRobotsTxt();
	std::chrono::steady_clock::duration GetCrawlDelay() const;
	
	static std::string GetBaseURL(std::string url);

private:
	HTMLRetriever& htmlRetriever;
	const std::string& baseURL;

	std::string robotsTxt;

	static const std::string robotsFileName;

	static std::chrono::steady_clock::duration ExtractDelayValue(const std::string& line);
};

#endif// ROBOTS_PARSER_H_
