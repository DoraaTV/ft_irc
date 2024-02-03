#pragma once

#include "Client.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <algorithm>
#include <vector>

const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

class Server {
public:
    Server(int _port);
    void start();
    void bindSocket();
    void listenForConnections();
    int createSocket();
    void handleNewConnection(int _serverSocket);
    void handleExistingConnection(int clientSocket);
    void broadcastMessage(int senderSocket, const std::string& message);

private:
    int _serverSocket;
    int _port;
    std::vector<Client> _clients;
    fd_set _masterSet;
    int _maxFd;
};
