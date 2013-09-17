#include <string>
#include <fstream>
using namespace std;

int main(int argc, char *argv[])
{
	std::ofstream out("args_out.txt");
	out << argc << std::endl;
	for(int i = 0; i < argc; ++i)
	{
		out << argv[i] << "\n";
	}
}
