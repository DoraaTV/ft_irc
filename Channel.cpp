/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:27 by thrio             #+#    #+#             */
/*   Updated: 2024/02/27 11:34:32 by parallels        ###   ########.fr       */
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

void Channel::setPasswd(std::string &passwd) {
    _isPasswordProtected = true;
    std::cout << "channel : " << _name << " password has been set to : " << passwd << std::endl;
    _password = passwd;
}

void Channel::removePasswd() {
    _isPasswordProtected = false;
    std::cout << "channel : " << _name << " password has been removed" << std::endl;
    _password = "";
}

void Channel::ClientJoin(Client &client) {

    if (_limit && _clients.size() >= _limit) {
        send((client)._socket, ERR_CHANNELISFULL(client, _name).c_str(), std::strlen(ERR_CHANNELISFULL(client, _name).c_str()), 0);
        return;
    }
    if (_isInviteOnly && _invited.find(client._name) == _invited.end()) {
        send((client)._socket, ERR_INVITEONLYCHAN(client, _name).c_str(), std::strlen(ERR_INVITEONLYCHAN(client, _name).c_str()), 0);
        return;
    }
    std::cout << "Client " +  client._name + " joined channel : " << _name << std::endl;
    _clients[client._name] = &client;
    client._channels.push_back(this);
    client.currentChannel = client._channels.back();
    if (_operators.size() == 0)
        addOperator(client._name);
    std::string notification = ":" + client._name + "@localhost JOIN " + _name + "\r\n";
    broadcastMessage(notification);
    nbClients++;
    std::string message = ":localhost 332 " + client._name + " " + _name + " :" + _topic + "\r\n";
    send((client)._socket, message.c_str(), message.length(), 0);
}

void Channel::addOperator(std::string &clientName) {

    removeTrailingCarriageReturn(clientName);

    _operators[clientName] = _clients[clientName];

    std::cout << "Operator added to channel " << _name << " : " << clientName << std::endl;
}

void Channel::removeOperator(std::string &clientName) {
        std::cout << "Operator removed from channel " << _name << " : " << clientName << std::endl;
        _operators.erase(clientName);
}

void Channel::ClientLeft(Client &client) {

    // check if client is in channel
    if (_clients.find(client._name) == _clients.end())
        return;
    std::string notification = ":" + client._name + "@localhost PART " + _name + "\r\n";
    send((client)._socket, notification.c_str(), notification.length(), 0);
    std::cout << "Client " +  client._name + " left channel : " << _name << std::endl;
    client._channels.erase(std::remove(client._channels.begin(), client._channels.end(), this), client._channels.end());
    if (client._channels.size())
        client.currentChannel = client._channels.back();
    else
        client.currentChannel = NULL;
    _operators.erase(client._name);
    _clients.erase(client._name);
    if (_operators.size() == 0 && _clients.size() > 0) {
        std::string name2 = _clients.begin()->first;
        addOperator(name2);
    }
    broadcastMessage(notification);
    nbClients--;
}

void Channel::sendMessage(const std::string &message, Client &sender) {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if ((*it).second != &sender) {
            std::string messageToSend = message;
            if (messageToSend[0] == ':')
                messageToSend = messageToSend.substr(1);
            // if client is operator, add operator prefix
            std::string notification;
            if (isOperator(sender._name)) {
                notification = ":\x02\x03";
                notification +=  "04(+)" + sender._name + "\02\x03";
                notification +=  "04 PRIVMSG " + _name + " :" + messageToSend + "\r\n";
            }
            else
                notification = ":" + sender._name + " PRIVMSG " + _name + " :" + messageToSend + "\r\n";
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
            std::string notification = message + "\r\n";
            send((*it).second->_socket, notification.c_str(), notification.length(), 0);
    }
}
