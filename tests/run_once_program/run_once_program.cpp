#include <QtTest>
#include "test.h"
#include <iostream>
#include <Windows.h>
#include <QCoreApplication>

int main()
{
	TestRun a;
	QTest::qExec(&a);
}
