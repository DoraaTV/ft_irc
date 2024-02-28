/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   leaveChannel.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:08 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 10:18:37 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::leaveChannel(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    // should be /PART <channel name>,<channel name>,etc
    std::vector<std::string> tokens2 = split(buffer, ' ');
    if (tokens2.size() < 2) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "PART").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "PART").c_str()), 0);
        return;
    }
    std::vector<std::string> tokens = split(tokens2[1], ',');
    if (tokens.empty()) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "PART").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "PART").c_str()), 0);
        return;
    }
    std::string channelNames = tokens[0];
    // if there is \n at the end of the last channel name
    removeTrailingCarriageReturn(tokens[tokens.size() - 1]);
    for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
        std::string channelName;
        if (it->c_str()[0] != '#')
            channelName = "#" + *it;
        else
            channelName = *it;
        if (_channels[channelName]) {
            _channels[channelName]->ClientLeft(*senderClient, 0);
        }
        else {
            send(clientSocket, ERR_NOSUCHCHANNEL(senderClient,channelName).c_str(), std::strlen(ERR_NOSUCHCHANNEL(senderClient, channelName).c_str()), 0);
        }
    }
}
