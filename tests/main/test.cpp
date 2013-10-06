#include "test.h"
#include "checklib/rp.h"

#include <QtTest/QtTest>
#include <fstream>

#include <boost/filesystem.hpp>
#include <QDebug>

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

void TestRun::isRunningChecking()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
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

void TestRun::testTL()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
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
	checklib::RestrictedProcess runner;
	runner.setProgram("./examples/pArgsOut");
	QStringList params;
	params << "param1" << "param with space" << "param3";
	runner.setParams(params);

	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psExited);

	std::ifstream is(boost::filesystem::path(args_output).native());

	params.prepend(QFileInfo("./examples/pArgsOut").absoluteFilePath());
	int count;
	is >> count;
	std::string str;
	std::getline(is, str);
	QVERIFY(is.good());
	QVERIFY(count == params.size());
	for(int i = 0; i < count; ++i)
	{
		std::getline(is, str);
		QVERIFY(is.good());
		QVERIFY(str == params[i].toStdString());
	}
}

void TestRun::testML()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit = 256 * 1000 * 1000;
	runner.setLimits(limits);

	runner.setProgram("./examples/pML");
	runner.start();
	runner.wait();

	qDebug() << "pML memory:" << runner.peakMemoryUsage();
	QVERIFY(runner.processStatus() == checklib::psMemoryLimitExceeded);
}

void TestRun::testRE()
{
	checklib::RestrictedProcess runner;
	runner.setProgram("./examples/pRE");
	runner.setParams(QStringList() << "1");

	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psRuntimeError);

	runner.reset();
	runner.setParams(QStringList() << "0");

	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psRuntimeError);
}

void TestRun::testStandardStreamsRedirection()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
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

	runner.setStandardInput(QFileInfo(QString::fromStdString(sum_input)).absoluteFilePath());
	runner.setStandardOutput(QFileInfo(QString::fromStdString(sum_output)).absoluteFilePath());

	runner.start();
	runner.wait();
	qDebug() << "sum time and memory" << runner.CPUTime() << runner.peakMemoryUsage();
	QVERIFY(runner.processStatus() == checklib::psExited);

	{
		std::ifstream is(boost::filesystem::path(sum_output).native());

		int val = a + b + 1;

		QVERIFY(is.good());
		QVERIFY(is >> val);
		QVERIFY(val == a + b);
	}

	runner.reset();
	runner.setProgram("./examples/pStderr_out");

#ifdef Q_OS_WIN
	runner.setStandardError(QString::fromStdWString(boost::filesystem::path(stderr_out_error).native()));
#else
	runner.setStandardError(QString::fromLocal8Bit(boost::filesystem::path(stderr_out_error).native().c_str()));
#endif
	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psExited);

	{
		std::ifstream is(boost::filesystem::path(stderr_out_error).native());
		std::string str;
		QVERIFY(is.good());
		QVERIFY(std::getline(is, str));
		QVERIFY(str == "Test printing to stderr");
		QVERIFY(std::getline(is, str));
		QVERIFY(str == "Line2");
	}
}

void TestRun::testIL()
{
	checklib::RestrictedProcess runner;

	runner.setProgram("./examples/pIL");
	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psIdlenessLimitExceeded);
}

void TestRun::testInteractive()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
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
	QString ans;
	runner.getDataFromStandardOutput(ans);
	QVERIFY(ans == "9\n");

	runner.sendDataToStandardInput("2 3", true);
	runner.getDataFromStandardOutput(ans);
	QVERIFY(ans == "5\n");

	runner.sendDataToStandardInput("0 0", true);
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psExited);
}

TestRun::TestRun():
	sum_input("./examples/sum_input.txt")
	, sum_output("./examples/sum_output.txt")
	, stderr_out_error("./examples/stderr_out_error.txt")
	, args_output("./examples/args_out.txt")
{

}
