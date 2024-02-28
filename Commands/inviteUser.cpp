/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inviteUser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:02 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 10:26:42 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::inviteUser(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    //set senderClient the current channel to the channel he wants to invite to, command must be /INVITE <client name> <channel name>
    std::string channelName = buffer + 7;
    std::string clientToInviteName;
    std::vector<std::string> tokens = split(channelName, ' ');
    if (tokens.size() < 2) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "INVITE").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "INVITE").c_str()), 0);
        return;
    }
    clientToInviteName = tokens[0];
    removeTrailingCarriageReturn(clientToInviteName);

    channelName = tokens[1];
    removeTrailingCarriageReturn(channelName);
    //set current channel to the channel he wants to invite to
    std::vector<Channel *>::iterator it2;
    for (it2 = senderClient->_channels.begin(); it2 != senderClient->_channels.end(); ++it2) {
        if ((*it2)->_name == channelName) {
            senderClient->currentChannel = *it2;
            break;
        }
    }
    if (it2 == senderClient->_channels.end()) {
        send(clientSocket, ERR_NOTONCHANNEL(senderClient, channelName).c_str(), std::strlen(ERR_NOTONCHANNEL(senderClient, channelName).c_str()), 0);
        return;
    }
    std::deque<Client>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); ++it)
        if (it->_name == clientToInviteName)
            break;
    if (it == _clients.end())
    {
        send(clientSocket, ERR_NOSUCHNICK(senderClient, clientToInviteName).c_str(), std::strlen(ERR_NOSUCHNICK(senderClient, clientToInviteName).c_str()), 0);
        return;
    }
    if (!_channels[channelName]) {
        send(clientSocket, ERR_NOSUCHCHANNEL(senderClient,channelName).c_str(), std::strlen(ERR_NOSUCHCHANNEL(senderClient, channelName).c_str()), 0);
        return;
    }
    if (senderClient->currentChannel->isInviteOnly() && !senderClient->currentChannel->isOperator(senderClient->_name))
    {
        send(clientSocket, ERR_CHANOPPRIVSNEEDED(senderClient, channelName).c_str(), std::strlen(ERR_CHANOPPRIVSNEEDED(senderClient, channelName).c_str()), 0);
    }
    else {
        std::cout << "\033[46mClient " << senderClient->_name << " has invited client " << clientToInviteName << " to the channel " << senderClient->currentChannel->_name << "\033[0m" << std::endl;
        if (senderClient->currentChannel->_invited[clientToInviteName]) {
            senderClient->currentChannel->_invited.erase(clientToInviteName);
            std::string message = "Your invitation to " + senderClient->currentChannel->_name + " has been canceled.\r\n";
            send(it->_socket, message.c_str(), message.length(), 0);
        } else {
            senderClient->currentChannel->_invited[clientToInviteName] = &(*it);
            send(it->_socket, RPL_INVITING(senderClient, clientToInviteName, senderClient->currentChannel->_name).c_str(), std::strlen(RPL_INVITING(senderClient, clientToInviteName, senderClient->currentChannel->_name).c_str()), 0);
        }
    }
}
