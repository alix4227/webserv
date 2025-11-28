#include "Server.hpp"

Server::Server()
{
	// 1. Créer la socket
	//SOCK STREAM c'est pour indiquer qu'on va utiliser le protocole TCP
	_socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (_socketFD == -1)
	{
		std::cerr << "(Serveur)echec initialisation du socket" << std::endl;
		exit (1);
	}
	// 2. SO_REUSEADDR → permet restart immédiat
	int opt = 1;
	setsockopt(_socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 3. rend le socketFD NON-BLOQUANT (OBLIGATOIRE!) Un serveur avec plusieurs clients ne veut pas être bloqué sur un seul client.
	//Exemple : si un client n’envoie rien, le serveur doit continuer de gérer les autres.
	fcntl(_socketFD, F_SETFL, O_NONBLOCK);
}

Server::~Server()
{
	
}
std::string Server::getStatusMessage(size_t code) 
{
    switch (code) 
	{
        case 200: return "OK";
        case 201: return "Created";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 504: return "Gateway Timeout";
        default: return "Unknown";
    }
}

std::string Server::getContentType(std::string _uri)
{
	if (_uri.find(".html") != std::string::npos)
		return ("text/html");
	else if (_uri.find(".css") != std::string::npos)
		return ("text/css");
	else if (_uri.find(".png") != std::string::npos)
		return ("image/png");
	else if (_uri.find(".jpg") != std::string::npos || _uri.find(".jpeg") != std::string::npos)
		return ("image/jpeg");
	else
		return ("text/plain");
}
void Server::sendResponse(void)
{
	int status = send(_clientSocket, _response.c_str(), strlen(_response.c_str()), 0);
    if (status == -1) {
        fprintf(stderr, "[Server] Send error to client %d: %s\n", _clientSocket, strerror(errno));
    }
}
void Server::getResponse(void)
{
	std::ostringstream file;
	file << _httpVersion << " " << _status << " " << getStatusMessage(_status) << "\r\n";
	std::map<std::string, std::string>headers;
	headers["Content-Type"] = getContentType(_uri);
	std::ostringstream contentLength;
    contentLength << _contentSize;
	headers["Content-Length"] = contentLength.str();
	headers["Connection"] = "close";
	// permet d'obtenir la date
    time_t now = time(0);
    char* dt = ctime(&now);
    headers["Date"] = std::string(dt);
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		file << it->first << ": " << it->second << "\r\n"; 
	}
	file << "\r\n";
	file << _content;
	_response = file.str();
}
void Server::handleGetMethod(void)
{
	// MODIFICATION: Ajout de la redirection "/" vers "/index.html"
	if (_uri == "/")
		_uri = "/index.html";
	
	std::string path = "./www" + _uri;
	// MODIFICATION: Ajout de log pour debug
	std::cout << "[Server] Trying to open: " << path << std::endl;
	
	if (path.find("..") != std::string::npos)
	{
		// MODIFICATION: Envoyer une vraie réponse 403 au lieu de return vide
		_status = 403;
		_content = "<html><body><h1>403 Forbidden</h1></body></html>";
		_contentSize = _content.size();
		getResponse();
		return ;
	}
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		// MODIFICATION: Envoyer une vraie réponse 404 au lieu de return vide
		_status = 404;
		_content = "<html><body><h1>404 Not Found</h1></body></html>";
		_contentSize = _content.size();
		std::cout << "[Server] File not found: " << path << std::endl;
		getResponse();
		return ;
	}
	file.seekg(0, std::ios::end);
	_contentSize = file.tellg();
	file.seekg(0, std::ios::beg);
	_content.resize(_contentSize);
	file.read(&_content[0], _contentSize);
	// MODIFICATION: Fermeture du fichier après lecture
	file.close();
	_status = 200;
	// MODIFICATION: Ajout de log pour debug
	std::cout << "[Server] File loaded: " << _contentSize << " bytes" << std::endl;
	getResponse();
}
void Server::handleMethod(void)
{
	if (_method == "GET")
		handleGetMethod();
	// else if (_method == "POST")
	// {

	// }
	// else
	// {

	// }
	sendResponse();
}
bool Server::parseRequest()
{
	size_t pos = 0;
	std::string key;
	std::string value;
	std::string body;
	std::string headers;
	std::string line;
	std::istringstream str(_buffer_in);

	str >> _method >> _uri >> _httpVersion;
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return (false);
	if (_httpVersion != "HTTP/1.1")
		return (false);
	size_t q_pos = _uri.find("?");
	if (q_pos != std::string::npos)
	{
		_query = _uri.substr(q_pos + 1);
		_uri = _uri.substr(0, q_pos);
	}
	pos = _buffer_in.find("\r\n");
	headers = _buffer_in.substr(pos + 2);
	std::istringstream h(headers);
	while (getline(h, line))
	{
		std::istringstream map(line);
		map >> key >> value;
		_headers[key] = value;
	}
	pos = _buffer_in.find("\r\n\r\n");
	_body = _buffer_in.substr(pos + 4);
	return (true);
}
void Server::add_to_poll_fds()
{
	pollfd p;
	p.fd = _clientSocket;
	p.events = POLLIN;
	_poll_fds.push_back(p);
 
}
void Server::accept_new_connection(int server_socket, std::vector<pollfd>& poll_fds,
	struct sockaddr_in& socketAddress, socklen_t& socketAdressLength)
{
	(void)poll_fds;
	// MODIFICATION: Suppression du message de bienvenue non-HTTP
    _clientSocket = accept(server_socket,(struct sockaddr*)&socketAddress, &socketAdressLength);
	if (_clientSocket == -1)
	{
		 if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
		std::cerr << "(Serveur)echec d'etablissement de la connexion" << std::endl;
		return ;
	}
	fcntl(_clientSocket, F_SETFL, O_NONBLOCK);
	add_to_poll_fds();
	std::cout << "[Server] Accepted new connection on client socket " <<  _clientSocket << std::endl;
	// MODIFICATION: Suppression de sprintf et send du message de bienvenue
}
void Server::read_data_from_socket(int Socket)
{
	char buffer[BUFSIZ];
    int bytes_read;
	
	// MODIFICATION: Sauvegarder le socket client pour sendResponse
	_clientSocket = Socket;
	
    memset(&buffer, '\0', sizeof buffer);
    bytes_read = recv(Socket, buffer, BUFSIZ, 0);
	if (bytes_read <= 0) 
	{
		if (bytes_read == 0) 
			printf("[%d] Client socket closed connection.\n", Socket);
		else 
			fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
		// MODIFICATION: Fermer le socket en cas d'erreur
		close(Socket);
		return;
    }
	_buffer_in.append(buffer, bytes_read);
	if (_buffer_in.find("\r\n\r\n") != std::string::npos)
	{
		// MODIFICATION: Amélioration du log
		std::cout << "[Server] Received:\n" << _buffer_in << std::endl;
		if (parseRequest())
		{
			handleMethod();
		}
		// MODIFICATION: Décommenter et gérer le cas d'erreur de parsing
		else
		{
			std::cout << "[Server] Parse error - sending 400" << std::endl;
			_status = 400;
			_content = "<html><body><h1>400 Bad Request</h1></body></html>";
			_contentSize = _content.size();
			getResponse();
			sendResponse();
		}
		_buffer_in.clear();
	}
}
int	Server::pollCreation(void)
{
	// Sonde les sockets prêtes (avec timeout de 2 secondes)
    int status = poll(_poll_fds.data(), _poll_fds.size(), 2000);
    if (status == -1) 
	{
        std::cerr << "(Serveur)echec de poll" << std::endl;
        exit(1);
    }
    else if (status == 0) 
	{
        // Aucun descipteur de fichier de socket n'est prêt
        std::cout << "En attente de nouvelles connexions..." << std::endl;
		return (0);
    }
	return (1);
}
void	Server::socketServerCreation(void)
{
	// 4. Configure la socket
	memset(&_socketAddress, 0, sizeof(_socketAddress));
	_socketAddress.sin_family = AF_INET;//indique qu'on veut une adresse IPV4
	_socketAddress.sin_port = htons(LISTENING_PORT); //on indique le port par lequel le serveur ecoute
	_socketAddress.sin_addr.s_addr = htonl(INADDR_ANY); // on accepte n'importe quelle adresse de connexion
	// 5. Liaison de la socket à l'adresse et au port
	_socketAdressLength = sizeof(_socketAddress);
	int bindReturnCode = bind(_socketFD,(struct sockaddr*)&_socketAddress, _socketAdressLength);
	if (bindReturnCode == -1)
	{
		perror("bind");
		// std::cerr << "(Serveur)echec liaison du socket" << std::endl;
		exit (1);
	}
	// 6.Écoute du port via la socket
	if (listen(_socketFD, PENDING_QUEUE_MAXLENGTH) == -1)
	{
		std::cerr << "(Serveur)echec de demarage de l'ecoute des connexions entrantes" << std::endl;
		exit (1);
	}
}

