﻿#include <cstdlib>
#include <iostream>
#include <cstring>
#include <algorithm>
using namespace std;

int f(int a)
{
	if(a < 0) return 1;
	return f(a) + f(a - 1);
}

int main(int argc, char *argv[])
{
	int val = 0;
	if(argc >= 2)
	{
		val = atoi(argv[1]);
		val = min(val, max(val, 0));
	}

	switch(val)
	{
	case 0:
		{
			int a = rand() % 100;
			a = a - a / 10 - a % 10;
			a /= (a % 9);
			cout << a << endl;
		}
		break;

	case 1:
		{
			cout << f(1000) << endl;
		}
		break;
	case 2:
		{
			cerr << "Normal_exit\n";
		}
		break;
	}
}
