/*
 * Copyright (C) 2014 Peter Christensen <pch@ordbogen.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef INCLUDE_EVENTCORE_MAINLOOP_H
#define INCLUDE_EVENTCORE_MAINLOOP_H

#include <map>
#include <functional>
#include <stdexcept>

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
			In,
			Out,
			InOut
		};

		typedef std::function<void(MainLoop&, int, Direction)> Callback;
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
		int m_epoll;
		int m_exitCode;
		int m_nextTimerId;
		MonitorMap m_monitors;
		TimerMap m_timers;
};

} // eventcore

#endif // INCLUDE_EVENTCORE_MAINLOOP_H
