#include "rp.h"
#ifdef CHECKLIB_WINDOWS
#include "details/rp_win.h"
#endif
#ifdef CHECKLIB_UNIX
#include "details/rp_linux.h"
#endif

checklib::RestrictedProcess::RestrictedProcess()
	: pimpl(new details::RestrictedProcessImpl())
{
	pimpl->finished.connect(this->finished);
}

checklib::RestrictedProcess::~RestrictedProcess()
{
}

std::string checklib::RestrictedProcess::program() const
{
	return pimpl->getProgram();
}

void checklib::RestrictedProcess::setProgram(const std::string &program)
{
	pimpl->setProgram(program);
}

std::vector<std::string> checklib::RestrictedProcess::params() const
{
	return pimpl->getParams();
}

void checklib::RestrictedProcess::setParams(const std::vector<std::string> &params)
{
	pimpl->setParams(params);
}

std::string checklib::RestrictedProcess::currentDirectory() const
{
	return pimpl->currentDirectory();
}

void checklib::RestrictedProcess::setCurrentDirectory(const std::string &directory)
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

checklib::Limits checklib::RestrictedProcess::limits() const
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
// Если ss::stdin, то перенаправления не происходит
// Если ss::interactive, то будет возможность писать в stdin процесса
void checklib::RestrictedProcess::setStandardInput(const std::string &fileName)
{
	pimpl->redirectStandardInput(fileName);
}

// Перенаправить стандартный поток вывода в указанный файл.
// Если ss::stdout, то перенаправления не происходит
// Если ss::interactive, то будет возможность читать из stdout процесса
void checklib::RestrictedProcess::setStandardOutput(const std::string &fileName)
{
	pimpl->redirectStandardOutput(fileName);
}

// Перенаправить стандартный поток ошибок в указанный файл.
// Если ss::stderr, то перенаправления не происходит
// Если ss::interactive, то будет возможность читать из stderr процесса
void checklib::RestrictedProcess::setStandardError(const std::string &fileName)
{
	pimpl->redirectStandardError(fileName);
}

bool checklib::RestrictedProcess::getDataFromStandardOutput(std::string &data)
{
	return pimpl->getDataFromStandardOutput(data);
}

bool checklib::RestrictedProcess::closeStandardInput()
{
	return pimpl->closeStandardInput();
}

bool checklib::RestrictedProcess::sendDataToStandardInput(const std::string &data, bool newLine)
{
	return pimpl->sendDataToStandardInput(data, newLine);
}
