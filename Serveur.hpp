#pragma once

#include "Channel.hpp"
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
#include <deque>
#include <map>

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
    void showChannels(int clientSocket);

private:
    int _serverSocket;
    int _port;
    std::deque<Client> _clients;
    fd_set _masterSet;
    int _maxFd;
    std::map<std::string, Channel*> _channels;
};
