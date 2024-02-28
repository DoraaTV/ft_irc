/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   changeTopic.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:13:57 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 09:37:29 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::changeTopic(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    if (std::strlen(buffer) <= 6) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "TOPIC").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "TOPIC").c_str()), 0);
        return;
    }
    std::string topic;
    bool isTopic = false;
    //check if the first param is a channel and set topic if there is a second param
    std::string channelName = buffer + 6;
    // check if there is a second param (topic) then var std::string topic
    if (channelName.find(" ") != std::string::npos) {
        topic = channelName.substr(channelName.find(" ") + 1);
        channelName = channelName.substr(0, channelName.find(" "));
        isTopic = true;
    }
    else {
        channelName.erase(channelName.length() - 1);
    }
    if (_channels.find(channelName) != _channels.end()) {
        // check if there is argument after the channel name (topic name)
        if (isTopic) {
            // check if client is operator
            if (_channels[channelName]->_isTopicRestricted && !_channels[channelName]->isOperator(senderClient->_name)) {
                send(clientSocket, ERR_CHANOPPRIVSNEEDED(senderClient, channelName).c_str(), std::strlen(ERR_CHANOPPRIVSNEEDED(senderClient, channelName).c_str()), 0);
                return;
            }
            if (topic[0] == ':')
                topic = topic.erase(0 ,1);
            if (topic[topic.length() - 1] == '\n')
                topic.erase(topic.length() - 1);
            if (topic[topic.length() - 1] == '\r')
                topic.erase(topic.length() - 1);
            _channels[channelName]->setTopic(topic);
            std::cout << "\033[46mTopic of the channel " << channelName << " has been set to " << topic << "\033[0m" << std::endl;
            std::string message = ":localhost 332 " + senderClient->_name + " " + channelName + " :" + topic + "\r\n";
            _channels[channelName]->broadcastMessage(message);
            return;
        }
        else {
            //show topic of given channel
            std::string topic = _channels[channelName]->getTopic();
            std::string message = ":localhost 332 " + senderClient->_name + " " + channelName + " :" + topic + "\r\n";
            _channels[channelName]->broadcastMessage(message);
        }
        return;
    }
    else {
        send(clientSocket, ERR_NOSUCHCHANNEL(senderClient,channelName).c_str(), std::strlen(ERR_NOSUCHCHANNEL(senderClient, channelName).c_str()), 0);
    }
}
