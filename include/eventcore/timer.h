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

#ifndef INCLUDE_EVENTCORE_TIMER_H
#define INCLUDE_EVENTCORE_TIMER_H

#include <functional>

namespace eventcore
{

class MainLoop;

class Timer
{
	public:
		typedef std::function<void(Timer&)> Callback;

		Timer(MainLoop *mainLoop, const Callback &callback = Callback());
		~Timer();

		void start(unsigned long int timeout);
		void stop();

		MainLoop *mainLoop() const;
		
		void setCallback(const Callback &callback);
		const Callback &callback() const;

		unsigned long int timeout() const;
		unsigned long long int expiration() const;

		static unsigned long long int currentTime();

	private:
		MainLoop *m_mainLoop;
		Callback m_callback;
		unsigned long long int m_expiration;
};

} // eventcore

#endif // INCLUDE_EVENTCORE_TIMER_H


