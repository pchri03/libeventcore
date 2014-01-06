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
			Out
		};

		typedef std::function<void(MainLoop&,int)> Callback;
		bool addMonitor(int fd, Direction direction, const Callback &callback) throw(std::runtime_error);
		bool removeMonitor(int fd) throw(std::runtime_error);

		void exit(int code);

		int run() throw(std::runtime_error);

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
		static unsigned long long int getTime() throw(std::runtime_error);

	private:
		int m_epoll;
		int m_exitCode;
		int m_nextTimerId;
		MonitorMap m_monitors;
		TimerMap m_timers;
};

} // eventcore

#endif // INCLUDE_EVENTCORE_MAINLOOP_H
