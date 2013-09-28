#include "rp.h"
#ifdef Q_OS_WIN
#include "details/rp_win.h"
#endif
#ifdef Q_OS_LINUX
#include "details/rp_linux.h"
#endif

checklib::RestrictedProcess::RestrictedProcess()
{
	pimpl = std::unique_ptr<details::RestrictedProcessImpl>(new details::RestrictedProcessImpl());
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

QString checklib::RestrictedProcess::currentDirectory() const
{
	return pimpl->currentDirectory();
}

void checklib::RestrictedProcess::setCurrentDirectory(const QString &directory)
{
	pimpl->setCurrentDirectory(directory);
}

bool checklib::RestrictedProcess::isRunning() const
{
	return pimpl->isRunning();
}

// Запуск процесса
void checklib::RestrictedProcess::start()
{
	pimpl->start();
}

// Завершает процесс вручную. Тип завершения становится etTerminated
void checklib::RestrictedProcess::terminate()
{
	pimpl->terminate();
}

// Ждать завершения процесса
void checklib::RestrictedProcess::wait()
{
	pimpl->wait();
}

// Ждать завершения процесса не более чем @param миллисекунд.
// return true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
bool checklib::RestrictedProcess::wait(int milliseconds)
{
	return pimpl->wait(milliseconds);
}

// Код возврата.
int checklib::RestrictedProcess::exitCode() const
{
	return pimpl->exitCode();
}

// Тип завершения программы
checklib::ProcessStatus checklib::RestrictedProcess::processStatus() const
{
	return pimpl->processStatus();
}

// Пиковое значение потребляемой памяти
int checklib::RestrictedProcess::peakMemoryUsage() const
{
	return pimpl->peakMemoryUsage();
}

// Сколько процессорного времени израсходовал процесс
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

void checklib::RestrictedProcess::reset()
{
	pimpl->reset();
}

// Перенаправить стандартный поток ввода в указанный файл.
// Если stdin, то перенаправления не происходит
void checklib::RestrictedProcess::setStandardInput(const QString &fileName)
{
	pimpl->redirectStandardInput(fileName);
}

// Перенаправить стандартный поток вывода в указанный файл.
// Если stdout, то перенаправления не происходит
void checklib::RestrictedProcess::setStandardOutput(const QString &fileName)
{
	pimpl->redirectStandardOutput(fileName);
}

// Перенаправить стандартный поток ошибок в указанный файл.
// Если stderr, то перенаправления не происходит
void checklib::RestrictedProcess::setStandardError(const QString &fileName)
{
	pimpl->redirectStandardError(fileName);
}

void checklib::RestrictedProcess::getDataFromStandardOutput(QByteArray &data)
{
	pimpl->getDataFromStandardOutput(data);
}

void checklib::RestrictedProcess::getDataFromStandardOutput(QString &data)
{
	pimpl->getDataFromStandardOutput(data);
}

// Отправить буфер в указанный стандартный поток.
// Если этот поток направлен в файл, или программа не запущена, то ничего не произойдет
void checklib::RestrictedProcess::sendDataToStandardInput(const QByteArray &data)
{
	pimpl->sendDataToStandardInput(data);
}

void checklib::RestrictedProcess::sendDataToStandardInput(const QString &data, bool newLine)
{
	pimpl->sendDataToStandardInput(data, newLine);
}
