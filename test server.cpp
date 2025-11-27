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

void read_data_from_socket(size_t i, std::vector<pollfd>& poll_fds)
{
    char buffer[BUFSIZ];
    char msg_to_send[BUFSIZ];
    int bytes_read;
    int status;
    int dest_fd;
    int client_fd;
	std::string buffer_in;

    client_fd = poll_fds[i].fd;
    memset(&buffer, '\0', sizeof buffer);
    bytes_read = recv(client_fd, buffer, BUFSIZ, 0);
    if (bytes_read <= 0) 
	{
        if (bytes_read == 0) {
            printf("[%d] Client socket closed connection.\n", client_fd);
        }
        else {
            fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
        }
		_state = CLIENT_CLOSE;//FERME LE SOCKET
        // del_from_poll_fds(poll_fds, i, poll_count);
    }
	buffer_in.append(buffer, bytes_read);
	if (buffer_in.find("\r\n\r\n"))
	{
		if (request.parse(buffer_in))
		{
			_state = CLIENT_PROCESS;
		}
		else
		{
			_bufferOut = _response.set_status(400);   
			_state = CLIENT_WRITE;
		}
	}
	break ;
}


void accept_new_connection(int server_socket, std::vector<pollfd>& poll_fds, struct sockaddr_in socketAddress, socklen_t socketAdressLength)
{
	(void)poll_fds;
	char msg_to_send[BUFSIZ];
    int clientSocket = accept(server_socket,(struct sockaddr*)&socketAddress, &socketAdressLength);
	if (clientSocket == -1)
	{
		 if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;  // Pas de connexion disponible
		std::cerr << "(Serveur)echec d'etablissement de la connexion" << std::endl;
		return ;
	}
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
    // add_to_poll_fds(poll_fds, client_fd, poll_count, poll_size);
	// std::cout << "[Server] Accepted new connection on client socket " <<  clientSocket << std::endl;
	// sprintf(msg_to_send, "Welcome. You are client fd [%d]\n", clientSocket);
    // int status = send(clientSocket, msg_to_send, strlen(msg_to_send), 0);
    // if (status == -1) 
	// {
	// 	std::cout << "[Server] Send error to client " << clientSocket << ": " << "message not send" << std::endl;
	// 	return ;
    // }

}

int main (void)
{
	// 1. Créer la socket
	//SOCK STREAM c'est pour indiquer qu'on va utiliser le protocole TCP
	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD == -1)
	{
		std::cerr << "(Serveur)echec initialisation du socket" << std::endl;
		exit (1);
	}
	// 2. SO_REUSEADDR → permet restart immédiat
	int opt = 1;
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. rend le socketFD NON-BLOQUANT (OBLIGATOIRE!) Un serveur avec plusieurs clients ne veut pas être bloqué sur un seul client.
	//Exemple : si un client n’envoie rien, le serveur doit continuer de gérer les autres.
    fcntl(socketFD, F_SETFL, O_NONBLOCK);

	// 4. Configure la socket
	struct sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;//indique qu'on veut une adresse IPV4
	socketAddress.sin_port = htons(LISTENING_PORT); //on indique le port par lequel le serveur ecoute
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY); // on accepte n'importe quelle adresse de connexion
	// 5. Liaison de la socket à l'adresse et au port
	socklen_t socketAdressLength = sizeof(socketAddress);
	int bindReturnCode = bind(socketFD,(struct sockaddr*)&socketAddress, socketAdressLength);
	if (bindReturnCode == -1)
	{
		std::cerr << "(Serveur)echec liaison du socket" << std::endl;
		exit (1);
	}
	// 6.Écoute du port via la socket
	if (listen(socketFD, PENDING_QUEUE_MAXLENGTH) == -1)
	{
		std::cerr << "(Serveur)echec de demarage de l'ecoute des connexions entrantes" << std::endl;
		exit (1);
	}
	
	std::vector<pollfd>poll_fds;
	pollfd p;
	p.fd = socketFD;
	//POLLIN est un événement utilisé avec poll() pour indiquer qu’un file descriptor (souvent une socket) a des données disponibles à lire.
	p.events = POLLIN;
	poll_fds.push_back(p);
	
    while (1) 
	{ 
        // Sonde les sockets prêtes (avec timeout de 2 secondes)
        int status = poll(poll_fds.data(), poll_fds.size(), 2000);
        if (status == -1) 
		{
           std::cerr << "(Serveur)echec de poll" << std::endl;
            exit(1);
        }
        else if (status == 0) 
		{
            // Aucun descipteur de fichier de socket n'est prêt
            std::cout << "En attente de nouvelles connexions..." << std::endl;
            continue;
        }

        // Boucle sur notre tableau de sockets
		
        for (size_t i = 0; i < poll_fds.size(); ++i) 
		{
			// Quand poll() est appelé, il remplit revents avec les événements qui se sont produits.
            if (!(poll_fds[i].revents & POLLIN)) 
			{
				// La socket n'est pas prête à être lue
				// on s'arrête là et on continue la boucle
				continue ;
            }
			std::cout << "[" << socketFD << "]" << "Ready for I/O operation\n" << std::endl;
           
            // La socket est prête à être lue !
            if (poll_fds[i].fd == socketFD) {
                // La socket est notre socket serveur qui écoute le port
                accept_new_connection(socketFD, poll_fds, socketAddress, socketAdressLength);
            }
            // else {
            //     // La socket est une socket client, on va la lire
            //     read_data_from_socket(i, &poll_fds, poll_fds.size(), socketFD);
            // }
        }
    }	








	// int connectedSocketFD = accept(socketFD,(struct sockaddr*)&socketAddress, &socketAdressLength);
	// if (connectedSocketFD == -1)
	// {
	// 	std::cerr << "(Serveur)echec d'etablissement de la connexion" << std::endl;
	// 	exit (1);
	// }
	// //On va utiliser la memoire tampon pour receptionner le message
	// char buffer[BUFSIZ] = {0};
	// int reveivedBytes = recv(connectedSocketFD, buffer, BUFSIZ, 0);
	// if (reveivedBytes == -1)
	// {
	// 	std::cerr << "(Serveur)echec de reception du message du client" << std::endl;
	// 	exit (1);
	// }
	// std::cout << "Client: "<< buffer << std::endl;

	// const char message[] = "Bonjour, je suis le serveur!";
	// int sentBytes = send(connectedSocketFD, message, strlen(message), 0);
	// if (sentBytes == -1)
	// {
	// 	std::cerr << "(Serveur)echec de l'envoi du message au client" << std::endl;
	// 	exit (1);
	// }
	// close(connectedSocketFD);
	close(socketFD);
	return (0);

}
