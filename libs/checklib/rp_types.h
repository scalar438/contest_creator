#pragma once

namespace checklib
{

// Ограничения
struct Limits
{
	bool useTimeLimit;
	bool useMemoryLimit;
	// Лимит времени на выполнение программы, в миллисекундах
	int timeLimit;
	// Лимит памяти, в байтах
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

}
