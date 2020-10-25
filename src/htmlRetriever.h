// File:  htmlRetriever.h
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Object for downloading HTML from the specified URL.

#ifndef HTML_RETRIEVER_H_
#define HTML_RETRIEVER_H_

// Local headers
#include "throttledSection.h"

// cURL headers
#include <curl/curl.h>

// Standard C++ headers
#include <string>
#include <chrono>

class HTMLRetriever
{
public:
	HTMLRetriever(const std::string& userAgent, const std::chrono::steady_clock::duration& crawlDelay);
	~HTMLRetriever();
	
	bool GetHTML(const std::string& url, std::string& html);

protected:
	const std::string userAgent;
	static const bool verbose;
	
	ThrottledSection rateLimiter;
	
	CURL* curl = nullptr;
	struct curl_slist* headerList = nullptr;
	bool DoGeneralCurlConfiguration();
	bool DoCURLGet(const std::string& url, std::string& response);
	static size_t CURLWriteCallback(char *ptr, size_t size, size_t nmemb, void *userData);
	static bool CURLCallHasError(const CURLcode& result, const std::string& message);
};

#endif// HTML_RETRIEVER_H_
