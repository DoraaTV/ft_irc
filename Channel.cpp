/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thrio <thrio@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:27 by thrio             #+#    #+#             */
/*   Updated: 2024/02/05 16:04:45 by thrio            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(Client &founder, std::string name) : _name(name) {
    _operators.push_back(&founder);
    ClientJoin(founder);
}

void Channel::ClientJoin(Client &client) {
    _clients.push_back(&client);
    client.currentChannel = this;
    std::string notification = "\n\nYou joined [" + _name + "] ! \n\n";
    send((client)._socket, notification.c_str(), notification.length(), 0);
}

void Channel::broadcastMessage(const std::string &message) {
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        {
            send((*it)->_socket, message.c_str(), message.length(), 0);
        }
    }
}
