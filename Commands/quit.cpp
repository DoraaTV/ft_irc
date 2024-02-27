/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thrio <thrio@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:15 by thrio             #+#    #+#             */
/*   Updated: 2024/02/27 15:14:16 by thrio            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Serveur.hpp"

struct ClientFinder {
    int socketToFind;
    ClientFinder(int socketToFind) : socketToFind(socketToFind) {}
    bool operator()(const Client& _client) const {
        return _client._socket == socketToFind;
    }
};

void Server::quit(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    static_cast<void>(buffer);
    // leaves all channels he is in using broadcastMessage to send a message to all clients in the channel
    for (std::vector<Channel *>::iterator it = senderClient->_channels.begin(); it != senderClient->_channels.end(); ++it) {
        std::string message = ":" + senderClient->_name + " PART " + (*it)->_name + " :Leaving\r\n";
        (*it)->_clients.erase(senderClient->_name);
        (*it)->_operators.erase(senderClient->_name);
        if ((*it)->_operators.size() == 0 && (*it)->_clients.size() > 0) {
            std::string name2 = (*it)->_clients.begin()->first;
            (*it)->addOperator(name2);
        }
        (*it)->broadcastMessage(message);
    }
    std::string message = "QUIT :Leaving\r\n";
    send(clientSocket, message.c_str(), message.length(), 0);
    close(clientSocket);
    _clients.erase(std::remove_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket)), _clients.end());
    FD_CLR(clientSocket, &_masterSet);
    std::cout << "Client " << senderClient->_name << " has left the server" << std::endl;
}