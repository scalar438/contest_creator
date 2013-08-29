#include <QtTest>
#include "test.h"
#include <iostream>
#include <Windows.h>

int main()
{
	TestRun a;
	QTest::qExec(&a);
}
