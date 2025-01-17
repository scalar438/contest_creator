﻿#include "process.hpp"
#include "check_stats.hpp"
#include "i_process.hpp"
#include "i_status_updater.hpp"
#include "internal_watcher.hpp"
#include "process_execute_parameters.hpp"
#include <future>

#include "rp.h"

#ifdef CHECKLIB_WINDOWS
#include "rp_win.h"
#endif
#ifdef CHECKLIB_UNIX
#include "rp_linux.h"
#endif

namespace
{
struct Status final : public checklib::details::IStatusUpdater
{
	Status() { m_status = checklib::ProcessStatus::psNotRunning; }

	void set_status(checklib::ProcessStatus status) override { m_status = status; }

	void set_cpu_time(int cpu_time) override { m_cpu_time = cpu_time; }

	void set_peak_memory(int memory) override { m_peak_memory = memory; }

	std::atomic<int> m_cpu_time;
	std::atomic<int> m_peak_memory;
	std::atomic<checklib::ProcessStatus> m_status;
};
} // namespace

struct checklib::Process::Pimpl
{
	Pimpl() { main_watcher = std::make_unique<::details::InternalWatcher>(); }

	std::unique_ptr<::details::InternalWatcher> main_watcher;

	// Platform-dependent process wrapper
	std::unique_ptr<details::IProcess> process;

	std::future<void> async_checker_fut;

	std::shared_ptr<std::atomic_bool> force_exit;

	ProcessExecuteParameters parameters;

	Status m_status;

	void start_internal()
	{
		// copy-paster from a Process constructor. Will be deleted after moving on new api
		force_exit   = std::make_shared<std::atomic_bool>(false);
		auto process = std::make_unique<details::RestrictedProcessImpl>(parameters);
		process->start();
		process           = std::move(process);
		async_checker_fut = std::async(details::async_checker, process.get(), parameters.limits,
		                               &m_status, force_exit);
	}
};

checklib::Process::Process(ProcessExecuteParameters params) : pimpl(new Pimpl)
{
	pimpl->parameters = std::move(params);
	pimpl->force_exit = std::make_shared<std::atomic_bool>(false);
	auto process      = std::make_unique<details::RestrictedProcessImpl>(pimpl->parameters);
	process->start();
	pimpl->process = std::move(process);
	pimpl->async_checker_fut = std::async(details::async_checker, pimpl->process.get(),
                                          pimpl->parameters.limits, &pimpl->m_status, pimpl->force_exit);
}

checklib::Process::Process() : pimpl(new Pimpl)
{
	pimpl->process = details::IProcess::create();
}

checklib::Process::~Process()
{
	// TODO: remove checking after moving new api
	if(*pimpl->force_exit)
		*pimpl->force_exit = false;
	if(pimpl->async_checker_fut.valid())
		pimpl->async_checker_fut.get();
}

void checklib::Process::setProgram(const std::string &program)
{
	set_program(program);
}

void checklib::Process::set_program(std::string program)
{
	pimpl->parameters.program = std::move(program);
}

std::vector<std::string> checklib::RestrictedProcess::params() const
{
	return cmdline();
}

const std::vector<std::string> &checklib::Process::cmdline() const
{
	return pimpl->parameters.cmdline;
}

void checklib::Process::setParams(const std::vector<std::string> &params)
{
	set_cmdline(params);
}

void checklib::Process::set_cmdline(std::vector<std::string> params)
{
    pimpl->parameters.cmdline = std::move(params);
}

std::string checklib::Process::currentDirectory() const
{
	return pimpl->parameters.current_directory;
}

const std::string &checklib::Process::current_directory() const
{
	return pimpl->parameters.current_directory;
}

void checklib::Process::setCurrentDirectory(const std::string &directory)
{
	set_current_directory(directory);
}

void checklib::Process::set_current_directory(std::string directory)
{
	pimpl->parameters.current_directory = std::move(directory);
}

bool checklib::Process::isRunning() const
{
	return !pimpl->process->exit_code().has_value();
}

// Запуск процесса
void checklib::Process::start()
{
	pimpl->start_internal();
}

// Завершает процесс вручную. Тип завершения становится etTerminated
void checklib::Process::terminate()
{
	pimpl->process->kill();
}

// Ждать завершения процесса
void checklib::RestrictedProcess::wait()
{
	// FIXME: this is just for compiling, it is obviuos wrong
	pimpl->process->wait(100500);
}

// Ждать завершения процесса не более чем @param миллисекунд.
// return true если программа завершилась (сама или от превышения лимитов), false - если таймаут
// ожидания
bool checklib::RestrictedProcess::wait(int milliseconds)
{
	return pimpl->process->wait(milliseconds);
}

// Код возврата.
int checklib::RestrictedProcess::exitCode() const
{
	return pimpl->process->exit_code().value_or(-1);
}

// Тип завершения программы
checklib::ProcessStatus checklib::RestrictedProcess::processStatus() const
{
	return pimpl->m_status.m_status;
}

// Пиковое значение потребляемой памяти
int checklib::RestrictedProcess::peakMemoryUsage() const
{
	return pimpl->process->peak_memory_usage();
}

// Сколько процессорного времени израсходовал процесс
int checklib::RestrictedProcess::CPUTime() const
{
	return pimpl->process->cpu_time();
}

checklib::Limits checklib::Process::limits() const
{
	return pimpl->parameters.limits;
}

void checklib::RestrictedProcess::setLimits(const Limits &limits)
{
    set_limits(limits);
}

void checklib::Process::set_limits(Limits limits) {
pimpl->parameters.limits = std::move(limits);}

void checklib::RestrictedProcess::reset()
{
	// TODO: fix it
	//pimpl->reset();
}

// Перенаправить стандартный поток ввода в указанный файл.
// Если ss::stdin, то перенаправления не происходит
// Если ss::interactive, то будет возможность писать в stdin процесса
void checklib::RestrictedProcess::setStandardInput(const std::string &fileName)
{
	pimpl->parameters.standard_input = fileName;
}

// Перенаправить стандартный поток вывода в указанный файл.
// Если ss::stdout, то перенаправления не происходит
// Если ss::interactive, то будет возможность читать из stdout процесса
void checklib::RestrictedProcess::setStandardOutput(const std::string &fileName)
{
    pimpl->parameters.standard_output = fileName;
}

// Перенаправить стандартный поток ошибок в указанный файл.
// Если ss::stderr, то перенаправления не происходит
// Если ss::interactive, то будет возможность читать из stderr процесса
void checklib::RestrictedProcess::setStandardError(const std::string &fileName)
{
    pimpl->parameters.standard_error = fileName;
}

bool checklib::RestrictedProcess::getDataFromStandardOutput(std::string &data)
{
	// TODO: this is temporal
    //return pimpl->getDataFromStandardOutput(data);
    return false;
}

bool checklib::RestrictedProcess::closeStandardInput()
{
	// TODO: this is temporal
    // pimpl->closeStandardInput();
	return false;
}

bool checklib::RestrictedProcess::sendDataToStandardInput(const std::string &data, bool newLine)
{
	// TODO: this is temporal
	//return pimpl->sendDataToStandardInput(data, newLine);
	return false;
}
