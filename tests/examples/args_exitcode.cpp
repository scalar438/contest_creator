#include <string>
#include <fstream>
#include <cstdlib>
using namespace std;

int main(int argc, char *argv[])
{
	if(argc == 1) return 42;
	else return atoi(argv[1]);
}
