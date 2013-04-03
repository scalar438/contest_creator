#pragma once
#include <QObject>

namespace testing
{

// Ограничения
struct Restrictions
{
	bool useTimeLimit;
	bool useMemoryLimit;
	size_t memory;
	int time;
};

// Класс, запускающий процесс
class RestrictedProcess : public QObject
{
	Q_OBJECT
public:
};

}
