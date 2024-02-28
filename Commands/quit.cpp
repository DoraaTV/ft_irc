/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:15 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 10:18:44 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::quit(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    static_cast<void>(buffer);
    static_cast<void>(clientSocket);
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if (!it->second)
            continue;
        else {
            it->second->ClientLeft(*senderClient, 1);
        }
    }

    close(clientSocket);
    _clients.erase(std::remove_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket)), _clients.end());
    FD_CLR(clientSocket, &_masterSet);
    std::cout << "\033[45mClient " << senderClient->_name << " has left the server\033[0m" << std::endl;
}