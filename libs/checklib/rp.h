#pragma once
#include "rp_types.h"
#include "rp_consts.h"

#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>


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

	QString getProgram() const;
	void setProgram(const QString &program);

	QStringList getParams() const;
	void setParams(const QStringList &params);

	QString currentDirectory() const;
	void setCurrentDirectory(const QString &directory);

	bool isRunning() const;

	// Запуск процесса
	void start();

	// Завершает процесс вручную. Тип завершения становится etTerminated
	void terminate();

	// Ждать завершения процесса.
	void wait();

	// Ждать завершения процесса не более чем @param миллисекунд.
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

	Limits getLimits() const;
	void setLimits(const Limits &restrictions);

	// Возвращает объект в исходное состояние
	// Если процесс запущен, то ничего не происходит
	void reset();

	// Перенаправить стандартный поток ввода в указанный файл.
	// Если ss::stdin, то перенаправления не происходит
	// Если ss::interactive, то будет возможность писать в stdin процесса
	void setStandardInput(const QString &fileName);

	// Перенаправить стандартный поток вывода в указанный файл.
	// Если ss::stdout, то перенаправления не происходит
	// Если ss::interactive, то будет возможность читать из stdout процесса
	void setStandardOutput(const QString &fileName);

	// Перенаправить стандартный поток ошибок в указанный файл.
	// Если ss::stderr, то перенаправления не происходит
	// Если ss::interactive, то будет возможность читать из stderr процесса
	void setStandardError(const QString &fileName);

	// Отправить буфер в указанный стандартный поток.
	// Если процесс не интерактивный или программа не запущена, то ничего не произойдет
	void sendDataToStandardInput(const QString &data, bool newLine = false);

	// Получить буфер из стандартного потока вывода
	void getDataFromStandardOutput(QString &data);

private:

	std::unique_ptr<details::RestrictedProcessImpl> pimpl;
};

}
