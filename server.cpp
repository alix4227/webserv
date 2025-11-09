#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include "server.hpp"
#include <unistd.h>
#include <cstring>
#include<vector>
#include <sys/poll.h>
int main (void)
{
	//SOCK STREAM c'est pour indiquer qu'on va utiliser le protocole TCP
	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD == -1)
	{
		std::cerr << "(Serveur)echec initialisation du socket" << std::endl;
		exit (1);
	}

	struct sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;//indique qu'on veut une adresse IPV4
	socketAddress.sin_port = htons(LISTENING_PORT); //on indique le port par lequel le serveur ecoute
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY); // on accepte n'importe quelle adresse de connexion

	socklen_t socketAdressLength = sizeof(socketAddress);
	int bindReturnCode = bind(socketFD,(struct sockaddr*)&socketAddress, socketAdressLength);
	if (bindReturnCode == -1)
	{
		std::cerr << "(Serveur)echec liaison du socket" << std::endl;
		exit (1);
	}
	if (listen(socketFD, PENDING_QUEUE_MAXLENGTH) == -1)
	{
		std::cerr << "(Serveur)echec de demarage de l'ecoute des connexions entrantes" << std::endl;
		exit (1);
	}
	std::cout << "En attente de nouvelles connexions..." << std::endl;

	std::vector<pollfd>poll_fds;
	pollfd p;
	p.fd = socketFD;
	p.events = POLLIN;
	poll_fds.push_back(p);









	int connectedSocketFD = accept(socketFD,(struct sockaddr*)&socketAddress, &socketAdressLength);
	if (connectedSocketFD == -1)
	{
		std::cerr << "(Serveur)echec d'etablissement de la connexion" << std::endl;
		exit (1);
	}
	//On va utiliser la memoire tampon pour receptionner le message
	char buffer[BUFFER_SIZE] = {0};
	int reveivedBytes = recv(connectedSocketFD, buffer, BUFFER_SIZE, 0);
	if (reveivedBytes == -1)
	{
		std::cerr << "(Serveur)echec de reception du message du client" << std::endl;
		exit (1);
	}
	std::cout << "Client: "<< buffer << std::endl;

	const char message[] = "Bonjour, je suis le serveur!";
	int sentBytes = send(connectedSocketFD, message, strlen(message), 0);
	if (sentBytes == -1)
	{
		std::cerr << "(Serveur)echec de l'envoi du message au client" << std::endl;
		exit (1);
	}
	close(connectedSocketFD);
	close(socketFD);
	return (0);

}
