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
	if (parser)
		delete parser;
}
std::string Server::findWhichErrorPage()
{
	switch (_status) 
	{
        case 403: return parser->getErrorPage403();
        case 404: return parser->getErrorPage404();
        case 405: return parser->getErrorPage405();
        case 413: return parser->getErrorPage413();
        case 500: return parser->getErrorPage500();
        case 502: return parser->getErrorPage502();
        default: return parser->getErrorPage500();
    }
}
void Server::getErrorPage()
{
	std::ifstream file(findWhichErrorPage().c_str(), std::ios::binary);
	if (!file.is_open())
		return ;
	file.seekg(0, std::ios::end);
	_contentSize = file.tellg();
	if (_contentSize > parser->getMaxBodySize())
	{
		_status = 413;
		_content = "<html><body><h1>413 Payload Too Large</h1></body></html>";
		_contentSize = _content.size();
		getResponse();
		return ;
	}
	file.seekg(0, std::ios::beg);
	_content.resize(_contentSize);
	file.read(&_content[0], _contentSize);
	file.close();
	
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

std::string Server::getContentType(std::string uri)
{
	size_t pos = uri.find_last_of('.');
	if (pos == std::string::npos)
		return "text/html; charset=utf-8";
	std::string ext = uri.substr(pos);
	if (ext == ".html" || ext == ".htm")
		return "text/html; charset=utf-8";
	else if (ext == ".css")
		return "text/css; charset=utf-8";
	else if (uri.find(".pdf") != std::string::npos)
		return ("application/pdf");
	else if (ext == ".js")
		return "application/javascript";
	else if (ext == ".png")
		return "image/png";
	else if (ext == ".jpg" || ext == ".jpeg")
		return "image/jpeg";
	else if (ext == ".gif")
		return "image/gif";
	else if (ext == ".svg")
		return "image/svg+xml";
	else if (ext == ".ico")
		return "image/x-icon";
	else
		return "text/html; charset=utf-8";
}

void Server::sendResponse(void)
{
	int status = send(_clientSocket, _response.c_str(), _response.size(), 0);
    if (status == -1) {
        fprintf(stderr, "[Server] Send error to client %d: %s\n", _clientSocket, strerror(errno));
    }
}

void Server::getResponse(void)
{
	std::ostringstream file;
	file << _httpVersion << " " << _status << " " << getStatusMessage(_status) << "\r\n";
	file << "Content-Type: " << getContentType(_uri) << "\r\n";
	file << "Content-Length: " << _contentSize << "\r\n";
	file << "Connection: close\r\n";
	time_t now = time(0);
	char* dt = ctime(&now);
	std::string dateStr(dt);
	if (!dateStr.empty() && dateStr[dateStr.length()-1] == '\n')// Retirer le \n final de ctime
		dateStr.erase(dateStr.length()-1);
	file << "Date: " << dateStr << "\r\n";
	file << "\r\n";
	_response.clear();
	_response = file.str();
	_response.append(_content);//on rajoute le contenu du fichier
}

std::string Server::getFileName(void)
{
    size_t pos = _body.find("filename=\"");
    if (pos == std::string::npos) 
        return ("");
    pos += strlen("filename=\""); // Se placer après 'filename="'
    size_t posEnd = _body.find("\"", pos); // Trouver le " de fermeture
    if (posEnd == std::string::npos) 
        return ("");
    return (_body.substr(pos, posEnd - pos));
}

void Server::handlePostMethod()
{
	std::string path = parser->getUploadPath() + getFileName();
	std::ofstream uploadfile(path.c_str(), std::ios::binary);
	size_t pos = _body.find("Content-Type:");
	if (pos == std::string::npos) 
		return;
	_body = _body.substr(pos);
	if (_body.size() > parser->getMaxBodySize())//verification de la taille du body
	{
		_status = 413;
		getErrorPage();
       	std::ostringstream file;
		file << _httpVersion << " " << _status << " " << getStatusMessage(_status) << "\r\n";
		file << "Content-Type: text/html\r\n";
		file << "Content-Length: " << _contentSize << "\r\n";
		file << "Connection: close\r\n";
		file << "\r\n";
		file << _content;
		_response = file.str();
        return ;
	}
	pos = _body.find("\r\n\r\n");
	pos += 4;
	size_t posEnd = _body.find("\r\n------");
	if (posEnd == std::string::npos) 
		return;
	_body = _body.substr(pos, posEnd - pos);
	if (uploadfile.is_open())
	{
		uploadfile.write(_body.c_str(),_body.length());//j'ecris le contenu du body dans le fichier
		uploadfile.close();
		if (!uploadfile.fail())
		{
			_status = 201;
			_content = "<html><body><h1>201 Created - File Uploaded Successfully</h1></body></html>";
			_contentSize = _content.size();
		}
		else
		{
			_status = 500;
			getErrorPage();
		}
	}
	std::ostringstream file;
	file << _httpVersion << " " << _status << " " << getStatusMessage(_status) << "\r\n";
	file << "Content-Type: text/html\r\n";
	file << "Content-Length: " << _contentSize << "\r\n";
	file << "Connection: close\r\n";
	file << "\r\n";
	file << _content;
	_response = file.str();
}

void Server::handleGetMethod(void)
{
	//redirection "/" vers "/index.html"
	if (_uri == "/")
		_uri = parser->getDefaultUri();
	
	std::string path = "./www" + _uri;

	std::cout << "[Server] Trying to open: " << path << std::endl;
	
	if (path.find("..") != std::string::npos)
	{
		_status = 403;
		getErrorPage();
		getResponse();
		return ;
	}
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		_status = 404;
		getErrorPage();
		std::cout << "[Server] File not found: " << path << std::endl;
		getResponse();
		return ;
	}
	file.seekg(0, std::ios::end);
	_contentSize = file.tellg();
	if (_contentSize > parser->getMaxBodySize())
	{
		_status = 413;
		getErrorPage();
		getResponse();
		return ;
	}
	file.seekg(0, std::ios::beg);
	_content.resize(_contentSize);
	file.read(&_content[0], _contentSize);
	file.close();
	_status = 200;
	std::cout << "[Server] File loaded: " << _contentSize << " bytes" << std::endl;
	getResponse();
}

