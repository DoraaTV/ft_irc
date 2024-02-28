/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kickUser.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:06 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 09:38:08 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::kickUser(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    // get channelname and set current channel to it
    std::vector<std::string> tokens3 = split(buffer, ' ');
    if (tokens3.size() < 3) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "KICK").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "KICK").c_str()), 0);
        return;
    }

    std::string channelName = tokens3[1];
    //set senderClient the current channel to the channel he wants to kick from
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


    if (senderClient->currentChannel->isOperator(senderClient->_name)) {
        std::string clientToKick = tokens3[2];
        std::vector<std::string> tokens = split(clientToKick, ',');
        // if there is \n at the end of the last client name
        if (tokens[tokens.size() - 1].find("\n") != std::string::npos)
            tokens[tokens.size() - 1].erase(tokens[tokens.size() - 1].length() - 1);
        //kick all user in the list
        for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
            if (senderClient->currentChannel->_clients.count(*it)) {
                Client &client = *(senderClient->currentChannel->_clients[*it]);
                std::string notification = ":" + *it + " PART " + senderClient->currentChannel->_name + " :You have been kicked from the channel.\r\n";
                send((client)._socket, notification.c_str(), notification.length(), 0);
                client.currentChannel = NULL;
                senderClient->currentChannel->_clients.erase(client._name);
                std::cout << "\033[45mClient " << senderClient->_name << " kicked : " << *it << " from channel " << senderClient->currentChannel->_name << "\033[0m" << std::endl;
                notification = ":" + senderClient->_name + " KICK " + senderClient->currentChannel->_name + " " + *it + " :Have been kicked from the channel.\r\n";
                senderClient->currentChannel->broadcastMessage(notification);
                senderClient->currentChannel->_operators.erase(client._name);
            }
            else {
                send(clientSocket, ERR_USERNOTINCHANNEL(senderClient, *it, senderClient->currentChannel->_name).c_str(), std::strlen(ERR_USERNOTINCHANNEL(senderClient, *it, senderClient->currentChannel->_name).c_str()), 0);
            }
        }
    }
    else {
        send(clientSocket, ERR_CHANOPPRIVSNEEDED(senderClient, channelName).c_str(), std::strlen(ERR_CHANOPPRIVSNEEDED(senderClient, channelName).c_str()), 0);
    }
}
