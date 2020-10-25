// File:  htmlRetriever.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Object for downloading HTML from the specified URL.

// Local headers
#include "htmlRetriever.h"

// Standard C++ headers
#include <iostream>
#include <cassert>

const bool HTMLRetriever::verbose(true);

HTMLRetriever::HTMLRetriever(const std::string& userAgent, const std::chrono::steady_clock::duration& crawlDelay) : userAgent(userAgent), rateLimiter(crawlDelay)
{
	DoGeneralCurlConfiguration();
}

HTMLRetriever::~HTMLRetriever()
{
	if (headerList)
		curl_slist_free_all(headerList);

	if (curl)
		curl_easy_cleanup(curl);
}
	
bool HTMLRetriever::GetHTML(const std::string& url, std::string& html)
{
	return DoCURLGet(url, html);
}

bool HTMLRetriever::DoGeneralCurlConfiguration()
{
	if (!curl)
		curl = curl_easy_init();

	if (!curl)
	{
		std::cerr << "Failed to initialize CURL" << std::endl;
		return false;
	}

	if (verbose)
		CURLCallHasError(curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L), "Failed to set verbose output");// Don't fail for this one

	/*if (!caCertificatePath.empty())
		curl_easy_setopt(curl, CURLOPT_CAPATH, caCertificatePath.c_str());*/

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL), "Failed to enable SSL"))
		return false;

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str()), "Failed to set user agent"))
		return false;

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L), "Failed to enable location following"))
		return false;

	headerList = curl_slist_append(headerList, "Connection: Keep-Alive");
	if (!headerList)
	{
		std::cerr << "Failed to append keep alive to header\n";
		return false;
	}

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList), "Failed to set header"))
		return false;

	/*if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.c_str()), _Failed to load the cookie file"))
		return false;

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.c_str()), "Failed to enable saving cookies"))
		return false;*/

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTMLRetriever::CURLWriteCallback), "Failed to set the write callback"))
		return false;

	return true;
}

bool HTMLRetriever::DoCURLGet(const std::string& url, std::string& response)
{
	assert(curl);

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response), "Failed to set write data"))
		return false;

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_POST, 0L), "Failed to set action to GET"))
		return false;

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()), "Failed to set URL"))
		return false;

	if (CURLCallHasError(curl_easy_perform(curl), "Failed issuing https GET"))
		return false;
	return true;
}

size_t HTMLRetriever::CURLWriteCallback(char *ptr, size_t size, size_t nmemb, void *userData)
{
	const size_t totalSize(size * nmemb);
	std::string& s(*static_cast<std::string*>(userData));
	s.append(ptr, totalSize);

	return totalSize;
}

bool HTMLRetriever::CURLCallHasError(const CURLcode& result, const std::string& message)
{
	if (result == CURLE_OK)
		return false;

	std::cerr << message << ":  " << curl_easy_strerror(result) << '\n';
	return true;
}
