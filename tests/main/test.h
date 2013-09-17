#pragma once
#include <QtTest/QtTest>
#include "checklib/rp.h"

// Класс, тестирующий работу библиотеки
class TestRun: public QObject
{
	Q_OBJECT
public:
	TestRun();

private slots:

	void initTestCase();

	void isRunningChecking();

	void testTL();

	void testArgs();


	void testML();

	void testRE();
	void testStandardStreamsRedirection();
	void testIL();


	void cleanupTestCase();

private:

	const std::string sum_input;
	const std::string sum_output;
	const std::string stderr_out_error;
	const std::string args_output;
};
