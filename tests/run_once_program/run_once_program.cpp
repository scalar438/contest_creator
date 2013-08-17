#include <QtTest>
#include "test.h"
#include <iostream>
#include <Windows.h>

int main()
{
	checklib::RestrictedProcess rp;
	rp.setProgram("D:/Develop/contest-creator/q.exe");
	rp.redirectStandardInput("D:/Develop/contest-creator/q.txt");
	rp.redirectStandardOutput("D:/Develop/contest-creator/q2.txt");
	qDebug() << "Before start";
	rp.start();
	rp.wait();
//	TestRun a;
//	QTest::qExec(&a);
}
