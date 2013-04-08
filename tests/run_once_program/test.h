#pragma once
#include <QtTest/QtTest>
//#include "checklib/restproc.h"

class TestRun: public QObject
{
	Q_OBJECT

private slots:

	void testTL();
	void testML();

};
