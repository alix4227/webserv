#ifndef PARSER_HPP
#define PARSER_HPP


#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <fcntl.h>
#include <map>
#include <fstream>
#include <sstream>
#include <sys/poll.h>
# include <sys/wait.h>
#include <algorithm>

class Parser
{
	private:
	std::string _port;
	std::string _root;
	std::string  _defaultUri;
	size_t _maxBodySize;
	std::string _uploadPath;
	std::vector<std::string>allowedMethod;
	std::vector<std::string>allowedCgiMethod;

	public:
	Parser();
	~Parser();
	bool configParser(std::string filename);
};

#endif