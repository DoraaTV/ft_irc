#pragma once

#include "Channel.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <algorithm>  // Pour std::find_if
#include <vector>

class Channel;

class Client {
public:
    Client(int _socket, const std::string& _name);
    ~Client();
    std::string nickname;

public:
    int _socket;
    std::string _name;
    std::string get_nickname();
    unsigned int _messageCount;
    Channel *currentChannel;
    std::vector<Channel *> _channels;
    bool _isconnected;
    std::string _input;
};

struct ClientFinder {
    int socketToFind;
    ClientFinder(int socketToFind) : socketToFind(socketToFind) {}
    bool operator()(const Client& _client) const {
        return _client._socket == socketToFind;
    }
};
