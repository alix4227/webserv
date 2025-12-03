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

	pos = content.find("POST");//method POST
	if (pos == std::string::npos)
		return (false);
	posEnd = content.find(" ", pos);
	std::string POST = content.substr(pos, posEnd - pos);

	pos = content.find("DELETE");//method DELETE
	if (pos == std::string::npos)
		return (false);
	posEnd = content.find(";", pos);
	std::string DELETE = content.substr(pos, posEnd - pos);
	return (true);
}