void Server::handleDeleteMethod(void)
{
	std::string path = parser->getRoot() + _uri;
	std::cout << "[Server] Trying to open: " << path << std::endl;
	if (path.find("..") != std::string::npos)
	{
		_status = 403;
		getErrorPage();
		getResponse();
		return ;
	}
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		file.close();
		_status = 404;
		getErrorPage();
		std::cout << "[Server] File not found: " << path << std::endl;
		getResponse();
		return ;
	}
	file.close();
	if (std::remove(path.c_str()) == 0) 
	{
		_status = 200;
		_content = "<html><body><h1>200 OK - File Deleted Successfully</h1></body></html>";
		_contentSize = _content.size();
	}
    else 
	{
    	_status = 500;
		getErrorPage();
		std::cout << "[Server] Error deleting file: " << strerror(errno) << std::endl;
    }
	getResponse();
}

void Server::handleMethod(void)
{
	if (_method == "GET")
		handleGetMethod();
	else if (_method == "POST")
		handlePostMethod();
	else if (_method == "DELETE")
        handleDeleteMethod();
    else
    {
        _status = 405;
        _content = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        _contentSize = _content.size();
        getResponse();
    }
	sendResponse();
}
bool Server::is_allowed_cgi_method()
{
	size_t i = 0;
	size_t size = parser->getAllowedCgiMethod().size();
	while (i < size)
	{
		if (parser->getAllowedCgiMethod()[i] == _method)
			return (true);
		i++;
	}
	return (false);
}
bool Server::is_allowed_method()
{
	size_t i = 0;
	size_t size = parser->getAllowedMethod().size();
	while (i < size)
	{
		if (parser->getAllowedMethod()[i] == _method)
			return (true);
		i++;
	}
	return (false);
}
bool Server::parseRequest()
{
	size_t pos = 0;
	std::string key;
	std::string value;
	std::string value2;
	std::string body;
	std::string headers;
	size_t header_end;
	std::string line;
	std::istringstream str(_buffer_in);

	str >> _method >> _uri >> _httpVersion;
	if (!is_allowed_method())
	{
		std::cerr << "[Server] Unsupported method: " << _method << std::endl;
        _status = 405;
        getErrorPage();
        getResponse();
        sendResponse();
        return false;
	}
	if (_httpVersion != "HTTP/1.1")
		return (false);
	size_t q_pos = _uri.find("?");
	if (q_pos != std::string::npos)
	{
		_query = _uri.substr(q_pos + 1);
		_uri = _uri.substr(0, q_pos);
	}
	pos = _buffer_in.find("\r\n");
	header_end = _buffer_in.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return (false);
	headers = _buffer_in.substr(pos + 2, header_end - pos - 2);
	std::istringstream h(headers);
	while (getline(h, line))
	{
		// Retirer le \r si présent à la fin de la ligne
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
		if (line.empty())
			break;
		pos = line.find(":");
		if (pos == std::string::npos)
			continue;
		key = line.substr(0, pos);
		// Trouver le début de la valeur (après ": ")
		size_t value_start = pos + 1;
		while (value_start < line.length() && line[value_start] == ' ')//parser les espaces apres ":"
			value_start++;
		value = line.substr(value_start);
		_headers[key] = value;
	}
	pos = _buffer_in.find("\r\n\r\n");
	_body = _buffer_in.substr(pos + 4);
	// if (_body.size() > parser->getMaxBodySize())//verification de la taille du body
	// {
	// 	_status = 413;
	// 	_content = "<html><body><h1>413 Payload Too Large</h1></body></html>";
    //     _contentSize = _content.size();
    //     getResponse();
    //     sendResponse();
    //     return (false);
	// }
	if (_method == "POST")
	{
		// Pour multipart/form-data (upload de fichiers) c'est a dire body avec boundary
		if (_body.find("--\r\n") != std::string::npos)
			return (true);
		
		// Pour body sans boundary
		std::map<std::string, std::string>::iterator it = _headers.find("Content-Length");
		if (it != _headers.end())
		{
			size_t content_length = atoi(it->second.c_str());
			if (_body.size() >= content_length)
				return (true);
		}
		return (false);
	}
	return (true);
}

