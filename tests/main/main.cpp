#include "test.h"
#include <QtTest>
#include <iostream>
#include <QCoreApplication>

int main()
{
	TestRun *a = new TestRun();
	QTest::qExec(a);
	delete a;
}
