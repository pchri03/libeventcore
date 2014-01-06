#include "eventcore/mainloop.h"
#include "eventcore/timer.h"

#include <cstring>
#include <cerrno>
#include <stdexcept>

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
	{
		m_exitCode = -1;
		throw std::runtime_error(std::strerror(errno));
	}
}

MainLoop::~MainLoop()
{
	for (MonitorMap::iterator it = m_monitors.begin(); it != m_monitors.end(); ++it)
		delete it->second;

	if (m_epoll != -1)
		while (::close(m_epoll) == -1 && errno == EINTR);
}

bool MainLoop::addMonitor(int fd, Direction direction, const Callback &callback) throw(std::runtime_error)
{
	if (m_monitors.find(fd) != m_monitors.end())
		return false;

	Monitor *monitor = new Monitor(fd, callback);

	::epoll_event event;
	event.events = (direction == Out ? EPOLLOUT : EPOLLIN);
	event.data.ptr = monitor;
	if (::epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		int err = errno;
		delete monitor;
		throw std::runtime_error(std::strerror(err));
	}

	m_monitors[fd] = monitor;

	return true;
}

bool MainLoop::removeMonitor(int fd) throw(std::runtime_error)
{
	MonitorMap::iterator it = m_monitors.find(fd);
	if (it == m_monitors.end())
		return false;

	// Notice: Linux < 2.6.9 requires a non-null event argument
	if (::epoll_ctl(m_epoll, EPOLL_CTL_DEL, fd, 0) == -1)
		throw std::runtime_error(std::strerror(errno));

	delete it->second;
	m_monitors.erase(it);

	return true;
}

void MainLoop::unregisterTimer(Timer *timer)
{
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

int MainLoop::run() throw(std::runtime_error)
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
		epoll_event events[16];
		int count = ::epoll_wait(m_epoll, events, sizeof(events) / sizeof(events[0]), timeout);
		if (count == -1 && errno == EINTR)
			continue;
		if (count == -1)
			throw std::runtime_error(std::strerror(errno));

		// Handle events
		for (int i = 0; i != count; ++i)
		{
			Monitor *monitor = reinterpret_cast<Monitor *>(events[i].data.ptr);
			if (monitor->callback)
				monitor->callback(*this, monitor->fd);
		}

		// Handle expired timers
		unsigned long long int t = Timer::currentTime();
		for (TimerMap::iterator it = m_timers.begin(); it != m_timers.end() && it->first <= t; it = m_timers.begin())
		{
			Timer *timer = it->second;
			m_timers.erase(it);

			timer->callback()(*timer);
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
