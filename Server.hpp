#ifndef SERVER_HPP
#define SERVER_HPP

#define LISTENING_PORT 8080
#define PENDING_QUEUE_MAXLENGTH 1000000 // nombre max de clients qui seront en attente

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include "Server.hpp"
#include <unistd.h>
#include <cstring>
#include<vector>
#include <fcntl.h>
#include <sys/poll.h>

class Server
{
	public:
	Server();
	~Server();
	void socketServerCreation(void);
	void pollLoop(void);
	int pollCreation(void);
	void accept_new_connection(int server_socket, std::vector<pollfd>& poll_fds,
		struct sockaddr_in socketAddress, socklen_t socketAdressLength);
	void add_to_poll_fds(poll_fds, client_fd, poll_count, poll_size);

	private:
	int _socketFD;
	int _clientSocket;
	struct sockaddr_in _socketAddress;
	socklen_t _socketAdressLength;
	std::vector<pollfd>_poll_fds;
};

#endif