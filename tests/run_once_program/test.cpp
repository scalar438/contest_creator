#include <QtTest/QtTest>
#include "test.h"

void TestRun::testTL()
{
	checklib::RestrictedProcess runner("../TL");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::etTimeLimt);
}

void TestRun::testML()
{
	checklib::RestrictedProcess runner("../ML");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::etMemoryLimit);
}

void TestRun::testRE1()
{
	checklib::RestrictedProcess runner("../RE1");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::etRuntimeError);
}

void TestRun::testRE2()
{
	checklib::RestrictedProcess runner("../RE2");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::etRuntimeError);
}
