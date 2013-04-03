#include <QtTest/QtTest>
#include "test.h"

void TestQString::toUpper_data()
{
	QTest::addColumn<QString>("string");
	QTest::addColumn<QString>("result");

	QTest::newRow("all lower") << "hello" << "HELLO";
	QTest::newRow("mixed")     << "Hello" << "HELLO";
	QTest::newRow("all upper") << "HELLO" << "HELLO";
}

void TestQString::toUpper()
{
	QFETCH(QString, string);
	QFETCH(QString, result);

	QCOMPARE(string.toUpper(), result);
}
