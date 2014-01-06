#ifndef INCLUDE_EVENTCORE_TIMER_H
#define INCLUDE_EVENTCORE_TIMER_H

#include <functional>
#include <stdexcept>

namespace eventcore
{

class MainLoop;

class Timer
{
	public:
		typedef std::function<void(Timer&)> Callback;

		Timer(MainLoop *mainLoop, const Callback &callback = Callback());
		~Timer();

		void start(unsigned long int timeout) throw(std::runtime_error);
		void stop();

		MainLoop *mainLoop() const;
		
		void setCallback(const Callback &callback);
		const Callback &callback() const;

		unsigned long int timeout() const throw(std::runtime_error);
		unsigned long long int expiration() const;

		static unsigned long long int currentTime();

	private:
		MainLoop *m_mainLoop;
		Callback m_callback;
		unsigned long long int m_expiration;
};

} // eventcore

#endif // INCLUDE_EVENTCORE_TIMER_H