void Server::add_to_poll_fds()
{
	pollfd p;
	memset(&p, 0, sizeof(p));
	p.fd = _clientSocket;
	p.events = POLLIN;
	_poll_fds.push_back(p);
 
}

void Server::accept_new_connection(int server_socket, std::vector<pollfd>& poll_fds,
	struct sockaddr_in& socketAddress, socklen_t& socketAdressLength)
{
	(void)poll_fds;
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
}

std::string toUpper(const std::string& str) 
{
	std::string result = str;
	for (size_t i = 0; i < result.length(); ++i) 
		result[i] = std::toupper(static_cast<unsigned char>(result[i]));
	return result;
}

void Server::getEnvp(std::vector<std::string>& env_strings)
{
	env_strings.push_back("REQUEST_METHOD=" + _method);
	env_strings.push_back("QUERY_STRING=" + _query);
	env_strings.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_strings.push_back("SCRIPT_NAME=" + _uri);

	if (_method == "POST") 
	{
        std::ostringstream oss;
        oss << _body.size();
        env_strings.push_back("CONTENT_LENGTH=" + oss.str());
    }

	std::map<std::string, std::string>::iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it) 
	{
		std::string env_var = "HTTP_" + toUpper(it->first);
		std::replace(env_var.begin(), env_var.end(), '-', '_');
		env_strings.push_back(env_var + "=" + it->second);
	}
}

