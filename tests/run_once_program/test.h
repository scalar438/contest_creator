#pragma once
#include <QtTest/QtTest>
#include "checklib/rp.h"

// Класс, тестирующий работу библиотеки
class TestRun: public QObject
{
	Q_OBJECT

private slots:

	void initTestCase();
	void cleanupTestCase();

	void testTL();

	void testML();

	void testRE();
	void testSumStandard();
	void testIL();
};
