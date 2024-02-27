/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   whois.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syakovle <syakovle@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:21 by thrio             #+#    #+#             */
/*   Updated: 2024/02/27 16:03:43 by syakovle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::whois(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    if (std::strlen(buffer) <= 7) {
        std::string message = ":localhost 431" + senderClient->_name + ":Please specify a client to find\r\n";
        send(clientSocket, message.c_str(), message.length(), 0);
        return;
    }
    std::string clientToFind = buffer + 6;
    if (clientToFind[clientToFind.length() - 1] == '\n')
        clientToFind.erase(clientToFind.length() - 1);
    if (clientToFind[clientToFind.length() - 1] == '\r')
        clientToFind.erase(clientToFind.length() - 1);
    std::deque<Client>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_name == clientToFind)
            break ;
    }
    if (it == _clients.end()) {
        std::string message = "localhost 401 :No such nick\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    std::string message = ":localhost 311 " + senderClient->_name + " " + it->_name + " localhost 8080\r\n";
    send(clientSocket, message.c_str(), message.length(), 0);
    //list all channels the client is in from his _channels vector
    for (std::vector<Channel *>::iterator it2 = it->_channels.begin(); it2 != it->_channels.end(); ++it2) {
        std::string message = ":localhost 319 " + senderClient->_name + " " + it->_name + " " + (*it2)->_name + "\r\n";
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    // end of list
    std::string message2 = ":localhost 318 " + senderClient->_name + it->_name + " :End of /WHOIS list\r\n";
    send(clientSocket, message2.c_str(), message2.length(), 0);
}
