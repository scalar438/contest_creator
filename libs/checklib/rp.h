#pragma once
#include "rp_types.h"
#include "rp_consts.h"

#include <memory>
#include <vector>
#include <string>

#include <boost/signals2.hpp>
#include <boost/filesystem.hpp>

namespace checklib
{

namespace details
{
class RestrictedProcessImpl;
}

// Класс, запускающий процесс с ограничениями
class RestrictedProcess
{
public:
	RestrictedProcess();
	~RestrictedProcess();

	std::string program() const;
	void setProgram(const std::string &program);

	std::vector<std::string> params() const;
	void setParams(const std::vector<std::string> &params);

	std::string currentDirectory() const;
	void setCurrentDirectory(const std::string &directory);

	bool isRunning() const;

	// Запуск процесса
	void start();

	// Завершает процесс вручную. Тип завершения становится etTerminated
	void terminate();

	// Ждать завершения процесса.
	void wait();

	// Ждать завершения процесса не более чем указанное количество миллисекунд.
	// true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
	bool wait(int milliseconds);

	// Код возврата.
	int exitCode() const;

	// Тип завершения программы
	ProcessStatus processStatus() const;

	// Пиковое значение потребляемой памяти
	// Если процесс не запущен, возвращает значение для последнего запуска
	int peakMemoryUsage() const;

	// Сколько процессорного времени израсходовал процесс
	// Если процесс не запущен, возвращает значение для последнего запуска
	int CPUTime() const;

	Limits limits() const;
	void setLimits(const Limits &restrictions);

	// Возвращает объект в исходное состояние
	// Если процесс запущен, то ничего не происходит
	void reset();

	// Перенаправить стандартный поток ввода в указанный файл.
	// Если ss::stdin, то перенаправления не происходит
	// Если ss::interactive, то будет возможность писать в stdin процесса
	void setStandardInput(const std::string &fileName);

	// Перенаправить стандартный поток вывода в указанный файл.
	// Если ss::stdout, то перенаправления не происходит
	// Если ss::interactive, то будет возможность читать из stdout процесса
	void setStandardOutput(const std::string &fileName);

	// Перенаправить стандартный поток ошибок в указанный файл.
	// Если ss::stderr, то перенаправления не происходит
	// Если ss::interactive, то будет возможность читать из stderr процесса
	void setStandardError(const std::string &fileName);

	// Отправить буфер в указанный стандартный поток.
	// Если процесс не интерактивный или программа не запущена, то ничего не произойдет
	bool sendDataToStandardInput(const std::string &data, bool newLine = false);

	// Получить буфер из стандартного потока вывода. Выдает целиком строку (без символа перевода строки).
	bool getDataFromStandardOutput(std::string &data);

	boost::signals2::signal<void(int)> finished;

private:

	std::unique_ptr<details::RestrictedProcessImpl> pimpl;
};

}
