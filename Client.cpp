#include "Client.hpp"

Client::Client(int _socket, const std::string& _name) : _socket(_socket), _name(_name), _messageCount(0){
    currentChannel = NULL;
}

