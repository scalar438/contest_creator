#include <QtTest/QtTest>
#include "test.h"

int main()
{
	TestRun a;
	QTest::qExec(&a);
}
