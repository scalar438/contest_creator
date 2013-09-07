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
	boost::filesystem::remove(boost::filesystem::path("./tests_runexamples/input.txt"));
	boost::filesystem::remove(boost::filesystem::path("./tests_runexamples/output.txt"));
}

void TestRun::testTL()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
	limits.useTimeLimit = true;
	limits.timeLimit = 2000;
	runner.setLimits(limits);

	runner.setProgram("tests_runexamples/pTL");
	runner.start();
	QVERIFY(!runner.wait(1000));
	QVERIFY(runner.processStatus() == checklib::psRunning);
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psTimeLimit);
}

void TestRun::testML()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit = 256 * 1000 * 1000;
	runner.setLimits(limits);

	runner.setProgram("tests_runexamples/pML");
	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psMemoryLimit);
}

void TestRun::testRE()
{
	checklib::RestrictedProcess runner;
	runner.setProgram("tests_runexamples/pRE");
	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psExited);
}

void TestRun::testSumStandard()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit = 65536 * 1024;
	limits.useTimeLimit = true;
	limits.timeLimit = 2000;
	runner.setLimits(limits);

	runner.setProgram("./tests_runexamples/pSum");

	const int a = 24;
	const int b = 18;

	std::ofstream os(boost::filesystem::path("./tests_runexamples/input.txt").native());
	os << a << " " << b;
	os.close();

	runner.setStandardInput(QString::fromStdWString(boost::filesystem::path("./tests_runexamples/input.txt").native()));
	runner.setStandardOutput(QString::fromStdWString(boost::filesystem::path("./tests_runexamples/output.txt").native()));

	runner.start();
	runner.wait();
	QVERIFY(runner.processStatus() == checklib::psExited);

	std::ifstream is(boost::filesystem::path("./tests_runexamples/output.txt").native());

	int val = a + b + 1;

	QVERIFY(is.good());
	QVERIFY(is >> val);
	QVERIFY(val == a + b);
}

void TestRun::testIL()
{
	checklib::RestrictedProcess runner;

	runner.setProgram("tests_runexamples/pIL");
	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psIdlenessLimit);
}
