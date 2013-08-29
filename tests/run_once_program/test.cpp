#include <QtTest/QtTest>
#include "test.h"

void TestRun::testTL()
{
	checklib::RestrictedProcess runner;
	runner.setProgram("../pTL.exe");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::psTimeLimit);
}

void TestRun::testML()
{
/*	checklib::RestrictedProcess runner;
	runner.setProgram("../ML");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::psMemoryLimit);*/
}

void TestRun::testRE1()
{
/*	checklib::RestrictedProcess runner;
	runner.setProgram("../RE1");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::psRuntimeError);*/
}

void TestRun::testRE2()
{
/*	checklib::RestrictedProcess runner;
	runner.setProgram("../RE2");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::psRuntimeError);*/
}
