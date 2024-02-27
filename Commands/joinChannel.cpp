/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   joinChannel.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syakovle <syakovle@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:04 by thrio             #+#    #+#             */
/*   Updated: 2024/02/27 17:09:07 by syakovle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::joinChannel(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    std::vector<std::string> tokens = split(buffer, ' ');
    if (tokens.size() < 2) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "JOIN").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "JOIN").c_str()), 0);
        return;
    }
    if (tokens.size() > 3) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "JOIN").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "JOIN").c_str()), 0);
        return;
    }
    std::vector<std::string> tokens2 = split(tokens[1], ',');
    if (tokens.size() == 3) {
        casePasswd(clientSocket, senderClient, tokens, tokens2);
    }
    else {
        caseNormal(clientSocket, senderClient, tokens2);
    }
}

void Server::casePasswd(int clientSocket, std::deque<Client>::iterator senderClient, std::vector<std::string> tokens, std::vector<std::string> tokens2) {
    std::vector<std::string> password = split(tokens[2], ',');
    //if there is \n at the end of the last password
    removeTrailingCarriageReturn(password[password.size() - 1]);
    std::vector<std::string>::iterator itPassword = password.begin();
    for (std::vector<std::string>::iterator it = tokens2.begin(); it != tokens2.end(); ++it) {
        std::string channelName;
        if (it->c_str()[0] != '#')
            channelName = "#" + *it;
        else
            channelName = *it;
        if (_channels[channelName]) {
            if (_channels[channelName]->_isPasswordProtected && (itPassword != password.end() && _channels[channelName]->_password != *itPassword)) {
                if (_channels[channelName]->_invited[senderClient->_name])
                    _channels[channelName]->ClientJoin(*senderClient);
                else
                    send(clientSocket, ERR_BADCHANNELKEY(senderClient, channelName).c_str(), std::strlen(ERR_BADCHANNELKEY(senderClient, channelName).c_str()), 0);
                continue;
            }
            _channels[channelName]->ClientJoin(*senderClient);
        } else
            _channels[channelName] = new Channel(*senderClient, channelName);
        if (itPassword != password.end())
            itPassword++;
    }
}

void Server::caseNormal(int clientSocket, std::deque<Client>::iterator senderClient, std::vector<std::string> tokens2) {
    // if there is \n at the end of the last channel name
    removeTrailingCarriageReturn(tokens2[tokens2.size() - 1]);
    for (std::vector<std::string>::iterator it = tokens2.begin(); it != tokens2.end(); ++it) {
        std::string channelName;
        if (it->c_str()[0] != '#')
            channelName = "#" + *it;
        else
            channelName = *it;
        if (_channels[channelName]) {
            if (_channels[channelName]->_isPasswordProtected) {
                if (_channels[channelName]->_invited[senderClient->_name])
                    _channels[channelName]->ClientJoin(*senderClient);
                else
                    send(clientSocket, ERR_BADCHANNELKEY(senderClient, channelName).c_str(), std::strlen(ERR_BADCHANNELKEY(senderClient, channelName).c_str()), 0);
                return;
            }
            _channels[channelName]->ClientJoin(*senderClient);
        }
        else {
            _channels[channelName] = new Channel(*senderClient, channelName);
        }
    }
}
