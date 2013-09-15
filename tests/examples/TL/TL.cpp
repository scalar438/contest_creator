#include <ctime>
#include <iostream>

int main()
{
	time_t st = time(NULL);
	while(true)
	{
		time_t cur = time(NULL);
		if(difftime(cur, st) > 15) return 0;
	}
}
