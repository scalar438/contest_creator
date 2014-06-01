#pragma once
#include "checklib/rp.h"
#include "params_reader.h"

#include <memory>
#include <boost/asio.hpp>

class RunController
{
public:
	RunController(boost::asio::io_service &io, ParamsReader &reader);
	void startTesting();
private:
	boost::asio::io_service &mIo;

	boost::asio::deadline_timer mTimer;

	ParamsReader &mReader;

	checklib::RestrictedProcess mProcess;

	int mCurrentTest;

	// Запускает на запуск очередной тест или выходит
	void startCurrentTest();

	// Запускает чекер ответов
	void endCurrrentTest();

	// Пишет использование программой процессорного времени и памяти
	void printUsageTimerHandler(boost::system::error_code err);

	void onProgramFinished(int);

	// Выводит использование процессорного времени/памяти тестируемой программы.
	// Если final == true, то выводит более ярко
	void printUsage(bool final);

	void endTesting();
};