void Server::handleCgi(void)
{
    // std::cout << "=== CGI DEBUG ===" << std::endl;
    // std::cout << "Method: [" << _method << "]" << std::endl;
    // std::cout << "URI: [" << _uri << "]" << std::endl;
    // std::cout << "Body size: " << _body.size() << std::endl;
    // std::cout << "Body content: [" << _body << "]" << std::endl;
    // std::cout << "================" << std::endl;
    std::string path = parser->getRoot() +_uri;
    // std::ifstream test(path.c_str());
    // if (!test.is_open()) 
    // {
    //     std::cerr << "[CGI] File not found: " << path << std::endl;
    //     _status = 404;
    //     getErrorPage();
    //     getResponse();
    //     sendResponse();
    //     return;
    // }
    // test.close();
	struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) 
    {
        std::cerr << "[CGI] File not found or not a regular file: " << path << std::endl;
        _status = 404;
        getErrorPage();
        getResponse();
        sendResponse();
        return;
    }
    char* argv[] = {const_cast<char*>(path.c_str()), NULL};
    std::vector<std::string> env_strings;
    getEnvp(env_strings);
    std::vector<char*> envp;
    for (size_t i = 0; i < env_strings.size(); ++i) 
        envp.push_back(const_cast<char*>(env_strings[i].c_str()));
    envp.push_back(NULL);

    int pipe_stdin[2];
    int pipe_stdout[2];
    
    if (pipe(pipe_stdin) == -1 || pipe(pipe_stdout) == -1) 
    {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) 
    {
        perror("fork");
        close(pipe_stdin[0]);
        close(pipe_stdin[1]);
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);
        return;
    }

    if (pid == 0)
    {
        close(pipe_stdin[1]);
        if (_method == "POST")
            dup2(pipe_stdin[0], STDIN_FILENO);
        close(pipe_stdin[0]);
        
        close(pipe_stdout[0]);
        dup2(pipe_stdout[1], STDOUT_FILENO);
        dup2(pipe_stdout[1], STDERR_FILENO); // Capturer aussi stderr
        close(pipe_stdout[1]);
        
        execve(path.c_str(), argv, envp.data());//le CGI s'execute
        perror("execve");
        exit(1);
    }

    // Processus parent
    close(pipe_stdin[0]);
    
    // Écrire le body si POST
    if (_method == "POST" && !_body.empty())
    {
        ssize_t written = write(pipe_stdin[1], _body.c_str(), _body.length());
        if (written == -1)
            perror("write to CGI stdin");
    }
    close(pipe_stdin[1]);
    close(pipe_stdout[1]);

    // Rendre le pipe non-bloquant
    int flags = fcntl(pipe_stdout[0], F_GETFL, 0);
    fcntl(pipe_stdout[0], F_SETFL, flags | O_NONBLOCK);

    std::string cgi_output;
    char buffer[BUFSIZ];
    time_t start_time = time(NULL);
    int timeout = 5; // 5 secondes de timeout

    // Lecture avec timeout
    while (true)
    {
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);//retourne immédiatement l'etat du processus sans bloquer
        
        if (result == -1)
        {
            perror("waitpid");
            close(pipe_stdout[0]);
            return;
        }
        
        // Le processus est terminé
        if (result > 0)
        {
            // Lire les données restantes
            ssize_t n;
            while ((n = read(pipe_stdout[0], buffer, BUFSIZ)) > 0)
                cgi_output.append(buffer, n);
            break;
        }
        
        // Timeout dépassé
        if (difftime(time(NULL), start_time) > timeout)//lorsque le processus enfant n'est pas termine on verifie le timeout
        {
            std::cerr << "[CGI] Timeout - killing process " << pid << std::endl;
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            close(pipe_stdout[0]);
            
            _status = 504;
            _content = "<html><body><h1>504 Gateway Timeout</h1></body></html>";
            _contentSize = _content.size();
            getResponse();
            sendResponse();
            return;
        }
        
        // Lire les données disponibles
        ssize_t n = read(pipe_stdout[0], buffer, BUFSIZ);//on lit au fur et a mesure pendant l'exécution du processus, pas seulement à la fin
        if (n > 0)
            cgi_output.append(buffer, n);
        else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("read from CGI");
            break;
        }
        
        usleep(10000); // Attendre 10ms
    }
    
    close(pipe_stdout[0]);

    std::cout << "[CGI] Output size: " << cgi_output.size() << std::endl;

    if (cgi_output.empty())
    {
        _status = 500;
       	getErrorPage();
        getResponse();
        sendResponse();
        return;
    }

    size_t header_end = cgi_output.find("\r\n\r\n");
    if (header_end == std::string::npos)
    {
        std::cerr << "[CGI] No header separator found in output" << std::endl;
        _status = 500;
        getErrorPage();
        getResponse();
        sendResponse();
        return;
    }
    
    header_end += 4;
    size_t pos = cgi_output.find("\r\n");
    std::string contentType = cgi_output.substr(0, pos);
    std::string body = cgi_output.substr(header_end);
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    
    _response = response.str();
    sendResponse();
}

