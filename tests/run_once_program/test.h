﻿#pragma once
#include <QtTest/QtTest>
#include "checklib/rp.h"

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

class BackgroundThread : public QThread
{
	Q_OBJECT
protected:
	void run() override
	{
		exec();
	}
};
