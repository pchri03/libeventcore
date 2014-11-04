/*
 * Copyright (c) 2014 Ordbogen A/S
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "eventcore/mainloop.h"
#include "eventcore/timer.h"

#include <cstring>
#include <cerrno>

#ifdef MAINLOOP_THREADS
#include <mutex>
#endif // MAINLOOP_THREADS

#include <sys/epoll.h>
#include <unistd.h>

namespace eventcore
{

MainLoop::MainLoop()
	// Notice: epoll_create1 requires Linux 2.6.27 and glibc version 2.9
	: m_epoll(::epoll_create1(EPOLL_CLOEXEC))
	, m_exitCode(0)
	, m_nextTimerId(0)
{
	if (m_epoll == -1)
		m_exitCode = -1;
}

MainLoop::~MainLoop()
{
	for (MonitorMap::iterator it = m_monitors.begin(); it != m_monitors.end(); ++it)
		delete it->second;

	if (m_epoll != -1)
		while (::close(m_epoll) == -1 && errno == EINTR);
}

bool MainLoop::addMonitor(int fd, Direction direction, const Callback &callback)
{
#ifdef MAINLOOP_THREADS
	std::unique_lock<std::mutex> lock(m_mutex);
#endif // MAINLOOP_THREADS

	if (m_epoll == -1 || m_monitors.find(fd) != m_monitors.end())
		return false;

	Monitor *monitor = new Monitor(fd, callback);

	::epoll_event event;
	switch (direction)
	{
		case In:
			event.events = EPOLLIN;
			break;

		case Out:
			event.events = EPOLLOUT;
			break;

		case InOut:
			event.events = EPOLLIN | EPOLLOUT;
			break;

		default:
			return false;
	}
	event.data.fd = fd;
	if (::epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		delete monitor;
		return false;
	}

	m_monitors[fd] = monitor;

	return true;
}

bool MainLoop::removeMonitor(int fd)
{
#ifdef MAINLOOP_THREADS
	std::unique_lock<std::mutex> lock(m_mutex);
#endif // MAINLOOP_THREADS

	if (m_epoll == -1)
		return false;

	MonitorMap::iterator it = m_monitors.find(fd);
	if (it == m_monitors.end())
		return false;

	// Notice: Linux < 2.6.9 requires a non-null event argument
	if (::epoll_ctl(m_epoll, EPOLL_CTL_DEL, fd, 0) == -1)
		return false;

	delete it->second;
	m_monitors.erase(it);

	return true;
}

void MainLoop::unregisterTimer(Timer *timer)
{
#ifdef MAINLOOP_THREADS
	std::unique_lock<std::mutex> lock(m_mutex);
#endif // MAINLOOP_THREADS

	unsigned long long int expiration = timer->expiration();

	for (
			TimerMap::iterator it = m_timers.find(expiration);
			it != m_timers.end() && it->first == expiration;
			++it)
	{
		if (it->second == timer)
		{
			m_timers.erase(it);
			break;
		}
	}
}

void MainLoop::registerTimer(Timer *timer)
{
#ifdef MAINLOOP_THREADS
	std::unique_lock<std::mutex> lock(m_mutex);
#endif // MAINLOOP_THREADS

	m_timers.insert(TimerMap::value_type(timer->expiration(), timer));
}

void MainLoop::exit(int code)
{
	if (m_epoll != -1)
	{
		while (::close(m_epoll) == -1 && errno == EINTR);
		m_epoll = -1;
		m_exitCode = code;
	}
}

int MainLoop::run()
{
	while (m_epoll != -1 && (m_monitors.size() != 0 || m_timers.size() != 0))
	{
		// Get desired timeout
		int timeout = -1;
		TimerMap::const_iterator timer = m_timers.begin();
		if (timer != m_timers.end())
		{
			timeout = timer->second->timeout();
		}

		// Wait for events
		epoll_event events[32];
		int count = ::epoll_wait(m_epoll, events, sizeof(events) / sizeof(events[0]), timeout);
		if (count == -1 && errno == EINTR)
			continue;
		if (count == -1)
			break;

#ifdef MAINLOOP_THREADS
		std::unique_lock<std::mutex> lock(m_mutex);
#endif // MAINLOOP_THREADS

		// Handle events
		for (int i = 0; i != count; ++i)
		{
			auto it = m_monitors.find(events[i].data.fd);
			if (it == m_monitors.end())
				continue;

			Monitor *monitor = it->second;
			if (monitor->callback)
			{
				Direction dir;
				switch (events[i].events & (EPOLLIN|EPOLLOUT))
				{
					case EPOLLIN:
						dir = In;
						break;

					case EPOLLOUT:
						dir = Out;
						break;

					case EPOLLIN | EPOLLOUT:
						dir = InOut;
						break;
				}
#ifdef MAINLOOP_THREADS
				lock.unlock();
#endif // MAINLOOP_THREADS
				monitor->callback(dir);
#ifdef MAINLOOP_THREADS
				lock.lock();
#endif // MAINLOOP_THREADS
			}
		}

		// Handle expired timers
		unsigned long long int t = Timer::currentTime();
		for (TimerMap::iterator it = m_timers.begin(); it != m_timers.end() && it->first <= t; it = m_timers.begin())
		{
			Timer *timer = it->second;
			m_timers.erase(it);

#ifdef MAINLOOP_THREADS
			lock.unlock();
#endif // MAINLOOP_THREADS
			timer->callback()();
#ifdef MAINLOOP_THREADS
			lock.lock();
#endif // MAIN_LOOP_THREADS
		}
	}

	return m_exitCode;
}

MainLoop::Monitor::Monitor(int fd, const Callback &callback)
	: fd(fd)
	, callback(callback)
{
}

} // eventcore
