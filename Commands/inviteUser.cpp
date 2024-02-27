/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inviteUser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syakovle <syakovle@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:02 by thrio             #+#    #+#             */
/*   Updated: 2024/02/27 16:03:43 by syakovle         ###   ########.fr       */
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
    if (clientToInviteName.find("\n") != std::string::npos)
        clientToInviteName.erase(clientToInviteName.length() - 1);
    if (clientToInviteName.find("\r") != std::string::npos)
        clientToInviteName.erase(clientToInviteName.length() - 1);

    channelName = tokens[1];
    if (channelName.find("\n") != std::string::npos)
        channelName.erase(channelName.length() - 1);
    if (channelName.find("\r") != std::string::npos)
        channelName.erase(channelName.length() - 1);
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
        std::cout << "Client " << senderClient->_name << " has invited client " << clientToInviteName << " to the channel " << senderClient->currentChannel->_name << std::endl;
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
