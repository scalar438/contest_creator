#pragma once

namespace checklib
{

class RestrictedProcess;

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

// Тип завершения программы. psFailed - внутренняя ошибка тестирования
enum ProcessStatus
{
	psNotRunning, psRunning, psExited, psTimeLimit, psMemoryLimit, psIdlenessLimit, psRuntimeError, psTerminated, psFailed
};

enum StandardStream
{
	ssStdin, ssStdout, ssStderr
};

}
