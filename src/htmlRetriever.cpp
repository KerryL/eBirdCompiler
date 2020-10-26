// File:  htmlRetriever.cpp
// Date:  10/25/2020
// Auth:  K. Loux
// Desc:  Object for downloading HTML from the specified URL.

// Local headers
#include "htmlRetriever.h"

// Standard C++ headers
#include <iostream>
#include <cassert>

//#define SAVE_TEST_FILE
//#define LOAD_TEST_FILE
#if defined(SAVE_TEST_FILE) || defined(LOAD_TEST_FILE)
#include <fstream>
const std::string testFileName("test.html");
#endif

const bool HTMLRetriever::verbose(false);
const std::string HTMLRetriever::cookieFile("cookies");

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
#ifdef SAVE_TEST_FILE
	bool ok(DoCURLGet(url, html));
	if (!ok)
		return false;
	std::ofstream f(testFileName);
	if (!f.good())
		return false;
	f << html;
	return true;
#elif defined(LOAD_TEST_FILE)
	std::ifstream f(testFileName, std::ios::ate);
	if (!f.good())
		return false;

	html.reserve(f.tellg());
	f.seekg(0, std::ios::beg);

	html.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return true;
#else
	return DoCURLGet(url, html);
#endif// LOAD_TEST_FILE
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

	// eBird requires cookies for following redirects, which are used for checklists
	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.c_str()), "Failed to load the cookie file"))
		return false;

	if (CURLCallHasError(curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.c_str()), "Failed to enable saving cookies"))
		return false;

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
