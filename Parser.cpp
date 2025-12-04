#include "Parser.hpp"
Parser::Parser()
{

}
Parser::~Parser()
{

}
bool Parser::configParser(std::string filename)
{
	size_t pos;
	size_t posEnd;
	std::string path = filename;
	std::string content;
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "ConfigFile Error" << std::endl;
		return (false);
	}
	file.seekg(0, std::ios::end);
	size_t contentSize = file.tellg();
	file.seekg(0, std::ios::beg);
	content.resize(contentSize);
	file.read(&content[0], contentSize);
	file.close();
	
	pos = content.find("listen");//port
	if (pos == std::string::npos)
		return (false);
	pos += 7;
	posEnd = content.find(";", pos);
	_port = content.substr(pos, posEnd - pos);
	
	pos = content.find("root");//path du root
	if (pos == std::string::npos)
		return (false); 
	pos += 5;
	posEnd = content.find(";", pos);
	_root = "./" + content.substr(pos, posEnd - pos);
	
	pos = content.find("index");//uri par defaut
	if (pos == std::string::npos)
		return (false);
	pos += 6;
	posEnd = content.find(";", pos);
	_defaultUri = "/" + content.substr(pos, posEnd - pos);
	
	pos = content.find("client_max_body_size");//taille max du body
	if (pos == std::string::npos)
		return (false);
	pos += 21;
	posEnd = content.find(";", pos);
	_maxBodySize = atoi(content.substr(pos, posEnd - pos).c_str());
	
	pos = content.find("upload_path");//path pour les uploads
	if (pos == std::string::npos)
		return (false);
	pos += 12;
	posEnd = content.find(";", pos);
	_uploadPath = "./" + content.substr(pos, posEnd - pos);

	pos = content.find("allow_methods");//method GET
	if (pos == std::string::npos)
		return (false);
	pos += 14;
	posEnd = content.find(" ", pos);
	std::string GET = content.substr(pos, posEnd - pos);
	allowedMethod.push_back(GET);

	pos = content.find("POST");//method POST
	if (pos == std::string::npos)
		return (false);
	posEnd = content.find(" ", pos);
	std::string POST = content.substr(pos, posEnd - pos);
	allowedMethod.push_back(POST);

	pos = content.find("DELETE");//method DELETE
	if (pos == std::string::npos)
		return (false);
	posEnd = content.find(";", pos);
	std::string DELETE = content.substr(pos, posEnd - pos);
	allowedMethod.push_back(DELETE);

	pos = content.find("location /cgi-bin");//cgi GET
	if (pos == std::string::npos)
		return (false);

	pos = content.find("allow_methods", pos);
	if (pos == std::string::npos)
		return (false);
	pos += 14;
	posEnd = content.find(" ", pos);
	std::string GET_CGI = content.substr(pos, posEnd - pos);
	allowedCgiMethod.push_back(GET_CGI);
	
	pos = posEnd + 1;//cgi POST
	posEnd = content.find(";", pos);
	std::string POST_CGI = content.substr(pos, posEnd - pos);
	allowedCgiMethod.push_back(POST_CGI);
	
	pos = content.find("location /cgi-bin");//cgi Path
	if (pos == std::string::npos)
		return (false);
	pos = content.find("root", pos);
	if (pos == std::string::npos)
		return (false);
	pos += 5;
	posEnd = content.find(";", pos);
	_cgiPath = "./" + content.substr(pos, posEnd - pos);

	pos = content.find("errors_page 403");//path error page 403
	if (pos == std::string::npos)
		return (false);
	pos += 16;
	posEnd = content.find(";", pos);
	_errorPage403 = "." + content.substr(pos, posEnd - pos);

	pos = content.find("errors_page 404");//path error page 404
	if (pos == std::string::npos)
		return (false);
	pos += 16;
	posEnd = content.find(";", pos);
	_errorPage404 = "." + content.substr(pos, posEnd - pos);

	pos = content.find("errors_page 405");//path error page 405
	if (pos == std::string::npos)
		return (false);
	pos += 16;
	posEnd = content.find(";", pos);
	_errorPage405 = "." + content.substr(pos, posEnd - pos);

	pos = content.find("errors_page 413");//path error page 413
	if (pos == std::string::npos)
		return (false);
	pos += 16;
	posEnd = content.find(";", pos);
	_errorPage413 = "." + content.substr(pos, posEnd - pos);

	pos = content.find("errors_page 500");//path error page 500
	if (pos == std::string::npos)
		return (false);
	pos += 16;
	posEnd = content.find(";", pos);
	_errorPage500 = "." + content.substr(pos, posEnd - pos);

	pos = content.find("errors_page 502");//path error page 502
	if (pos == std::string::npos)
		return (false);
	pos += 16;
	posEnd = content.find(";", pos);
	_errorPage502 = "." + content.substr(pos, posEnd - pos);
	return (true);
}

std::string Parser::getPort() const
{
	return _port;
}

std::string Parser::getRoot() const
{
	return _root;
}

std::string Parser::getDefaultUri() const
{
	return _defaultUri;
}

size_t Parser::getMaxBodySize() const
{
	return _maxBodySize;
}

std::string Parser::getUploadPath() const
{
	return _uploadPath;
}

std::string Parser::getCgiPath() const
{
	return _cgiPath;
}

std::vector<std::string> Parser::getAllowedMethod() const
{
	return allowedMethod;
}

std::vector<std::string> Parser::getAllowedCgiMethod() const
{
	return allowedCgiMethod;
}

std::string Parser::getErrorPage403() const
{
	return _errorPage403;
}

std::string Parser::getErrorPage404() const
{
	return _errorPage404;
}

std::string Parser::getErrorPage405() const
{
	return _errorPage405;
}

std::string Parser::getErrorPage413() const
{
	return _errorPage413;
}

std::string Parser::getErrorPage500() const
{
	return _errorPage500;
}

std::string Parser::getErrorPage502() const
{
	return _errorPage502;
}