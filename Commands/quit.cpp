/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:15 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 09:19:16 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::quit(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    static_cast<void>(buffer);
    static_cast<void>(clientSocket);
    // leaves all channels he is in using broadcastMessage to send a message to all clients in the channel
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if (!it->second)
            continue;
        else {
            it->second->ClientLeft(*senderClient);
        }
    }
    std::string message = "QUIT :Leaving\r\n";
    send(clientSocket, message.c_str(), message.length(), 0);
    close(clientSocket);
    _clients.erase(std::remove_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket)), _clients.end());
    FD_CLR(clientSocket, &_masterSet);
    std::cout << "Client " << senderClient->_name << " has left the server" << std::endl;
}