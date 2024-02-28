/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privateMessage.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:13:50 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 10:41:18 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"
#include <stdio.h>

void Server::privateMessage(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    static_cast<void>(clientSocket);
    std::string command = buffer + 8;
    if (command[0] == '#') {
        std::string channelName = command.substr(0, command.find(" "));
        std::string textToSend = command.substr(command.find(" ") + 1);
        if (_channels.find(channelName) != _channels.end()) {
            if (_channels[channelName]->_clients.count(senderClient->_name)) {
                if (textToSend[textToSend.length() - 1] == '\n')
                    textToSend.erase(textToSend.length() - 1);
                if (textToSend[textToSend.length() - 1] == '\r')
                    textToSend.erase(textToSend.length() - 1);
                _channels[channelName]->sendMessage(textToSend, *senderClient);
            }
            else {
                send(clientSocket, ERR_NOTONCHANNEL(senderClient, channelName).c_str(), std::strlen(ERR_NOTONCHANNEL(senderClient, channelName).c_str()), 0);
            }
        } else {
            send(clientSocket, ERR_NOSUCHCHANNEL(senderClient, channelName).c_str(), std::strlen(ERR_NOSUCHCHANNEL(senderClient, channelName).c_str()), 0);
        }
        return ;
    }
    size_t space_pos = command.find(" ");
    if (space_pos != std::string::npos) {
        std::string receiverName = command.substr(0, command.find(" "));
        command = command.substr(space_pos + 1);
        if (space_pos != std::string::npos) {
            std::string textToSend = command;
            if (textToSend[textToSend.length() - 1] == '\n')
            textToSend.erase(textToSend.length() - 1);
            if (textToSend[textToSend.length() - 1] == '\r')
                textToSend.erase(textToSend.length() - 1);
            std::deque<Client>::iterator it;
            for (it = _clients.begin(); it != _clients.end(); ++it) {
                std::cout << it->_name << " " << receiverName << std::endl;
                if (it->_name == receiverName) {
                    if (textToSend[0] == ':')
                        textToSend.erase(0, 1);
                    send(it->_socket, RPL_PRIVMSG(senderClient, textToSend, receiverName).c_str(), std::strlen(RPL_PRIVMSG(senderClient, textToSend, receiverName).c_str()), 0);
                    break;
                }
            }
            if (it == _clients.end()) {
                send(clientSocket, ERR_NOSUCHNICK(senderClient, receiverName).c_str(), std::strlen(ERR_NOSUCHNICK(senderClient, receiverName).c_str()), 0);
            }
        }
    }
}
