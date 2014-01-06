#include "eventcore/timer.h"
#include "eventcore/mainloop.h"

#include <stdexcept>
#include <cstring>
#include <cerrno>

#include <time.h>

namespace eventcore
{

Timer::Timer(MainLoop *mainLoop, const Callback &callback)
	: m_mainLoop(mainLoop)
	, m_callback(callback)
	, m_expiration(0)
{
}

Timer::~Timer()
{
	m_mainLoop->unregisterTimer(this);
}

void Timer::start(unsigned long int timeout) throw(std::runtime_error)
{
	m_mainLoop->unregisterTimer(this);
	
	m_expiration = currentTime() + timeout;
	m_mainLoop->registerTimer(this);
}

void Timer::stop()
{
	m_mainLoop->unregisterTimer(this);
	m_expiration = 0;
}

MainLoop *Timer::mainLoop() const
{
	return m_mainLoop;
}

const Timer::Callback &Timer::callback() const
{
	return m_callback;
}

void Timer::setCallback(const Callback &callback)
{
	m_callback = callback;
}

unsigned long long int Timer::expiration() const
{
	return m_expiration;
}

unsigned long int Timer::timeout() const throw(std::runtime_error)
{
	unsigned long long int now = currentTime();
	if (now >= m_expiration)
		return 0;
	else
		return m_expiration - now;
}

unsigned long long int Timer::currentTime()
{
	// Notice: CLOCK_MONOTONIC_RAW requires Linux 2.6.28
#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW 4
#endif // CLOCK_MONOTONIC_RAW

	timespec tp;
	if (::clock_gettime(CLOCK_MONOTONIC_RAW, &tp) == -1)
		throw std::runtime_error(std::strerror(errno));

	return (unsigned long long int)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}

} // eventcore
