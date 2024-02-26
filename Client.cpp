#include "Client.hpp"

Client::Client(int _socket, const std::string& _name) : _socket(_socket), _name(_name), _messageCount(0), _isconnected(false) {
    currentChannel = NULL;
    nickname = _name;
    _input = "";
}

Client::~Client() {
    // for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    //     this->currentChannel->ClientLeft(*this);
    return ;
}

std::string Client::get_nickname() {
    return nickname;
}