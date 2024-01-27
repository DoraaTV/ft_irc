#pragma once
#include <netinet/in.h>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <vector>

class User;

class Server
{
	private:
		int _port;
		int _socket;
		struct sockaddr_in _csin;
		std::vector<User*> _users;
	public:
		Server();
		~Server();
		void createServer(int port);
		int getSocket() { return _socket; }
		struct sockaddr_in *getcsin() { return &_csin; }
};