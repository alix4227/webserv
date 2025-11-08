#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iomanip>
#include <iostream>
#include "server.hpp"
int main ()
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
	socketAddress.sin_port = LISTENING_PORT; //on indique le port par lequel le serveur ecoute
	socketAddress.sin_addr.s_addr = INADDR_ANY; // on accepte n'importe quelle adresse de connexion

	int socketAdressLength = sizeof(socketAddress);
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
}
