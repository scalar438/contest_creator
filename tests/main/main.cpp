#include "test.h"
#include <QtTest>
#include <iostream>
#include <QCoreApplication>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>

int main()
{
	TestRun *a = new TestRun();
	QTest::qExec(a);

	delete a;
}
