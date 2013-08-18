#include <ctime>
#include <QDateTime>

int main()
{
	QDateTime cur = QDateTime::currentDateTime();
	while(cur.msecsTo(QDateTime::currentDateTime()) < 2500);
	return 0;
}
