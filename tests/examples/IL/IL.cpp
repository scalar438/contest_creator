#ifdef OS_WIN32
#include <Windows.h>
#define sleep(x) Sleep((x) * 1000)
#else
#include <unistd.h>
#endif

int main()
{
	sleep(15);
}
