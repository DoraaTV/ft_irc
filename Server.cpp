#include "Server.hpp"

Server::Server()
{
	_port = 0;
	_socket = 0;
}

Server::~Server()
{

}

void Server::createServer(int port)
{
    if (port == 0)
    {
        std::cout << "\031 invalid port: Must be 5 valid digits" << std::endl;
    }
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    _csin.sin_family = AF_INET;
    _csin.sin_port = htons(port);
    _csin.sin_addr.s_addr = htons(INADDR_ANY);
    bind(_socket, (const sockaddr*)(&_csin), sizeof(_csin));
    listen(_socket, 42);
}


