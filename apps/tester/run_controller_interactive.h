#pragma once
#include "run_controller_abstract.h"
#include <checklib/checklib.h>
#include <thread>
#include <boost/asio/deadline_timer.hpp>

class RunControllerInteractive : public RunControllerAbstract
{
public:
	RunControllerInteractive(boost::asio::io_service &io, ParamsReader &reader);
	~RunControllerInteractive();

	void startTesting();

private:

	boost::asio::io_service &mIo;

	std::thread mProgramReaderThread, mInteractorReaderThread;

	checklib::RestrictedProcess mProgram, mInteractor;

	boost::asio::deadline_timer mTimer;

	ParamsReader &mReader;

	int mCurrentTest;

	void printUsageTimerHandler(boost::system::error_code err);

	void printUsage(bool final);

	void startCurrentTest();

	void programFinished();

	// Функции, запускаемые объектами checklib
	void onInteractorFinished(int exitCode);
	void onProgramFinished(int exitCode);

	void endTesting();
};
