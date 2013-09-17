#pragma once

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
	psNotRunning, psRunning, psExited, psTimeLimit, psMemoryLimit, psIdlenessLimit, psRuntimeError, psTerminated
};

enum StandardStream
{
	ssStdin, ssStdout, ssStderr
};

}
