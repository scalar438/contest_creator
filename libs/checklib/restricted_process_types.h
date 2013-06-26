#pragma once

namespace checklib
{

/// Ограничения
struct Restrictions
{
	bool useTimeLimit;
	bool useMemoryLimit;
	int timeLimit;
	int memoryLimit;
	Restrictions()
	{
		useTimeLimit = false;
		useMemoryLimit = false;
		timeLimit = memoryLimit = 0;
	}
};

/// Тип завершения программы. etFailed - внутренняя ошибка тестирования
enum ProcessStatus
{
	etNormal, etRunning, etTimeLimit, etMemoryLimit, etIdlenessLimit, etRuntimeError, etTerminated, etFailed
};

enum StandardStream
{
	ssStdin, ssStdout, ssStderr
};

}
