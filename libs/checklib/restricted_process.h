#pragma once
#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

namespace checklib
{

/// Ограничения
struct Restrictions
{
	bool useTimeLimit;
	bool useMemoryLimit;
	int timeLimit;
	size_t memoryLimit;
};

/// Тип завершения программы. etFailed - внутренняя ошибка тестирования
enum ProcessStatus
{
	etNormal, etRunning, etTimeLimit, etMemoryLimit, etIdlenessLimit, etRuntimeError, etTerminated, etFailed
};

namespace details
{
struct platform_data;
}

enum StandardStream
{
	ssStdin, ssStdout, ssStderr
};

/// @class RestrictedProcess
/// Класс, запускающий процесс с ограничениями
class RestrictedProcess : public QObject
{
	Q_OBJECT
public:
	/// @param - имя программы, возможно с абсолютным или относительным путем. Задается без расширения
	RestrictedProcess(const QString &program, const QStringList &params = QStringList());
	RestrictedProcess(QObject *parent, const QString &program, const QStringList &params = QStringList());
	~RestrictedProcess();

	bool isRunning() const;

	/// Запуск процесса
	void start();

	/// Завершает процесс вручную. Тип завершения становится etTerminated
	void terminate();

	/// Ждать завершения процесса
	void wait();

	/// Ждать завершения процесса не более чем @param миллисекунд.
	/// @return true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
	bool wait(int milliseconds);

	/// Код возврата.
	int exitCode() const;

	/// Тип завершения программы
	ProcessStatus exitType() const;

	/// Пиковое значение потребляемой памяти
	size_t peakMemoryUsage() const;

	/// Сколько процессорного времени израсходовал процесс
	int CPUTime() const;

	Restrictions getRestrictions() const;
	void setRestrictions(const Restrictions &restrictions);

	/// Перенаправить стандартный поток ввода в указанный файл.
	/// Если stdin, то перенаправления не происходит
	void redirectStandardInput(const QString &fileName);

	/// Перенаправить стандартный поток вывода в указанный файл.
	/// Если stdout, то перенаправления не происходит
	void redirectStandardOutput(const QString &fileName);

	/// Перенаправить стандартный поток ошибок в указанный файл.
	/// Если stderr, то перенаправления не происходит
	void redirectStandardError(const QString &fileName);

	/// Перенаправление указанного стандартного потока.
	void redirectStandardStream(StandardStream stream, const QString &fileName);

	/// Отправить буфер в указанный стандартный поток.
	/// Если этот поток направлен в файл, или программа не запущена, то ничего не произойдет
	void sendBufferToStandardStream(StandardStream stream, const QByteArray &data);

signals:

	/// Вызывается при завершении процесса
	void finished();

private:

	std::shared_ptr<details::platform_data> mPlatformData;

	Restrictions mRestrictions;

	int mExitCode;
	ProcessStatus mProcessStatus;

	long long int mPeakMemory;

	QString mStandardInput;
	QString mStandardOutput;
	QString mStandardError;

	QString mProgram;
	QStringList mParams;

	QTimer mCheckTimer;

private slots:

	/// Проверяет соответствие запущенной программы огранияениям
	void checkOnce();
};

}
