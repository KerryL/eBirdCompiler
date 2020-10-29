// File:  throttledSection.h
// Date:  1/29/2018
// Auth:  K. Loux
// Desc:  Tool for managing access to a resource with a rate limit.

#ifndef THROTTLED_SECTION_H_
#define THROTTLED_SECTION_H_

// Standard C++ headers
#include <chrono>
#include <mutex>

class ThrottledSection
{
public:
	typedef std::chrono::steady_clock Clock;
	ThrottledSection(const Clock::duration& minAccessDelta);
	void SetMinAccessDelta(const Clock::duration& newMinAccessDelta) { minAccessDelta = newMinAccessDelta; }
	void Wait();

private:
	Clock::duration minAccessDelta;
	Clock::time_point lastAccess = Clock::now();
	std::mutex mutex;
};

#endif// THROTTLED_SECTION_H_