void Server::read_data_from_socket(int Socket)
{
	char buffer[BUFSIZ];
    int bytes_read;
	
	//Sauvegarder le socket client pour sendResponse
	_clientSocket = Socket;
	
    memset(&buffer, '\0', sizeof buffer);
    bytes_read = recv(Socket, buffer, BUFSIZ, 0);
	if (bytes_read <= 0) 
	{
		if (bytes_read == 0) 
			printf("[%d] Client socket closed connection.\n", Socket);
		else 
			fprintf(stderr, "[Server] Recv error: %s\n", strerror(errno));
		close(Socket);
		for (size_t i = 0; i < _poll_fds.size(); ++i) 
        {
            if (_poll_fds[i].fd == Socket) 
            {
                _poll_fds.erase(_poll_fds.begin() + i);
                break;
            }
        }
		return;
    }
	_buffer_in.append(buffer, bytes_read);
	if (_buffer_in.find("\r\n\r\n") != std::string::npos)
	{
		if (parseRequest())
		{
			if (_uri.find("/cgi-bin") != std::string::npos && is_allowed_cgi_method())
				handleCgi();
			else
				handleMethod();
			_buffer_in.clear();
			close(Socket);
            for (size_t i = 0; i < _poll_fds.size(); ++i) 
            {
                if (_poll_fds[i].fd == Socket) 
                {
                    _poll_fds.erase(_poll_fds.begin() + i);
                    break;
                }
            }
		}
		// else
		// {
		// 	std::cout << "[Server] Parse error - sending 400" << std::endl;
		// 	_status = 400;
		// 	_content = "<html><body><h1>400 Bad Request</h1></body></html>";
		// 	_contentSize = _content.size();
		// 	getResponse();
		// 	sendResponse();
		// 	_buffer_in.clear();
		// }
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
        // std::cout << "En attente de nouvelles connexions..." << std::endl;
		return (0);
    }
	return (1);
}
void	Server::socketServerCreation(void)
{
	// 4. Configure la socket
	memset(&_socketAddress, 0, sizeof(_socketAddress));
	_socketAddress.sin_family = AF_INET;//indique qu'on veut une adresse IPV4
	_socketAddress.sin_port = htons(atoi(parser->getPort().c_str())); //on indique le port par lequel le serveur ecoute
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

bool	Server::pollLoop(std::string configFileName)
{
	parser = new Parser();
	if (!parser->configParser(configFileName))
		return (false);
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
	return (true);
}