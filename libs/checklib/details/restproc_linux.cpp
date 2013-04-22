#include "restproc.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <QStringList>

struct checklib::details::platform_data
{
	pid_t pid;

};

checklib::RestrictedProcess::RestrictedProcess(const QString &program, const QStringList &params)
	: RestrictedProcess(nullptr, program, params)
{

}

checklib::RestrictedProcess::RestrictedProcess(QObject *parent, const QString &program, const QStringList &params)
	: QObject(parent), mPlatformData(new checklib::details::platform_data),
	  mExitCode(0),
	  mExitType(etNormal),
	  mStandardInput("stdin"),
	  mStandardOutput("stdout"),
	  mStandardError("stderr"),
	  mProgram(program)
{
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
	mPlatformData->pid = fork();
	if(mPlatformData->pid == -1)
	{
		mExitType = etFailed;
		return;
	}
	if(mPlatformData->pid == 0)
	{
		// Дочерний процесс, задает лимиты и запускаем
		if(mRestrinctions.useMemoryLimit)
		{
			rlimit limit;
			limit.rlim_max = limit.rlim_cur = mRestrinctions.memoryLimit + 1024 * 1024;

			setrlimit(RLIMIT_AS, &limit);
		}
		if(mRestrinctions.useTimeLimit)
		{
			rlimit limit;
			limit.rlim_cur = limit.rlim_max = 100 * mRestrinctions.timeLimit;
			setrlimit(RLIMIT_CPU, &limit);
		}
		execl(mProgram.toAscii().data(), 0);
	}
	else
	{
		// Родительский процесс, задаем параметры, необходимые для слежения за дочерним
	}
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

}

/// Код возврата.
int checklib::RestrictedProcess::exitCode() const
{
	return mExitCode;
}

/// Тип завершения программы
checklib::ExitType checklib::RestrictedProcess::exitType() const
{
	return mExitType;
}

/// Пиковое значение потребляемой памяти
size_t checklib::RestrictedProcess::peakMemoryUsage() const
{

}

/// Сколько процессорного времени израсходовал процесс
int checklib::RestrictedProcess::CPUTime() const
{

}

checklib::Restrictions checklib::RestrictedProcess::getRestrictions() const
{

}

void checklib::RestrictedProcess::setRestrictions(const Restrictions &restrictions)
{

}

/// Перенаправить стандартный поток ввода в указанный файл.
/// Если stdin, то перенаправления не происходит.
/// Если stdout, то перенавравляется на вывод текущего приложения
void checklib::RestrictedProcess::redirectStandardInput(const QString &fileName)
{
	mStandardInput = fileName;
}

/// Перенаправить стандартный поток вывода в указанный файл.
/// Если stdout, то без перенаправления
/// Если stdin, то направляяется во ввод текущего процесса.
void checklib::RestrictedProcess::redirectStandardOutput(const QString &fileName)
{
	mStandardOutput = fileName;
}

/// Перенаправить стандартный поток ошибок в указанный файл.
/// Если stderr, то перенаправления не происзодит
void checklib::RestrictedProcess::redirectStandardError(const QString &fileName)
{
	mStandardError = fileName;
}
