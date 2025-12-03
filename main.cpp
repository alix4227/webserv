#include "Server.hpp"
#include "Parser.hpp"
int main(int ac, char** av)
{
	if (ac < 2)
	{
		std::cout << "Wrong input" << std::endl;
		return (1);
	}
	std::string configFileName = av[1];
	Server Server;
	// Parser Parser;
	// if (!Parser.configParser(configFileName))
	// 	return (1);
	Server.pollLoop(configFileName);
}