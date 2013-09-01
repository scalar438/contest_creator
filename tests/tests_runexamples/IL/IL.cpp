#include <QThread>

class Sleeper : QThread
{
public:
	static void sleep(unsigned long a)
	{
		QThread::sleep(a);
	}
};

int main()
{
	Sleeper::sleep(10);
	return 0;
}
