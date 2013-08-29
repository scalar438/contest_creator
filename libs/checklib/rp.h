#pragma once
#include "rp_types.h"

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
class RestrictedProcess : public QObject
{
	Q_OBJECT
public:
	RestrictedProcess(QObject *parent = nullptr);
	~RestrictedProcess();

	QString getProgram() const;
	void setProgram(const QString &program);

	QStringList getParams() const;
	void setParams(const QStringList &params);

	bool isRunning() const;

	// Запуск процесса
	void start();

	// Завершает процесс вручную. Тип завершения становится etTerminated
	void terminate();

	// Ждать завершения процесса
	void wait();

	// Ждать завершения процесса не более чем @param миллисекунд.
	// true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
	bool wait(int milliseconds);

	// Код возврата.
	int exitCode() const;

	// Тип завершения программы
	ProcessStatus exitType() const;

	// Пиковое значение потребляемой памяти
	// Если процесс не запущен, возвращает значение для последнего запуска
	int peakMemoryUsage() const;

	// Сколько процессорного времени израсходовал процесс
	// Если процесс не запущен, возвращает значение для последнего запуска
	int CPUTime() const;

	Limits getLimits() const;
	void setLimits(const Limits &restrictions);

	// Перенаправить стандартный поток ввода в указанный файл.
	// Если stdin, то перенаправления не происходит
	void redirectStandardInput(const QString &fileName);

	// Перенаправить стандартный поток вывода в указанный файл.
	// Если stdout, то перенаправления не происходит
	void redirectStandardOutput(const QString &fileName);

	// Перенаправить стандартный поток ошибок в указанный файл.
	// Если stderr, то перенаправления не происходит
	void redirectStandardError(const QString &fileName);

	// Отправить буфер в указанный стандартный поток.
	// Если этот поток направлен в файл, или программа не запущена, то ничего не произойдет
	void sendBufferToStandardStream(StandardStream stream, const QByteArray &data);

signals:

	// Вызывается при завершении процесса
	void finished();

private:

	std::unique_ptr<details::RestrictedProcessImpl> pimpl;
};

}
