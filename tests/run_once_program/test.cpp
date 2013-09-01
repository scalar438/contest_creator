#include <QtTest/QtTest>
#include "test.h"

void TestRun::testTL()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
	limits.useTimeLimit = true;
	limits.timeLimit = 2000;
	runner.setLimits(limits);

	runner.setProgram("tests_runexamples/pTL.exe");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::psTimeLimit);
}

void TestRun::testML()
{
	checklib::RestrictedProcess runner;

	checklib::Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit = 65536;
	runner.setLimits(limits);

	runner.setProgram("tests_runexamples/pML.exe");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::psMemoryLimit);
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

void TestRun::testNormal()
{

}

void TestRun::testIL()
{
	checklib::RestrictedProcess runner;

	runner.setProgram("tests_runexamples/pIL.exe");
	runner.start();
	runner.wait();

	QVERIFY(runner.exitType() == checklib::psIdlenessLimit);
}
