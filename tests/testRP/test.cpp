#include "test.h"
#include "checklib/rp.h"
#include "checklib/checklib_exception.h"

#include <QtTest/QtTest>
#include <fstream>

#include <boost/filesystem.hpp>
#include <QDebug>

using namespace checklib;

void TestRun::initTestCase()
{
}

void TestRun::cleanupTestCase()
{
	boost::filesystem::remove(boost::filesystem::path(sum_input));
	boost::filesystem::remove(boost::filesystem::path(sum_output));
	boost::filesystem::remove(boost::filesystem::path(stderr_out_error));
	boost::filesystem::remove(boost::filesystem::path(args_output));
}

std::vector<std::string> TestRun::toStringList(const QStringList &list)
{
	std::vector<std::string> res;
	for(int i = 0; i < list.size(); ++i) res.push_back(list[i].toStdString());
	return res;
}

void TestRun::isRunningChecking()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useTimeLimit = true;
	limits.timeLimit = 1000;
	runner.setLimits(limits);

	runner.setProgram("./examples/pTL");
	runner.start();
	QVERIFY(!runner.wait(500));
	QVERIFY(runner.isRunning());
	QVERIFY(runner.processStatus() == checklib::psRunning);
	runner.wait();
	QVERIFY(!runner.isRunning());
}

void TestRun::testTerminate()
{
	RestrictedProcess runner;

	runner.setProgram("./examples/pTL");
	runner.start();

	QVERIFY(!runner.wait(500));

	runner.terminate();
	QVERIFY(runner.wait(100));
	QVERIFY(runner.processStatus() == checklib::psTerminated);
}

void TestRun::testExitCode()
{
	RestrictedProcess runner;
	runner.setProgram("./examples/pArgsExitCode");
	runner.start();

	runner.wait();
	QVERIFY(runner.exitCode() == 42);

	runner.reset();
	runner.setParams(toStringList(QStringList() << "123"));
	runner.start();
	runner.wait();
	QVERIFY(runner.exitCode() == 123);
}

void TestRun::testTL()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useTimeLimit = true;
	limits.timeLimit = 2000;
	runner.setLimits(limits);

	runner.setProgram("./examples/pTL");
	runner.start();

	runner.wait();

	qDebug() << "pTL time:" << runner.CPUTime();
	QVERIFY(runner.processStatus() == checklib::psTimeLimitExceeded);
}

void TestRun::testArgs()
{
	RestrictedProcess runner;
	runner.setProgram("./examples/pArgsOut");
	QStringList params;
	params << "param1" << "param with space" << "param3";
	runner.setParams(toStringList(params));

	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psExited);

	std::ifstream is(boost::filesystem::path(args_output).native());

	params.prepend("./examples/pArgsOut");
	int count;
	is >> count;
	std::string str;
	std::getline(is, str);
	QVERIFY(is.good());
	QVERIFY(count == params.size());
	std::getline(is, str);
	QVERIFY(is.good());
	// Первый аргумент - путь до исполняемого файла. Сравниваем его именно как путь (из-за того,
	// что в windows в нем могут быть как прямые, так и обратные слеши)
	QVERIFY(boost::filesystem::path(params[0].toStdString()) == boost::filesystem::path(str));
	for(int i = 1; i < count; ++i)
	{
		std::getline(is, str);
		QVERIFY(is.good());
		QVERIFY(str == params[i].toStdString());
	}
}

void TestRun::testML()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit = 64 * 1000 * 1000;
	runner.setLimits(limits);

	runner.setProgram("./examples/pML");
	runner.start();
	runner.wait();

	qDebug() << "pML memory:" << runner.peakMemoryUsage();
	QVERIFY(runner.processStatus() == checklib::psMemoryLimitExceeded);
}

void TestRun::testRE()
{
	RestrictedProcess runner;
	runner.setProgram("./examples/pRE");
	runner.setParams(toStringList(QStringList() << "1"));

	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psRuntimeError);

	runner.reset();
	runner.setParams(toStringList(QStringList() << "0"));

	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psRuntimeError);
}

void TestRun::testStandardStreamsRedirection()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit = 65536 * 1024;
	limits.useTimeLimit = true;
	limits.timeLimit = 2000;
	runner.setLimits(limits);

	runner.setProgram("./examples/pSum");

	const int a = 24;
	const int b = 18;

	std::ofstream os(boost::filesystem::path(sum_input).native());
	os << a << " " << b << std::endl << "0 0";
	os.close();

	// TODO: надо сделать без кучи конвертаций - напрямую через path
	runner.setStandardInput(QFileInfo(QString::fromStdString(sum_input)).absoluteFilePath().toStdString());
	runner.setStandardOutput(QFileInfo(QString::fromStdString(sum_output)).absoluteFilePath().toStdString());

	runner.start();
	runner.wait();
	qDebug() << "sum time and memory" << runner.CPUTime() << runner.peakMemoryUsage();
	QVERIFY(runner.processStatus() == checklib::psExited);

	{
		std::ifstream is(boost::filesystem::path(sum_output).native());

		int val = a + b + 1;

		QVERIFY(is.good());
		QVERIFY(bool(is >> val));
		QVERIFY(val == a + b);
	}

	runner.reset();
	runner.setProgram("./examples/pStderr_out");

#ifdef Q_OS_WIN
	runner.setStandardError(QString::fromStdWString(boost::filesystem::path(stderr_out_error).native()).toStdString());
#else
	runner.setStandardError(QString::fromLocal8Bit(boost::filesystem::path(stderr_out_error).native().c_str()).toStdString());
#endif
	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psExited);

	{
		std::ifstream is(boost::filesystem::path(stderr_out_error).native());
		std::string str;
		QVERIFY(is.good());
		QVERIFY(bool(std::getline(is, str)));
		QVERIFY(str == "Test printing to stderr");
		QVERIFY(bool(std::getline(is, str)));
		QVERIFY(str == "Line2");
	}
}

void TestRun::testIL()
{
	RestrictedProcess runner;

	runner.setProgram("./examples/pIL");
	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psIdlenessLimitExceeded);
}

void TestRun::testInteractive()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit = 65536 * 1024;
	limits.useTimeLimit = true;
	limits.timeLimit = 2000;
	runner.setLimits(limits);

	runner.setStandardInput(checklib::ss::Interactive);
	runner.setStandardOutput(checklib::ss::Interactive);

	runner.setProgram("./examples/pSum");

	runner.start();
	runner.sendDataToStandardInput("4 5\n");
	std::string ans;
	if(!runner.getDataFromStandardOutput(ans)) QFAIL("getDataFromStandardOutput returned false");
	QVERIFY(ans == "9");

	runner.sendDataToStandardInput("2 3", true);
	if(!runner.getDataFromStandardOutput(ans)) QFAIL("getDataFromStandardOutput returned false");
	QVERIFY(ans == "5");

	runner.sendDataToStandardInput("0 0", true);
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psExited);
}

void TestRun::testException()
{
	RestrictedProcess runner;
	runner.setProgram("./examples/FileNotExists");
	bool exceptionFlag = false;
	try
	{
		runner.start();
		runner.wait();
	}
	catch(Exception &)
	{
		exceptionFlag = true;
	}

	QVERIFY(exceptionFlag);
}

TestRun::TestRun():
	sum_input("sum_input.txt")
	, sum_output("sum_output.txt")
	, stderr_out_error("stderr_out_error.txt")
	, args_output("args_out.txt")
{

}
