/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setMode.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:17 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 09:39:45 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

int inviteOnly(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode, std::vector<std::string> tokens);
int limiteSet(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode, char *buffer, std::vector<std::string> tokens);
int setPasswd(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode, std::vector<std::string> tokens);
int setTopicMode(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode);

void Server::setMode(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    // should be /MODE <channel name> <mode> <client name>

    //Parse args and buffer, do checks on channel and client
    std::vector <std::string> tokens = split(buffer, ' ');
    if (tokens.size() < 3) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "MODE").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "MODE").c_str()), 0);
        return;
    }
    std::string channelName = tokens[1];
    removeTrailingCarriageReturn(channelName);
    if (!_channels[channelName]) {
        send(clientSocket, ERR_NOSUCHCHANNEL(senderClient,channelName).c_str(), std::strlen(ERR_NOSUCHCHANNEL(senderClient, channelName).c_str()), 0);
        return;
    }
    std::string mode2 = tokens[2];
    const char *mode = mode2.c_str();
    if (mode2.length() < 2) {
        send(clientSocket, ERR_NEEDMOREPARAMS(senderClient, "MODE").c_str(), std::strlen(ERR_NEEDMOREPARAMS(senderClient, "MODE").c_str()), 0);
        return;
    }
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
    else if (!senderClient->currentChannel->isOperator(senderClient->_name)) {
        send(clientSocket, ERR_CHANOPPRIVSNEEDED(senderClient, channelName).c_str(), std::strlen(ERR_CHANOPPRIVSNEEDED(senderClient, channelName).c_str()), 0);
        return;
    }

    // Check and execute mode
    if (inviteOnly(clientSocket, senderClient, mode, tokens))
        return;
    else if (limiteSet(clientSocket, senderClient, mode, buffer, tokens))
        return;
    else if (operatorAdd(clientSocket, senderClient, mode, tokens))
        return;
    else if (setPasswd(clientSocket, senderClient, mode, tokens))
        return;
    else if (setTopicMode(clientSocket, senderClient, mode))
        return;
    else {
        std::string message = ":localhost 472 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :Unknown mode\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
    }
}

int inviteOnly(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode, std::vector<std::string> tokens) {
    if (!strncmp(mode, "+i", 2)) {
        if (senderClient->currentChannel->_isPasswordProtected)
            setPasswd(clientSocket, senderClient, "-k", tokens);
        senderClient->currentChannel->setInviteOnly(true);
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Invite only added", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Invite only added", "").c_str()), 0);
        return (1);
    }
    else if (!strncmp(mode, "-i", 2)) {
        senderClient->currentChannel->setInviteOnly(false);
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Invite only removed", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Invite only removed", "").c_str()), 0);
        return (1);
    }
    return (0);
}

int limiteSet(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode, char *buffer, std::vector<std::string> tokens) {
    static_cast<void>(buffer);
    if (!std::strncmp(mode, "+l", 2)) {
        if (tokens.size() < 4) {
                send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Error, not enough parameters", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Error, not enough parameters", "").c_str()), 0);
                return (1);
            }
        int limit = std::atoi(tokens[3].c_str());
        senderClient->currentChannel->setLimit(limit);
        std::stringstream ss;
        ss << limit;
        std::string limitStr = ss.str();
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Limit to channel set to ", limitStr).c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Limit to channel set to ", limitStr).c_str()), 0);
        return (1);
    }
    else if (!std::strncmp(mode, "-l", 2)) {
        senderClient->currentChannel->setLimit(0);
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Limit to channel removed", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Limit to channel removed", "").c_str()), 0);
        return (1);
    }
    return (0);
}

int Server::operatorAdd(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode, std::vector<std::string> tokens) {
    if (!std::strncmp(mode, "+o", 2)) {
        if (tokens.size() < 4) {
            send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Error, not enough parameters", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Error, not enough parameters", "").c_str()), 0);
            return (1);
        }
        std::string clientToOp = tokens[3];
        removeTrailingCarriageReturn(clientToOp);
        for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->_name == clientToOp) {
                if (senderClient->currentChannel->_clients.count(clientToOp)) {
                    senderClient->currentChannel->addOperator(it->_name);
                    send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "+o, added operator", clientToOp).c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "+o, added operator", clientToOp).c_str()), 0);
                    return (1);
                }
                else {
                    send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "User not in channel", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "User not in channel", "").c_str()), 0);
                    return (1);
                }
            }
        }
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "No such nick", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "No such nick", "").c_str()), 0);
        return (1);
    }
    else if (!std::strncmp(mode, "-o", 2)) {
        if (tokens.size() < 4) {
            send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Error, not enough parameters", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Error, not enough parameters", "").c_str()), 0);
            return (1);
        }
        std::string clientToOp = tokens[3];
        removeTrailingCarriageReturn(clientToOp);
        for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->_name == clientToOp) {
                if (senderClient->currentChannel->_clients.count(clientToOp)) {
                    senderClient->currentChannel->removeOperator(it->_name);
                    send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "-o, Removed operator", it->_name).c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "-o, Removed operator", it->_name).c_str()), 0);
                    return (1);
                }
                else {
                    send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "User not in channel", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "User not in channel", "").c_str()), 0);
                    return (1);
                }
            }
        }
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "No such nick", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "No such nick", "").c_str()), 0);
        return (1);
    }
    return (0);
}

int setPasswd(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode, std::vector<std::string> tokens) {
    if (!std::strncmp(mode, "+k", 2)) {
        if (tokens.size() < 4) {
            send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Error, not enough parameters", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Error, not enough parameters", "").c_str()), 0);
            return (1);
        }
        std::string password = tokens[3];
        removeTrailingCarriageReturn(password);
        senderClient->currentChannel->setPasswd(password);
        if (senderClient->currentChannel->_isInviteOnly)
            inviteOnly(clientSocket, senderClient, "-i", tokens);
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Password has been set to", password).c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Password has been set to", password).c_str()), 0);
        return (1);
    }
    else if (!std::strncmp(mode, "-k", 2)) {
        if (!senderClient->currentChannel->_isPasswordProtected) {
            send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Channel is not password protected", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Channel is not password protected", "").c_str()), 0);
            return (1);
        }
        senderClient->currentChannel->removePasswd();
        // send(clientSocket, message.c_str(), message.length(), 0);
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Password removed", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Password removed", "").c_str()), 0);
        return (1);
    }
    return (0);
}

int setTopicMode(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode) {
    if (!std::strncmp(mode, "+t", 2)) {
        senderClient->currentChannel->setModeTopic(true);
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Topic mode restrictions has been set to true", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Topic mode has been set to true", "").c_str()), 0);
        return (1);
    }
    else if (!std::strncmp(mode, "-t", 2)) {
        senderClient->currentChannel->setModeTopic(false);
        send(clientSocket, RPL_MODE(senderClient, senderClient->currentChannel->_name, "Topic mode restrictions has been set to false", "").c_str(), std::strlen(RPL_MODE(senderClient, senderClient->currentChannel->_name, "Topic mode has been set to false", "").c_str()), 0);
        return (1);
    }
    return (0);
}
