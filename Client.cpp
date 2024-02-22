#include "Client.hpp"

Client::Client(int _socket, const std::string& _name) : _socket(_socket), _name(_name), _messageCount(0){
    currentChannel = NULL;
    nickname = _name;
}

Client::~Client() {
    // for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    //     this->currentChannel->ClientLeft(*this);
    return ;
}
