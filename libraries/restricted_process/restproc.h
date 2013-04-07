#pragma once
#include <memory>

#include <QObject>

namespace testing
{

/// Ограничения
struct Restrictions
{
	bool useTimeLimit;
	bool useMemoryLimit;
	int time;
	size_t memory;
};

/// Тип завершения программы. etFailed - внутренняя ошибка тестирования
enum ExitType
{
	etNormal, etTimeLimt, etMemoryLimit, etIdlenessLimit, etRuntimeError, etTerminated, etFailed
};

namespace details
{
struct platform_data;
}

/// @class RestrictedProcess
/// Класс, запускающий процесс с ограничениями
class RestrictedProcess : public QObject
{
	Q_OBJECT
public:
	RestrictedProcess(const QString &program);
	~RestrictedProcess();

	bool isRunning() const;

	/// Запуск процесса
	void start();

	/// Завершает процесс вручную. Тип завершения становится etTerminated
	void terminate();

	/// Ждать завершения процесса
	void wait();

	/// Код возврата.
	int exitCode() const;

	/// Тип завершения программы
	ExitType exitType() const;

	/// Пиковое значение потребляемой памяти
	size_t peakMemoryUsage() const;

	/// Сколько процессорного времени израсходовал процесс
	int CPUTime() const;

	Restrictions getRestrictions() const;
	void setRestrictions(const Restrictions &restrictions);


signals:

private:

	std::shared_ptr<details::platform_data> mPlatformData;
};

}
