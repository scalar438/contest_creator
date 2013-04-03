#pragma once
#include <QtTest/QtTest>

class TestQString: public QObject
{
	Q_OBJECT

private slots:
	void toUpper_data();
	void toUpper();
};
