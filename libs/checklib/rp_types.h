#pragma once
#include <QString>

namespace checklib
{

// Ограничения
struct Limits
{
	bool useTimeLimit;
	bool useMemoryLimit;
	int timeLimit;
	int memoryLimit;
	Limits()
	{
		useTimeLimit = false;
		useMemoryLimit = false;
		timeLimit = memoryLimit = 0;
	}
};

// Тип завершения программы.
enum ProcessStatus
{
	psNotRunning, psRunning, psExited, psTimeLimitExceeded, psMemoryLimitExceeded, psIdlenessLimitExceeded, psRuntimeError, psTerminated
};

namespace StandardStreams
{
const QString stdin = "stdin";
const QString stdout = "stdout";
const QString stderr = "stderr";
const QString interactive = "interactive";
}

}
