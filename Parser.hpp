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
	// private:
	std::string _port;
	std::string _errorPage403;
	std::string _errorPage404;
	std::string _errorPage405;
	std::string _errorPage413;
	std::string _errorPage500;
	std::string _errorPage502;
	std::string _root;
	std::string  _defaultUri;
	size_t _maxBodySize;
	std::string _uploadPath;
	std::string _cgiPath;
	std::vector<std::string>allowedMethod;
	std::vector<std::string>allowedCgiMethod;

	public:
	Parser();
	~Parser();
	bool configParser(std::string filename);
	
	std::string getPort() const;
	std::string getRoot() const;
	std::string getDefaultUri() const;
	size_t getMaxBodySize() const;
	std::string getUploadPath() const;
	std::string getCgiPath() const;
	std::vector<std::string> getAllowedMethod() const;
	std::vector<std::string> getAllowedCgiMethod() const;
	std::string getErrorPage403() const;
	std::string getErrorPage404() const;
	std::string getErrorPage405() const;
	std::string getErrorPage413() const;
	std::string getErrorPage500() const;
	std::string getErrorPage502() const;
};

#endif