/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   changeNick.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:13:54 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 09:41:30 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::changeNick(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    std::string newNick = buffer + 5;
    if (newNick.length() <= 1) {
        send(clientSocket, ERR_NONICKNAMEGIVEN(senderClient).c_str(), std::strlen(ERR_NONICKNAMEGIVEN(senderClient).c_str()), 0);
        return;
    }
    if (newNick[newNick.length() - 1] == '\n')
        newNick.erase(newNick.length() - 1);
    if (newNick[newNick.length() - 1] == '\r')
        newNick.erase(newNick.length() - 1);
    for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_name == newNick) {
            send(clientSocket, ERR_NICKNAMEINUSE(senderClient, newNick).c_str(), std::strlen(ERR_NICKNAMEINUSE(senderClient, newNick).c_str()), 0);
            return;
        }
    }
    std::string nickname2 = split(newNick, '\n')[0];
    if (senderClient->_name.empty()) {
        std::string message1 = ":@localhost NICK " + nickname2 + "\r\n";
        std::string welcomeMessage = ":localhost 001 " + nickname2 + " : Welcome to the chat room!\r\n";
        send(clientSocket, welcomeMessage.c_str(), welcomeMessage.length(), 0);
        send(clientSocket, message1.c_str(), message1.length(), 0);
        std::string message3 = ":localhost 002 :Your host is 42_Ftirc (localhost), running version 1.1\r\n";
        send(clientSocket, message3.c_str(), message3.length(), 0);
        std::string message4 = ":localhost 003 :This server was created 20-02-2024 19:45:17\r\n";
        send(clientSocket, message4.c_str(), message4.length(), 0);
        std::string message5 = ":localhost 004 localhost 1.1 io kost k\r\n";
        send(clientSocket, message5.c_str(), message5.length(), 0);
        std::string message6 = ":localhost 005 CHANNELLEN=32 NICKLEN=9 TOPICLEN=307 :are supported by this server\r\n";
        send(clientSocket, message6.c_str(), message6.length(), 0);
    } else {
        std::string message = ":localhost 001 " + nickname2 + " :Your nickname is now " + nickname2 + "\r\n";
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    std::cout << "\033[46mClient " << senderClient->_name << " changed his nickname to " << nickname2 << "\033[0m" << std::endl;
    senderClient->nickname = nickname2;
    senderClient->_name = nickname2;
}