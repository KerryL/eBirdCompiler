// File:  robotsParser.cpp
// Date:  7/19/2019
// Auth:  K. Loux
// Desc:  Object for parsing robots.txt files.

// Local headers
#include "robotsParser.h"
#include "htmlRetriever.h"

// Standard C++ headers
#include <sstream>

const std::string RobotsParser::robotsFileName("robots.txt");

// TODO:  This is realy basic - it assumes we're only interested in the crawl delay.  We should fix this...
RobotsParser::RobotsParser(HTMLRetriever& htmlRetriever, const std::string& baseURL)
	: htmlRetriever(htmlRetriever), baseURL(baseURL)
{
}

std::string RobotsParser::GetBaseURL(std::string url)
{
	std::string::size_type start(0);
	const std::string http("http");
	if (url.substr(0, http.length()) == http)
	{
		start = url.find("//");
		if (start == std::string::npos)
			return std::string();// TODO:  Something better?
	}
	
	const std::string::size_type slash(url.find(std::string("/"), start + 2));
	if (slash == std::string::npos)
		return url;
	return url.substr(0, slash);
}

bool RobotsParser::RetrieveRobotsTxt()
{
	std::string fullURL(baseURL);
	if (fullURL.back() != '/')
		fullURL.append("/");

	if (!htmlRetriever.GetHTML(fullURL + robotsFileName, robotsTxt))
		return false;
	return true;
}

std::chrono::steady_clock::duration RobotsParser::GetCrawlDelay() const
{
	using namespace std::chrono_literals;
	std::chrono::steady_clock::duration crawlDelay = 0s;

	std::string line;
	std::istringstream ss(robotsTxt);
	bool theseRulesApply(false);
	while (std::getline(ss, line))
	{
		const std::string userAgentTag("User-agent:");
		const std::string crawlDelayTag("Crawl-delay:");

		if (line.empty())
			continue;
		else if (line.find(userAgentTag) != std::string::npos)
		{
			if (line.find(htmlRetriever.GetUserAgent()) != std::string::npos ||
				line.find("*") != std::string::npos)
				theseRulesApply = true;
			else
				theseRulesApply = false;
		}
		else if (theseRulesApply && line.find(crawlDelayTag) != std::string::npos)
		{
			const auto delay(ExtractDelayValue(line));
			if (delay > crawlDelay)
				crawlDelay = delay;
		}
	}

	return crawlDelay;
}

std::chrono::steady_clock::duration RobotsParser::ExtractDelayValue(const std::string& line)
{
	const auto colon(line.find(":"));
	if (colon == std::string::npos)
		return std::chrono::steady_clock::duration();

	std::istringstream ss(line.substr(colon + 1));
	unsigned int seconds;
	if ((ss >> seconds).fail())
		return std::chrono::steady_clock::duration();

	return std::chrono::seconds(seconds);
}
