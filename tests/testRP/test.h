#pragma once
#include <QTest>
#ifndef Q_MOC_RUN
#include "checklib/rp.h"
#endif

// Класс, тестирующий работу библиотеки
class TestRun: public QObject
{
	Q_OBJECT
public:
	TestRun();

Q_SLOTS

	void initTestCase();

	void isRunningChecking();

	void testTerminate();

	void testExitCode();

	void testTL();

	void testArgs();


	void testML();

	void testRE();
	void testStandardStreamsRedirection();
	void testIL();
	void testInteractive();

	void testException();

	void cleanupTestCase();

private:

	const std::string sum_input;
	const std::string sum_output;
	const std::string stderr_out_error;
	const std::string args_output;

	std::vector<std::string> toStringList(const QStringList &list);
};
