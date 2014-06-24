/*
 * Copyright (c) 2014 Ordbogen A/S
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "eventcore/signalhandler.h"
#include "eventcore/mainloop.h"

#include <unistd.h>
#include <sys/signalfd.h>

namespace eventcore
{

SignalHandler::SignalHandler(eventcore::MainLoop *mainLoop)
	: m_mainLoop(mainLoop)
	, m_fd(-1)
{
	sigemptyset(&m_signals);
}

SignalHandler::~SignalHandler()
{
	if (m_fd != -1)
	{
		removeMonitor();
	}
}

void SignalHandler::addSignal(int signum, const Handler &handler)
{
	m_handlers[signum] = handler;
	if (!sigismember(&m_signals, signum))
	{
		sigaddset(&m_signals, signum);
		sigprocmask(SIG_BLOCK, &m_signals, NULL);
		if (m_fd == -1)
		{
			// Notice: signalfd requires Linux 2.6.26
			// Notice: flags requires Linux 2.6.27
			// Notice: glibc 2.8 is needed to deliver signalfd macro
			m_fd = signalfd(-1, &m_signals, SFD_CLOEXEC | SFD_NONBLOCK);
			if (m_fd != -1)
				m_mainLoop->addMonitor(m_fd, eventcore::MainLoop::In, std::bind(&SignalHandler::onSignal, this));
		}
		else
			signalfd(m_fd, &m_signals, SFD_CLOEXEC | SFD_NONBLOCK);
	}
}

void SignalHandler::removeSignal(int signum)
{
	if (!sigismember(&m_signals, signum))
		return;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, signum);
	sigprocmask(SIG_UNBLOCK, &set, NULL);

	m_handlers.erase(signum);
	if (m_fd != -1 && m_handlers.empty())
		removeMonitor();
}

void SignalHandler::removeMonitor()
{
	m_mainLoop->removeMonitor(m_fd);
	close(m_fd);
}

void SignalHandler::onSignal()
{
	signalfd_siginfo info;
	int size = read(m_fd, &info, sizeof(info));
	if (size != sizeof(info))
		return;

	auto handler = m_handlers.find(info.ssi_signo);
	if (handler == m_handlers.end())
		return;

	handler->second();
}

} // eventcore
