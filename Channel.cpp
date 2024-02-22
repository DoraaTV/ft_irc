/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thrio <thrio@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:27 by thrio             #+#    #+#             */
/*   Updated: 2024/02/22 15:17:34 by thrio            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(Client &founder, std::string name) : _name(name) {
    _limit = 10;
    nbClients = 0;
    _isInviteOnly = false;
    _isPasswordProtected = false;
    _operators[founder._name] = &founder;
    _topic = "No topic set for this channel.";
    _isTopicRestricted = true;
    ClientJoin(founder);
}

Channel::~Channel() {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        it->second->currentChannel = NULL;
    std::string message = "Channel " + _name + " has been destroyed !\r\n";
    broadcastMessage(message);
    std::cout << message << std::endl;
    _clients.clear();
}

bool Channel::isOperator(const std::string &clientName) {
    for (std::map<std::string, Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if (it->first == clientName) {
            return true;
        }
    }
    return false;
}

void Channel::setModeTopic(bool mode) {
    _isTopicRestricted = mode;
}

void Channel::setTopic(std::string &topic) {
    topic = topic.substr(0, topic.size());
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
    if (_operators.size() == 0)
        addOperator(client._name);
    std::string notification = ":" + client._name + "@localhost JOIN " + _name + "\r\n";
    std::cout << notification << std::endl;
    broadcastMessage(notification);
    // send((client)._socket, notification.c_str(), notification.length(), 0);
    nbClients++;
    std::string message = ":localhost 332 " + client._name + " " + _name + " :" + _topic + "\r\n";
    std::cout << message << std::endl;
    send((client)._socket, message.c_str(), message.length(), 0);
}

void Channel::addOperator(std::string &clientName) {
    
    if (clientName.find("\n") != std::string::npos)
        clientName = clientName.substr(0, clientName.size() - 1);
    if (clientName.find("\r") != std::string::npos)
        clientName = clientName.substr(0, clientName.size() - 1);
        
    _operators[clientName] = _clients[clientName];

    // debug message
    std::cout << "Operator added to channel " << _name << " : " << clientName << std::endl;
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
    _operators.erase(client._name);
    _clients.erase(client._name);
    // if no operator left, the first client becomes operator
    if (_operators.size() == 0 && _clients.size() > 0) {
        std::string name2 = _clients.begin()->first;
        addOperator(name2);
    }
    broadcastMessage(notification);
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
