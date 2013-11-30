﻿#pragma once
#include <QObject>
#include <QString>
#include "checklib/rp.h"

class Tester : public QObject
{
	Q_OBJECT
private:
	Tester();
	~Tester();

private slots:

	// Проверка и вывод использования ресурсов
	void onCheckTimerTimeout();

	void onTestFinished(int exitCode);
};

// Класс, живущий в отдельном потоке, запускающий программу и выдающий ее вердикты
class Runner : public QObject
{
	Q_OBJECT
public:
	Runner(const QString &programName);

	int getTime() const;

	int getMemoryUsage() const;

	checklib::ProcessStatus getProcessStatus() const;
public slots:
	void run(QString inputFileName, QString outputFileName);

signals:

	void finished(int exitCode);

private:

	checklib::RestrictedProcess rp;
};
