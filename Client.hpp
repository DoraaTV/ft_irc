#pragma once

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

class Client {
public:
    Client(int _socket, const std::string& _name);

public:
    int _socket;
    std::string _name;
    unsigned int _messageCount;
};
