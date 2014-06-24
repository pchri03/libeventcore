/*
 * Copyright (c) 2014 Ordbogen A/S
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef INCLUDE_EVENTCORE_SIGNALHANDLER_H
#define INCLUDE_EVENTCORE_SIGNALHANDLER_H

#include <map>
#include <functional>

#include <signal.h>

namespace eventcore
{

class MainLoop;

class SignalHandler
{
	public:
		typedef std::function<void(void)> Handler;

		explicit SignalHandler(eventcore::MainLoop *mainLoop);

		~SignalHandler();

		void addSignal(int signum, const Handler &handler);
		void removeSignal(int signum);

	private:
		void onSignal();
		void removeMonitor();

	private:
		eventcore::MainLoop *m_mainLoop;
		int m_fd;
		sigset_t m_signals;
		std::map<int,Handler> m_handlers;
};

} // eventcore

#endif // INCLUDE_EVENTCORE_SIGNALHANDLER_H
