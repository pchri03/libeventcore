/*
 * Copyright (c) 2014 Ordbogen A/S
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef INCLUDE_EVENTCORE_MAINLOOP_H
#define INCLUDE_EVENTCORE_MAINLOOP_H

#include <map>
#include <functional>
#include <stdexcept>

#ifdef MAINLOOP_THREADS
#include <mutex>
#endif // MAINLOOP_THREADS

namespace eventcore
{

class Timer;

class MainLoop
{
	friend class Timer;

	public:
		MainLoop();
		~MainLoop();

		enum Direction
		{
			Error,
			In,
			Out,
			InOut
		};

		typedef std::function<void(Direction)> Callback;
		bool addMonitor(int fd, Direction direction, const Callback &callback);
		bool removeMonitor(int fd);

		void exit(int code);

		int run();

	private:
		void registerTimer(Timer *timer);
		void unregisterTimer(Timer *timer);

		struct Monitor
		{
			int fd;
			Callback callback;

			Monitor();
			Monitor(int fd, const Callback &callback);
		};

		typedef std::map<int, Monitor*> MonitorMap;
		typedef std::multimap<unsigned long long int, Timer*> TimerMap;

	private:
#ifdef MAINLOOP_THREADS
		std::mutex m_mutex;
#endif // MAINLOOP_THREADS
		int m_epoll;
		int m_exitCode;
		int m_nextTimerId;
		MonitorMap m_monitors;
		TimerMap m_timers;
};

} // eventcore

#endif // INCLUDE_EVENTCORE_MAINLOOP_H
