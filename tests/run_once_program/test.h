#pragma once
#include <QtTest/QtTest>
#include "checklib/restricted_process.h"

// Класс, тестирующий работу библиотеки
class TestRun: public QObject
{
	Q_OBJECT

private slots:

	void testTL();
	void testML();
	void testRE1();
	void testRE2();
};