void	Server::pollLoop(void)
{
	socketServerCreation();
	//on cree un tableau de structure pollfd. Une structure pollfd par client.
	pollfd p;
	p.fd = _socketFD;
	//POLLIN est un événement utilisé avec poll() pour indiquer qu’un file descriptor (souvent une socket) a des données disponibles à lire.
	p.events = POLLIN;
	_poll_fds.push_back(p);
    while (1) 
	{ 
		if (!pollCreation())
			continue ;
        // Boucle sur notre tableau de sockets
        for (size_t i = 0; i < _poll_fds.size(); ++i) 
		{
			// Quand poll() est appelé, il remplit revents avec les événements qui se sont produits.
            if (!(_poll_fds[i].revents & POLLIN)) 
			{
				// La socket n'est pas prête à être lue
				// on s'arrête là et on continue la boucle
				continue ;
            }
			// std::cout << "[" << _socketFD << "]" << "Ready for I/O operation\n" << std::endl;
           
            // La socket est prête à être lue !
            if (_poll_fds[i].fd == _socketFD) 
			{
                // La socket est notre socket serveur qui écoute le port
                accept_new_connection(_socketFD, _poll_fds, _socketAddress, _socketAdressLength);
            }
            else 
			{
				// La socket est une socket client, on va la lire
				read_data_from_socket(_poll_fds[i].fd);
            }
        }
    }
}