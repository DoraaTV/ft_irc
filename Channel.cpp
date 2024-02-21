/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:27 by thrio             #+#    #+#             */
/*   Updated: 2024/02/21 17:25:53 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(Client &founder, std::string name) : _name(name) {
    _limit = 10;
    nbClients = 0;
    _isInviteOnly = false;
    _isPasswordProtected = false;
    _operators[founder._name] = &founder;
    _topic = "No topic";
    _canSetTopic = true;
    ClientJoin(founder);
}

Channel::~Channel() {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        it->second->currentChannel = NULL;
    std::string message = "Channel " + _name + " has been destroyed !\r\n";
    broadcastMessage(message);
}

bool Channel::isOperator(const std::string &clientName) {
    return _operators.count(clientName);
}

void Channel::setModeTopic(bool mode) {
    _canSetTopic = mode;
}

void Channel::setTopic(std::string &topic) {
    topic = topic.substr(0, topic.size() - 1);
    _topic = topic;
}

std::string Channel::getTopic() {
    return _topic;
}

void Channel::ClientKick(std::string &clientToKick) {
    if (_clients.count(clientToKick)) {
        Client &client = *(_clients[clientToKick]);
        std::string notification = "You have been kicked from [" + _name + "] !\r\n";
        send((client)._socket, notification.c_str(), notification.length(), 0);
        client.currentChannel = NULL;
        _clients.erase(client._name);
        std::string message = client._name + " has been kicked from the channel !\r\n";
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
        std::string notification = ":localhost 471 " + client._name + " " + _name + " :Cannot join channel (+l)\r\n";
        send((client)._socket, notification.c_str(), notification.length(), 0);
        return;
    }
    if (_isInviteOnly && _invited.find(client._name) == _invited.end()) {
        std::string notification = ":localhost 473 " + client._name + " " + _name + " :Cannot join channel (+i)\r\n";
        send((client)._socket, notification.c_str(), notification.length(), 0);
        return;
    }
    std::cout << "Client joined channel : " << _name << std::endl;
    _clients[client._name] = &client;
    client._channels.push_back(this);
    client.currentChannel = client._channels.back();
    std::string notification = ":" + client._name + "@localhost JOIN " + _name + "\r\n";
    std::cout << notification << std::endl;
    send((client)._socket, notification.c_str(), notification.length(), 0);
    broadcastMessage(notification);
    nbClients++;
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
    
    // check if client is in channel
    if (_clients.find(client._name) == _clients.end())
        return;
    std::string notification = ":" + client._name + "@localhost PART " + _name + "\r\n";
    std::cout << notification << std::endl;
    send((client)._socket, notification.c_str(), notification.length(), 0);
    client._channels.erase(std::remove(client._channels.begin(), client._channels.end(), this), client._channels.end());
    if (client._channels.size())
        client.currentChannel = client._channels.back();
    else
        client.currentChannel = NULL;
    _clients.erase(client._name);
    broadcastMessage(notification);
    _operators.erase(client._name);\
    nbClients--;
}

void Channel::sendMessage(const std::string &message, Client &sender) {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        std::cout << "Sending message to " << (*it).second->_name << "socket : " << (*it).second->_socket << std::endl;
        if ((*it).second != &sender) {
            std::string messageToSend = message;
            if (messageToSend[0] == ':')
                messageToSend = messageToSend.substr(1);
            std::string notification = ":" + sender._name + " PRIVMSG " + _name + " :" + messageToSend + "\r\n";
            std::cout << notification << std::endl;
            send((*it).second->_socket, notification.c_str(), notification.length(), 0);
        }
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
            std::string notification = message + "\r\n";
            std::cout << notification << std::endl;
            send((*it).second->_socket, notification.c_str(), notification.length(), 0);
    }
}
