#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include "Client.hpp"
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
int main (void)
{
	//SOCK STREAM c'est pour indiquer qu'on va utiliser le protocole TCP
	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD == -1)
	{
		std::cerr << "(Client)echec initialisation du socket" << std::endl;
		exit (1);
	}

	struct sockaddr_in socketAddress;
	char ipBuffer[INET_ADDRSTRLEN];
	socklen_t socketAdressLength = sizeof(socketAddress);
	socketAddress.sin_family = AF_INET;//indique qu'on accepte une adresse IPV4
	socketAddress.sin_port = htons(LISTENING_PORT); //on indique le port par lequel le client ecoute
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);//on accepte n'importe quelle adresse IPV4
	const char*inetReturnCode = inet_ntop(AF_INET, &(socketAddress.sin_addr.s_addr), ipBuffer, INET_ADDRSTRLEN);
	if (inetReturnCode == NULL)
	{
		std::cerr << "(Client)adresse non prise en charge" << std::endl;
		exit (1);
	}
	int connectionStatus = connect(socketFD, (struct sockaddr*)&socketAddress,socketAdressLength);
	if (connectionStatus == -1)
	{
		std::cerr << "(Serveur)echec de la connexion au serveur" << std::endl;
		exit (1);
	}
	const char message[] = "Bonjour, je suis le client!";
	int sentBytes = send(socketFD, message, strlen(message), 0);
	if (sentBytes == -1)
	{
		std::cerr << "(Client)echec de l'envoi du message au serveur" << std::endl;
		exit (1);
	}
	char buffer[BUFSIZ] = {0};
	int reveivedBytes = recv(socketFD, buffer, BUFSIZ, 0);
	if (reveivedBytes == -1)
	{
		std::cerr << "(Client)echec de reception du message du serveur" << std::endl;
		exit (1);
	}
	std::cout << "Serveur: "<< buffer << std::endl;
	close(socketFD);
	return (0);

}
