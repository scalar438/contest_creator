#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

namespace checklib {

class TimerService : private boost::noncopyable
{
public:

	~TimerService()
	{
		mService.stop();
		mThread.join();
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
	boost::thread mThread;

	TimerService()
		: mWork(mService)
	{
		mThread = boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(mService)));
	}
};

}
