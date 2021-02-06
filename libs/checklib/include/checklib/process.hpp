#pragma once
#include "process_events.hpp"
#include "rp_consts.h"
#include "rp_types.h"
#include "process_execute_parameters.hpp"

#include <memory>
#include <string>
#include <vector>

namespace checklib
{

/// Класс, запускающий процесс с ограничениями
class Process
{
public:
	/// Create and start the process
	Process(ProcessExecuteParameters parameters);
	[[deprecated]] Process();
	~Process();

	const std::string &program() const;
	void set_program(std::string program);
	[[deprecated]] void setProgram(const std::string &program);

    [[deprecated]] std::vector<std::string> params() const;
    [[deprecated]] void setParams(const std::vector<std::string> &params);

	const std::vector<std::string> &cmdline() const;
	void set_cmdline(std::vector<std::string> cmdline);

	const std::string& current_directory() const;
	void set_current_directory(std::string directory);

	[[deprecated]] std::string currentDirectory() const;
	[[deprecated]] void setCurrentDirectory(const std::string &directory);

	bool isRunning() const;

	/// Запуск процесса
	void start();

	/// Завершает процесс вручную.
	/// Если процесс запущен, тип завершения становится etTerminated
	/// Иначе ничего не происходит
	void terminate();

	/// Ждать завершения процесса. После выхода isRunning() == false
	void wait();

	/// Ждать завершения процесса не более чем указанное количество миллисекунд.
	/// @param milliseconds время ожидания, в милисекундах
	/// @return true - если программа завершилась (сама или от превышения лимитов),
	///         false - если таймаут ожидания
	bool wait(int milliseconds);

	/// Код возврата. Если программа выполянется, то результат не определен.
	int exitCode() const;

	/// Тип завершения программы
	ProcessStatus processStatus() const;

	/// Пиковое значение потребляемой памяти
	/// Если процесс не запущен, возвращает значение для последнего запуска
	int peakMemoryUsage() const;

	/// Сколько процессорного времени израсходовал процесс
	/// Если процесс не запущен, возвращает значение для последнего запуска
	int CPUTime() const;

	Limits limits() const;
	[[deprecated]] void setLimits(const Limits &restrictions);
	void set_limits(Limits limits);

	/// Возвращает объект в стартовое состояние
	/// Если процесс запущен, то завершает его
	/// Сбрасывает сохраненные peakMemoryUsage, CPUTime, processStatus и exitCode
	void reset();

	/// Перенаправить стандартный поток ввода в указанный файл.
	/// @param fileName Название файла, откуда будет читать стандартный поток ввода
	/// Если ss::stdin, то перенаправления не происходит
	/// Если ss::interactive, то будет возможность писать в stdin процесса
	/// (см. RestrictedProcess::sendDataToStandardInput)
	void setStandardInput(const std::string &fileName);

	/// Перенаправить стандартный поток вывода в указанный файл.
	/// @param fileName Название файла, куда будет перенаправляться стандартный поток выхода
	/// Если ss::stdout, то перенаправления не происходит
	/// Если ss::interactive, то будет возможность читать из stdout процесса
	/// (см. RestrictedProcess::getDataFromStandardOutput)
	void setStandardOutput(const std::string &fileName);

	/// Перенаправить стандартный поток ошибок в указанный файл.
	/// @param fileName Название файла, куда будет перенаправляться стандартный поток ошибок
	/// Если ss::stderr, то перенаправления не происходит
	void setStandardError(const std::string &fileName);

	/// Отправить буфер в указанный стандартный поток.
	/// @param data строка, отправляемая в стандартный поток ввода
	/// @param newLine добавлять или нет к выводу символ перевода строки.
	/// Если стандартный поток ввода не интерактивный или программа не запущена, то ничего не
	/// произойдет
	bool sendDataToStandardInput(const std::string &data, bool newLine = false);

	/// Получает строку из стандартного потока вывода процесса.
	/// Выдает полную строку, без символа перевода строки
	bool getDataFromStandardOutput(std::string &data);

	/// Закрывает стандартный поток ввода в интерактивный процесс.
	/// Если поток ввода закрыть не удалось, или поток ввода не интерактивный, возвращает true,
	/// иначе false
	bool closeStandardInput();

	void set_watcher(std::shared_ptr<IProcessEvents> watcher);

private:
	struct Pimpl;

	std::unique_ptr<Pimpl> pimpl;
};

} // namespace checklib
