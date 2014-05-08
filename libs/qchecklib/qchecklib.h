#pragma once

#include "checklib/checklib.h"

#include <QObject>

class QRestrictedProcess : public QObject
{
	Q_OBJECT
public:
	QRestrictedProcess(QObject *parent = nullptr);
	virtual ~QRestrictedProcess();

private:

	checklib::RestrictedProcess mRp;
};
