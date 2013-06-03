#include <windows.h>
#include "checklib_exception.h"
#include "restricted_process.h"
#include "psapi.h"

struct checklib::details::platform_data
{
	HANDLE process;
};

checklib::RestrictedProcess::RestrictedProcess(const QString &program, const QStringList &params)
	: RestrictedProcess(nullptr, program, params)
{
	// Implementation in other constructor
}

checklib::RestrictedProcess::RestrictedProcess(QObject *parent, const QString &program, const QStringList &params)
	: QObject(parent), mPlatformData(new checklib::details::platform_data),
	  mExitCode(0),
	  mProcessStatus(etNormal),
	  mStandardInput("stdin"),
	  mStandardOutput("stdout"),
	  mStandardError("stderr"),
	  mProgram(program),
	  mParams(params)
{
	mCheckTimer.setInterval(100);
	connect(&mCheckTimer, SIGNAL(timeout()), SLOT(checkOnce()));
}

checklib::RestrictedProcess::~RestrictedProcess()
{

}

bool checklib::RestrictedProcess::isRunning() const
{
	return exitType() == etRunning;
}

/// Запуск процесса
void checklib::RestrictedProcess::start()
{
	if(isRunning()) return;

	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	CreateProcessA(NULL, "szCommandLine", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}

/// Завершает процесс вручную. Тип завершения становится etTerminated
void checklib::RestrictedProcess::terminate()
{

}

/// Ждать завершения процесса
void checklib::RestrictedProcess::wait()
{

}

/// Ждать завершения процесса не более чем @param миллисекунд.
/// @return true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
bool checklib::RestrictedProcess::wait(int milliseconds)
{
	return 0;
}

/// Код возврата.
int checklib::RestrictedProcess::exitCode() const
{
	return mExitCode;
}

/// Тип завершения программы
checklib::ProcessStatus checklib::RestrictedProcess::exitType() const
{
	return mProcessStatus;
}

/// Пиковое значение потребляемой памяти
size_t checklib::RestrictedProcess::peakMemoryUsage() const
{
	return 0;
}

/// Сколько процессорного времени израсходовал процесс
int checklib::RestrictedProcess::CPUTime() const
{
	return 0;
}

checklib::Restrictions checklib::RestrictedProcess::getRestrictions() const
{
	return mRestrictions;
}

void checklib::RestrictedProcess::setRestrictions(const Restrictions &restrictions)
{
	mRestrictions = restrictions;
}

/// Перенаправить стандартный поток ввода в указанный файл.
/// Если stdin, то перенаправления не происходит.
/// Если stdout, то перенавравляется на вывод текущего приложения
void checklib::RestrictedProcess::redirectStandardInput(const QString &fileName)
{
	if(isRunning()) return;
	mStandardInput = fileName;
}

/// Перенаправить стандартный поток вывода в указанный файл.
/// Если stdout, то перенаправления не происходит.
void checklib::RestrictedProcess::redirectStandardOutput(const QString &fileName)
{
	if(isRunning()) return;
	mStandardOutput = fileName;
}

/// Перенаправить стандартный поток ошибок в указанный файл.
/// Если stderr, то перенаправления не происходит.
void checklib::RestrictedProcess::redirectStandardError(const QString &fileName)
{
	if(isRunning()) return;
	mStandardError = fileName;
}

void checklib::RestrictedProcess::redirectStandardStream(checklib::StandardStream stream, const QString &fileName)
{
	switch(stream)
	{
	case ssStdin:
		redirectStandardInput(fileName);
		break;
	case ssStdout:
		redirectStandardOutput(fileName);
		break;
	case ssStderr:
		redirectStandardError(fileName);
		break;
	}
}

void checklib::RestrictedProcess::sendBufferToStandardStream(checklib::StandardStream stream, const QByteArray &data)
{
}

void checklib::RestrictedProcess::checkOnce()
{
	if(!isRunning()) return;
	FILETIME creationTime, exitTime, kernelTime, userTime;
	GetProcessTimes(mPlatformData->process, &creationTime, &exitTime, &kernelTime, &userTime);

	kernelTime.dwHighDateTime += userTime.dwHighDateTime;
	kernelTime.dwLowDateTime += userTime.dwLowDateTime;

	if(((kernelTime.dwHighDateTime << 32) + kernelTime.dwLowDateTime) / 10000000 > mRestrictions.timeLimit)
	{
		// time limit
	}

	PROCESS_MEMORY_COUNTERS counters;
	GetProcessMemoryInfo(mPlatformData->process, &counters, sizeof counters);
}
