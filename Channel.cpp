/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:27 by thrio             #+#    #+#             */
/*   Updated: 2024/02/10 13:28:47 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(Client &founder, std::string name) : _name(name) {
    _operators[founder._name] = &founder;
    ClientJoin(founder);
}

Channel::~Channel() {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        it->second->currentChannel = NULL;
    std::string message = "\nChannel " + _name + " has been destroyed !\n";
    broadcastMessage(message);
}

void Channel::ClientJoin(Client &client) {
    //if (client.currentChannel)
    //    client.currentChannel->ClientLeft(client);
    if (_limit && _nbClients >= _limit) {
        std::string notification = "Channel [" + _name + "] is full !\n";
        send((client)._socket, notification.c_str(), notification.length(), 0);
        return;
    }
    if (_isInviteOnly && _invited.find(client._name) == _invited.end()) {
        std::string notification = "You are not invited to join [" + _name + "] !\n";
        send((client)._socket, notification.c_str(), notification.length(), 0);
        return;
    }
    std::string message = "\n" + client._name + " has joined the channel !\n";
    broadcastMessage(message);
    _clients[client._name] = &client;
    client.currentChannel = this;
    std::string notification = "You joined [" + _name + "] !\n";
    _nbClients++;
    send((client)._socket, notification.c_str(), notification.length(), 0);

}

void Channel::ClientLeft(Client &client) {
    std::string notification = "You left [" + _name + "] !\n";
    send((client)._socket, notification.c_str(), notification.length(), 0);
    client.currentChannel = NULL;
    _clients.erase(client._name);
    std::string message = "\n" + client._name + " has left the channel !\n";
    broadcastMessage(message);
    _nbClients--;
    _operators.erase(client._name);
}

void Channel::sendMessage(const std::string &message, Client &sender) {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if ((*it).second != &sender)
            send((*it).second->_socket, message.c_str(), message.length(), 0);
    }
}

int Channel::isInviteOnly() {
    return _isInviteOnly;
}

void Channel::setInviteOnly(bool inviteOnly) {
    _isInviteOnly = inviteOnly;
}

void Channel::ClientKick(Client &client) {
    std::string message = "\n" + client._name + " has been kicked from the channel !\n";
    sendMessage(message, client);
    _clients.erase(client._name);
    _operators.erase(client._name);
    client.currentChannel = NULL;
    std::string notification = "You have been kicked from [" + _name + "] !\n";
    send((client)._socket, notification.c_str(), notification.length(), 0);
}

void Channel::setLimit(unsigned int limit) {
    _limit = limit;
}


void Channel::broadcastMessage(const std::string &message) {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            send((*it).second->_socket, message.c_str(), message.length(), 0);
    }
}
