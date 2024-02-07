/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:27 by thrio             #+#    #+#             */
/*   Updated: 2024/02/07 14:35:37 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(Client &founder, std::string name) : _name(name) {
    _operators[founder._name] = &founder;
    ClientJoin(founder);
}

void Channel::ClientJoin(Client &client) {
    if (client.currentChannel)
        client.currentChannel->ClientLeft(client);
    std::string message = "\n" + client._name + " has joined the channel !\n";
    broadcastMessage(message);
    _clients[client._name] = &client;
    client.currentChannel = this;
    std::string notification = "\n\nYou joined [" + _name + "] ! \n\n";
    send((client)._socket, notification.c_str(), notification.length(), 0);
    
}

void Channel::ClientLeft(Client &client) {
    std::string notification = "\n\nYou left [" + _name + "] ! \n\n";
    send((client)._socket, notification.c_str(), notification.length(), 0);
    client.currentChannel = NULL;
    _clients.erase(_clients.find(client._name));
    std::string message = "\n" + client._name + " has left the channel !\n";
    broadcastMessage(message);

}

void Channel::broadcastMessage(const std::string &message) {
    for (std::map<std::string, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        {
            send((*it).second->_socket, message.c_str(), message.length(), 0);
        }
    }
}
