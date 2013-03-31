#include <ctime>
#include <QDateTime>

int main()
{
	QDateTime cur = QDateTime::currentDateTime();
	while(cur.msecsTo(QDateTime::currentDateTime()) < 1500);
	return 0;
}
