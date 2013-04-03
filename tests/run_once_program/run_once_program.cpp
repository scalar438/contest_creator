#include <QtTest/QtTest>
#include "test.h"

int main()
{
	TestRun *a = new TestRun;
	QTest::qExec(a);
	delete a;
}
