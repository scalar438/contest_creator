#include <QtTest/QtTest>
#include "test.h"

void TestRun::testTL()
{
	checklib::RestrictedProcess runner;
	runner.setProgram("../TL");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::etTimeLimit);
}

void TestRun::testML()
{
	checklib::RestrictedProcess runner;
	runner.setProgram("../ML");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::etMemoryLimit);
}

void TestRun::testRE1()
{
	checklib::RestrictedProcess runner;
	runner.setProgram("../RE1");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::etRuntimeError);
}

void TestRun::testRE2()
{
	checklib::RestrictedProcess runner;
	runner.setProgram("../RE2");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::etRuntimeError);
}
