/*
 * Copyright (c) 2014 Ordbogen A/S
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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


