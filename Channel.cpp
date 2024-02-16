/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:27 by thrio             #+#    #+#             */
/*   Updated: 2024/02/16 16:12:12 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(Client &founder, std::string name) : _name(name) {
    _limit = 10;
    _isInviteOnly = false;
    _isPasswordProtected = false;
    _operators[founder._name] = &founder;
    _topic = "No topic";
    ClientJoin(founder);
}

Channel::~Channel() {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        it->second->currentChannel = NULL;
    std::string message = "\nChannel " + _name + " has been destroyed !\n";
    broadcastMessage(message);
}

bool Channel::isOperator(const std::string &clientName) {
    return _operators.count(clientName);
}

void Channel::setTopic(std::string &topic) {
    topic = topic.substr(0, topic.size() - 1);
    _topic = topic;
}

std::string Channel::getTopic() {
    std::string message = "The topic of " + _name + " is : " + _topic + "\n";
    return message;
}

void Channel::ClientKick(std::string &clientToKick) {
    if (_clients.count(clientToKick)) {
        Client &client = *(_clients[clientToKick]);
        std::string notification = "\033[31mYou have been kicked from [" + _name + "] !\n\033[0m";
        send((client)._socket, notification.c_str(), notification.length(), 0);
        client.currentChannel = NULL;
        _clients.erase(client._name);
        std::string message = "\033[31m\n" + client._name + " has been kicked from the channel !\n\033[0m";
        broadcastMessage(message);
        _operators.erase(client._name);
    }
}

void Channel::setPasswd(std::string &passwd) {
    _isPasswordProtected = true;
    _password = passwd;
}

void Channel::removePasswd() {
    _isPasswordProtected = false;
    _password = "";
}

void Channel::ClientJoin(Client &client) {
    //if (client.currentChannel)
    //    client.currentChannel->ClientLeft(client);
    if (_limit && _clients.size() >= _limit) {
        std::string notification = "\033[31mChannel [" + _name + "] is full !\n\033[0m";
        send((client)._socket, notification.c_str(), notification.length(), 0);
        return;
    }
    if (_isInviteOnly && _invited.find(client._name) == _invited.end()) {
        std::string notification = "\033[31mYou are not invited to join [" + _name + "] !\n\033[0m";
        send((client)._socket, notification.c_str(), notification.length(), 0);
        return;
    }
    std::cout << "Client joined channel : " << _name << std::endl;
    std::string message = "\033[32m\n" + client._name + " has joined the channel !\n\033[0m";
    broadcastMessage(message);
    _clients[client._name] = &client;
    client.currentChannel = this;
    std::string notification = "\033[32mYou joined [" + _name + "] !\n\033[0m";
    send((client)._socket, notification.c_str(), notification.length(), 0);

}

void Channel::addOperator(std::string &clientName) {
    //remove last char from clientName that is \n
    clientName = clientName.substr(0, clientName.size() - 1);
    std::cout << "Trying to add operator : " << clientName << " to channel " << _name << std::endl;
    std::cout << "Client : " << _clients[clientName]->currentChannel << std::endl;
    if (_clients[clientName]) {
        std::cout << "Adding operator : " << clientName << " to channel " << _name << std::endl;
        _operators[clientName] = _clients[clientName];
    }

    //show all op for debug
    for (std::map<std::string, Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it) {
        std::cout << "Operator : " << (*it).second->_name << std::endl;
    }
}

void Channel::removeOperator(std::string &clientName) {
        _operators.erase(clientName);
}

void Channel::ClientLeft(Client &client) {
    std::string notification = "\033[32mYou left [" + _name + "] !\n\033[0m";
    send((client)._socket, notification.c_str(), notification.length(), 0);
    client.currentChannel = NULL;
    _clients.erase(client._name);
    std::string message = "\033[32m\n" + client._name + " has left the channel !\n\033[0m";
    broadcastMessage(message);
    _operators.erase(client._name);
}

void Channel::sendMessage(const std::string &message, Client &sender) {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        std::cout << "Sending message to " << (*it).second->_name << "socket : " << (*it).second->_socket << std::endl;
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


void Channel::setLimit(unsigned int limit) {
    _limit = limit;
}


void Channel::broadcastMessage(const std::string &message) {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            std::cout << "Sending message to " << (*it).second->_name << "socket : " << (*it).second->_socket << std::endl;
            send((*it).second->_socket, message.c_str(), message.length(), 0);
    }
}
