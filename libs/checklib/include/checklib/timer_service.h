#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <boost/noncopyable.hpp>

namespace checklib {

#ifndef CHECKLIB_WINDOWS

class TimerService : private boost::noncopyable
{
public:

	~TimerService()
	{
		mService.stop();
		m_thread.join();
	}

	boost::asio::io_service &io_service()
	{
		return mService;
	}

	static TimerService *instance()
	{
		static TimerService res;
		return &res;
	}

private:
	boost::asio::io_service mService;
	boost::asio::io_service::work mWork;
	std::thread m_thread;

	TimerService()
		: mWork(mService)
	{
		m_thread = std::thread([this]() { this->mService.run(); });
	}
};

#endif

}
