#ifndef SERVER_HPP
#define SERVER_HPP

#define LISTENING_PORT 8081
#define PENDING_QUEUE_MAXLENGTH 1000000 // nombre max de clients qui seront en attente

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
#include <sys/stat.h>
#include "Parser.hpp"

class Server
{
	public:
	Server();
	~Server();
	void		socketServerCreation(void);
	bool		pollLoop(std::string configFileName);
	int			pollCreation(void);
	void		accept_new_connection(int server_socket, std::vector<pollfd>& poll_fds,
					struct sockaddr_in& socketAddress, socklen_t& socketAdressLength);
	void		add_to_poll_fds(void);
	void		read_data_from_socket(int Socket);
	bool		parseRequest();
	void		handleMethod();
	void		handleGetMethod();
	void		handlePostMethod();
	void		handleDeleteMethod(void);
	void		sendResponse();
	void		getResponse(void);
	void		handleCgi(void);
	std::string	getStatusMessage(size_t code);
	std::string	getContentType(std::string _uri);
	std::string	getFileName(void);
	void		getEnvp(std::vector<std::string>& env_strings);
	bool is_allowed_method();
	bool is_allowed_cgi_method();
	void getErrorPage();
	std::string findWhichErrorPage();

	private:
	int _socketFD;
	int _clientSocket;
	struct sockaddr_in _socketAddress;
	socklen_t _socketAdressLength;
	std::vector<pollfd>_poll_fds;
	std::string _buffer_in;
	std::string _method;
	std::string _uri;
	std::string _httpVersion;
	std::string _body;
	std::string _query;
	std::string _content;
	std::string _response;
	size_t _contentSize;
	std::map<std::string, std::string>_headers;
	size_t _status;
	Parser* parser;
	
};

#endif