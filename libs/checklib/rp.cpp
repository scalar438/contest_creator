#include "rp.h"
#ifdef Q_OS_WIN
#include "details/rp_win.h"
#endif

checklib::RestrictedProcess::RestrictedProcess(QObject *parent)
	: QObject(parent)
{
	pimpl = std::move(std::unique_ptr<details::RestrictedProcessImpl>(new details::RestrictedProcessImpl(this)));
}

checklib::RestrictedProcess::~RestrictedProcess()
{
}

QString checklib::RestrictedProcess::getProgram() const
{
	return pimpl->getProgram();
}

void checklib::RestrictedProcess::setProgram(const QString &program)
{
	if(!pimpl) qDebug() << "WTF";
	pimpl->setProgram(program);
}

QStringList checklib::RestrictedProcess::getParams() const
{
	return pimpl->getParams();
}

void checklib::RestrictedProcess::setParams(const QStringList &params)
{
	pimpl->setParams(params);
}

bool checklib::RestrictedProcess::isRunning() const
{
	return pimpl->isRunning();
}

/// Запуск процесса
void checklib::RestrictedProcess::start()
{
	pimpl->start();
}

/// Завершает процесс вручную. Тип завершения становится etTerminated
void checklib::RestrictedProcess::terminate()
{
	pimpl->terminate();
}

/// Ждать завершения процесса
void checklib::RestrictedProcess::wait()
{
	pimpl->run();
//	pimpl->wait();
}

/// Ждать завершения процесса не более чем @param миллисекунд.
/// @return true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
bool checklib::RestrictedProcess::wait(int milliseconds)
{
	return pimpl->wait(milliseconds);
}

/// Код возврата.
int checklib::RestrictedProcess::exitCode() const
{
	return pimpl->exitCode();
}

/// Тип завершения программы
checklib::ProcessStatus checklib::RestrictedProcess::exitType() const
{
	return pimpl->processStatus();
}

/// Пиковое значение потребляемой памяти
int checklib::RestrictedProcess::peakMemoryUsage() const
{
	return pimpl->peakMemoryUsage();
}

/// Сколько процессорного времени израсходовал процесс
int checklib::RestrictedProcess::CPUTime() const
{
	return pimpl->CPUTime();
}

checklib::Limits checklib::RestrictedProcess::getLimits() const
{
	return pimpl->getLimits();
}

void checklib::RestrictedProcess::setLimits(const Limits &restrictions)
{
	pimpl->setLimits(restrictions);
}

/// Перенаправить стандартный поток ввода в указанный файл.
/// Если stdin, то перенаправления не происходит
void checklib::RestrictedProcess::redirectStandardInput(const QString &fileName)
{
	pimpl->redirectStandardInput(fileName);
}

/// Перенаправить стандартный поток вывода в указанный файл.
/// Если stdout, то перенаправления не происходит
void checklib::RestrictedProcess::redirectStandardOutput(const QString &fileName)
{
	pimpl->redirectStandardOutput(fileName);
}

/// Перенаправить стандартный поток ошибок в указанный файл.
/// Если stderr, то перенаправления не происходит
void checklib::RestrictedProcess::redirectStandardError(const QString &fileName)
{
	pimpl->redirectStandardError(fileName);
}

/// Отправить буфер в указанный стандартный поток.
/// Если этот поток направлен в файл, или программа не запущена, то ничего не произойдет
void checklib::RestrictedProcess::sendBufferToStandardStream(StandardStream stream, const QByteArray &data)
{
	pimpl->sendBufferToStandardStream(stream, data);
}

