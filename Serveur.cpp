#include "Serveur.hpp"
#include "Channel.hpp"
#include <string>
#include <stdio.h>
#include <signal.h>

struct ClientFinder {
    int socketToFind;
    ClientFinder(int socketToFind) : socketToFind(socketToFind) {}
    bool operator()(const Client& _client) const {
        return _client._socket == socketToFind;
    }
};

Server::Server(int _port) : _port(_port), _maxFd(0) {
    _serverSocket = createSocket();
    FD_ZERO(&_masterSet);
    FD_SET(_serverSocket, &_masterSet);
    _maxFd = _serverSocket;
    _commands[0].name = "PRIVMSG";
    _commands[0].function = &Server::privateMessage;
    _commands[1].name = "JOIN";
    _commands[1].function = &Server::joinChannel;
    _commands[2].name = "PART";
    _commands[2].function = &Server::leaveChannel;
    _commands[3].name = "KICK";
    _commands[3].function = &Server::kickUser;
    _commands[4].name = "MODE";
    _commands[4].function = &Server::setMode;
    _commands[5].name = "NICK";
    _commands[5].function = &Server::changeNick;
    _commands[6].name = "TOPIC";
    _commands[6].function = &Server::changeTopic;
    _commands[7].name = "INVITE";
    _commands[7].function = &Server::inviteUser;
    _commands[8].name = "PING";
    _commands[8].function = &Server::ping;
    _commands[9].name = "WHOIS";
    _commands[9].function = &Server::whois;
    _commands[10].name = "QUIT";
    _commands[10].function = &Server::quit;
}

Server::~Server()
{
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (it->second)
        {
            delete (it->second);
        }
    }
    for (std::deque<Client>::iterator it2 = _clients.begin(); it2 != _clients.end(); it2++)
    {
        close(it2->_socket);
        FD_CLR(it2->_socket, &_masterSet);
    }
}

void Server::quit(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    static_cast<void>(buffer);
    // leaves all channels he is in using broadcastMessage to send a message to all clients in the channel
    for (std::vector<Channel *>::iterator it = senderClient->_channels.begin(); it != senderClient->_channels.end(); ++it) {
        std::string message = ":" + senderClient->_name + " PART " + (*it)->_name + " :Leaving\r\n";
        (*it)->_clients.erase(senderClient->_name);
        (*it)->_operators.erase(senderClient->_name);
        if ((*it)->_operators.size() == 0 && (*it)->_clients.size() > 0) {
            std::string name2 = (*it)->_clients.begin()->first;
            (*it)->addOperator(name2);
        }
        (*it)->broadcastMessage(message);
    }
    std::string message = "QUIT :Leaving\r\n";
    send(clientSocket, message.c_str(), message.length(), 0);
    close(clientSocket);
    _clients.erase(std::remove_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket)), _clients.end());
    FD_CLR(clientSocket, &_masterSet);
    std::cout << "Client " << senderClient->_name << " has left the server" << std::endl;
}

void Server::ping(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    static_cast<void>(buffer);
    static_cast<void>(senderClient);
    std::string message = "PONG :localhost\r\n";
    std::cout << "PONG :localhost\r\n" << std::endl;
    send(clientSocket, message.c_str(), message.length(), 0);
}

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

void Server::inviteUser(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    //set senderClient the current channel to the channel he wants to invite to, command must be /INVITE <client name> <channel name>
    std::string channelName = buffer + 7;
    std::string clientToInviteName;
    std::vector<std::string> tokens = split(channelName, ' ');
    if (tokens.size() < 2) {
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
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
        std::string message = ":localhost 401 " + senderClient->_name + " " + channelName  + " :No such nick\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    if (senderClient->currentChannel->isInviteOnly() && !senderClient->currentChannel->isOperator(senderClient->_name))
    {
        std::string message = ":localhost 482 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :You're not a channel operator\r\n";
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    else {
        std::cout << "Client " << senderClient->_name << " has invited client " << clientToInviteName << " to the channel " << senderClient->currentChannel->_name << std::endl;
        if (senderClient->currentChannel->_invited.count(clientToInviteName)) {
            senderClient->currentChannel->_invited.erase(clientToInviteName);
            std::string message = "Your invitation to " + senderClient->currentChannel->_name + " has been canceled.\r\n";
            send(it->_socket, message.c_str(), message.length(), 0);
        } else {
            senderClient->currentChannel->_invited[clientToInviteName] = &(*it);
            std::string message = "You have been invited to " + senderClient->currentChannel->_name + ".\r\n";
            send(it->_socket, message.c_str(), message.length(), 0);
        }
    }
}

void Server::changeTopic(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    if (std::strlen(buffer) <= 6) {
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
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
                std::string message = ":localhost 482 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :You're not a allowed to change the topic\r\n";
                send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
                return;
            }
            if (topic[0] == ':')
                topic = topic.erase(0 ,1);
            if (topic[topic.length() - 1] == '\n')
                topic.erase(topic.length() - 1);
            if (topic[topic.length() - 1] == '\r')
                topic.erase(topic.length() - 1);
            _channels[channelName]->setTopic(topic);
            std::cout << "Topic of the channel " << channelName << " has been set to " << topic << std::endl;
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
        std::string message = ":localhost 403 " + senderClient->_name + " " + channelName + " :No such channel\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
    }
}

void Server::changeNick(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    std::cout << senderClient->_name << std::endl;
    std::string newNick = buffer + 5;
    if (newNick.length() <= 1) {
        std::string message = "localhost 431 :Please specify a nickname\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    if (newNick[newNick.length() - 1] == '\n')
        newNick.erase(newNick.length() - 1);
    if (newNick[newNick.length() - 1] == '\r')
        newNick.erase(newNick.length() - 1);
    for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_name == newNick) {
            std::string message = ":localhost 433 * " + newNick + " :Nickname is already taken\r\n";
            send(clientSocket, message.c_str(), message.length(), 0);
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
    std::cout << "Client " << senderClient->_name << " changed his nickname to " << nickname2 << std::endl;
    senderClient->nickname = nickname2;
    senderClient->_name = nickname2;
}

void Server::leaveChannel(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    // should be /PART <channel name>,<channel name>,etc
    std::vector<std::string> tokens2 = split(buffer, ' ');
    if (tokens2.size() < 2) {
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    std::vector<std::string> tokens = split(tokens2[1], ',');
    if (tokens.empty()) {
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    std::string channelNames = tokens[0];
    // if there is \n at the end of the last channel name
    if (tokens[tokens.size() - 1].find("\n") != std::string::npos)
        tokens[tokens.size() - 1].erase(tokens[tokens.size() - 1].length() - 1);
    if (tokens[tokens.size() - 1].find("\r") != std::string::npos)
        tokens[tokens.size() - 1].erase(tokens[tokens.size() - 1].length() - 1);
    for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
        std::string channelName;
        if (it->c_str()[0] != '#')
            channelName = "#" + *it;
        else
            channelName = *it;
        if (_channels[channelName]) {
            _channels[channelName]->ClientLeft(*senderClient);
        }
        else {
            std::string message = ":localhost 403 " + senderClient->_name + " " + channelName + " :No such channel\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        }
    }
}

void Server::kickUser(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    // get channelname and set current channel to it
    std::vector<std::string> tokens3 = split(buffer, ' ');
    if (tokens3.size() < 3) {
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
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
        std::string message = ":localhost 442 :You're not on that channel\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
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
                if (senderClient->_name == *it) {
                    std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + ":You can't kick yourself\r\n";
                    send(clientSocket, message.c_str(), message.length(), 0);
                    return;
                }
                Client &client = *(senderClient->currentChannel->_clients[*it]);
                std::string notification = ":" + *it + " PART " + senderClient->currentChannel->_name + " :You have been kicked from the channel.\r\n";
                send((client)._socket, notification.c_str(), notification.length(), 0);
                client.currentChannel = NULL;
                senderClient->currentChannel->_clients.erase(client._name);
                std::cout << "Client " << senderClient->_name << " kicked : " << *it << " from channel " << senderClient->currentChannel->_name << std::endl;
                notification = ":" + senderClient->_name + " KICK " + senderClient->currentChannel->_name + " " + *it + " :have been kicked from the channel.\r\n";
                senderClient->currentChannel->broadcastMessage(notification);
                senderClient->currentChannel->_operators.erase(client._name);
            }
        }
    }
    else {
        std::string message = ":localhost 482 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :You're not a channel operator\r\n";
        send(clientSocket, message.c_str(), message.length(), 0);
    }
}

void Server::setMode(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    // should be /MODE <channel name> <mode> <client name>

    std::vector <std::string> tokens = split(buffer, ' ');
    if (tokens.size() < 3) {
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    std::string channelName = tokens[1];
    if (channelName.find("\n") != std::string::npos)
        channelName.erase(channelName.length() - 1);
    if (channelName.find("\r") != std::string::npos)
        channelName.erase(channelName.length() - 1);
    if (!_channels[channelName]) {
        std::string message = ":localhost 403 :No such channel\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    std::string mode2 = tokens[2];
    const char *mode = mode2.c_str();
    std::cout << "mode: " << mode << std::endl;
    if (mode2.length() < 2){
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
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
        std::string message = ":localhost 442 :You're not on that channel\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    else if (!senderClient->currentChannel->isOperator(senderClient->_name)) {
        std::string message = ":localhost 482 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :You're not a channel operator\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    if (!strncmp(mode, "+i", 2)) {
        senderClient->currentChannel->setInviteOnly(true);
    }
    else if (!strncmp(mode, "-i", 2)) {
        senderClient->currentChannel->setInviteOnly(false);
    }
    else if (!strncmp(mode, "+l", 2)) {
        if (tokens.size() < 4) {
            std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
            return;
        }
        int limit = std::atoi(tokens[3].c_str());
        senderClient->currentChannel->setLimit(limit);
    }
    else if (!strncmp(mode, "-l", 2)) {
        senderClient->currentChannel->setLimit(0);
    }
    else if (!strncmp(mode, "+o", 2)) {
        if (tokens.size() < 4) {
            std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
            return;
        }
        std::string clientToOp = tokens[3];
        senderClient->currentChannel->addOperator(clientToOp);
    }
    else if (!strncmp(mode, "-o", 2)) {
        if (tokens.size() < 4) {
            std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
            return;
        }
        std::string clientToDeop = tokens[3];
        senderClient->currentChannel->removeOperator(clientToDeop);
    }
    else if (!strncmp(mode, "+k", 2)) {
        if (tokens.size() < 4) {
            std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
            return;
        }
        std::string password = tokens[3];
        if (password.find("\n") != std::string::npos)
            password.erase(password.length() - 1);
        if (password.find("\r") != std::string::npos)
            password.erase(password.length() - 1);
        senderClient->currentChannel->setPasswd(password);
        std::string message = "Password for channel " + senderClient->currentChannel->_name + " has been set to " + password + "\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    else if (!strncmp(mode, "-k", 2)) {
        if (!senderClient->currentChannel->_isPasswordProtected) {
            std::string message = "localhost 467 :Channel key not set\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
            return;
        }
        senderClient->currentChannel->removePasswd();
        std::string message = "Password for channel " + senderClient->currentChannel->_name + " has been removed\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    else if (!strncmp(mode, "+t", 2)) {
        senderClient->currentChannel->setModeTopic(true);
    }
    else if (!strncmp(mode, "-t", 2)) {
        senderClient->currentChannel->setModeTopic(false);
    }
    else {
        const char* message = "Unknown mode\r\n";
        send(clientSocket, message, std::strlen(message), 0);
    }
}

void Server::joinChannel(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    std::vector<std::string> tokens = split(buffer, ' ');
    if (tokens.size() < 2) {
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    if (tokens.size() > 3) {
        std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Too much parameters.\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    std::vector<std::string> tokens2 = split(tokens[1], ',');
    if (tokens.size() == 3) {
        std::vector<std::string> password = split(tokens[2], ',');
        //if there is \n at the end of the last password
        if (password[password.size() - 1].find("\n") != std::string::npos)
            password[password.size() - 1].erase(password[password.size() - 1].length() - 1);
        if (password[password.size() - 1].find("\r") != std::string::npos)
            password[password.size() - 1].erase(password[password.size() - 1].length() - 1);
        std::vector<std::string>::iterator itPassword = password.begin();
        for (std::vector<std::string>::iterator it = tokens2.begin(); it != tokens2.end(); ++it) {
            std::string channelName;
            if (it->c_str()[0] != '#')
                channelName = "#" + *it;
            else 
                channelName = *it;
            if (_channels[channelName]) {
                if (_channels[channelName]->_isPasswordProtected && (itPassword != password.end() && _channels[channelName]->_password != *itPassword)) {
                    std::string message = ":localhost 475 " + senderClient->_name + " " + channelName + " :Wrong password\r\n";
                    send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
                    return;
                }
                _channels[channelName]->ClientJoin(*senderClient);
            } else
                _channels[channelName] = new Channel(*senderClient, channelName);
            if (itPassword != password.end())
                itPassword++;
        }
    }
    else {
        // if there is \n at the end of the last channel name
        if (tokens2[tokens2.size() - 1].find("\n") != std::string::npos)
            tokens2[tokens2.size() - 1].erase(tokens2[tokens2.size() - 1].length() - 1);
        if (tokens2[tokens2.size() - 1].find("\r") != std::string::npos)
            tokens2[tokens2.size() - 1].erase(tokens2[tokens2.size() - 1].length() - 1);
        for (std::vector<std::string>::iterator it = tokens2.begin(); it != tokens2.end(); ++it) {
            std::string channelName;
            if (it->c_str()[0] != '#')
                channelName = "#" + *it;
            else
                channelName = *it;
            if (_channels[channelName]) {
                if (_channels[channelName]->_isPasswordProtected) {
                    std::string message = ":localhost 475 " + senderClient->_name + " " + channelName + " :Password required\r\n";
                    send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
                    return;
                }
                _channels[channelName]->ClientJoin(*senderClient);
            }
            else {
                _channels[channelName] = new Channel(*senderClient, channelName);
            }
        }
    }
}

void Server::privateMessage(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    static_cast<void>(clientSocket);
    std::string command = buffer + 8;
    if (command[0] == '#') {
        std::string channelName = command.substr(0, command.find(" "));
        std::string textToSend = command.substr(command.find(" ") + 1);
        if (_channels.find(channelName) != _channels.end()) {
            if (_channels[channelName]->_clients.count(senderClient->_name)) {
                _channels[channelName]->sendMessage(textToSend, *senderClient);
            }
            else {
                const char* message = "You are not in this channel\n";
                send(clientSocket, message, std::strlen(message), 0);
            }
        }
        return ;
    }
    size_t space_pos = command.find(" ");
    if (space_pos != std::string::npos) {
        command = command.substr(space_pos + 1);
        space_pos = command.find(" ");
        if (space_pos != std::string::npos) {
            std::string receiverName = command.substr(0, space_pos);
            std::string textToSend = command.substr(space_pos + 1);
            for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
                if (it->_name == receiverName) {
                    std::string message = "Private Message from " + senderClient->_name + ": " + textToSend + "\r\n";
                    send(it->_socket, message.c_str(), message.length(), 0);
                    break;
                }
            }
        }
    }
}

int loop = true;

void handler(int sig, siginfo_t *info, void *ptr1)
{
    static_cast<void>(sig);
    static_cast<void>(info);
    static_cast<void>(ptr1);
    loop = false;
}

void Server::start() {
    std::cout << "Server listening on port " << _port << "..." << std::endl;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    struct sigaction	sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = &handler;
    sigaction(SIGINT, &sa, 0);
    while (loop) {
        fd_set readSet = _masterSet;

        int result = select(_maxFd + 1, &readSet, NULL, NULL, &timeout);
        if (result == -1) {
            std::cerr << "Error in select" << std::endl;
            break;
        }
        for (int i = 0; i <= _maxFd; ++i) {
            if (FD_ISSET(i, &readSet)) {
                if (i == _serverSocket) {
                    handleNewConnection(i);
                } else {
                    handleExistingConnection(i);
                }
            }
        }
    }
}

int Server::createSocket() {
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDescriptor == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return (-1);
    }

    //option_value = 1 pour activer l'option SO_REUSEADDR (permet de relancer le serveur + rapidement sur une même addresse)
    int option_value = 1;
    socklen_t optlen = sizeof(option_value);
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &option_value, optlen) == -1) {
        std::cout << "error: setsockopt" << std::endl;
        close(socketDescriptor);
        return (-1);
    }
    return socketDescriptor;
}

int Server::bindSocket() {
    //creation du serveur avec le _port
    sockaddr_in serverAddress;
    //memset(&serverAddress, 0, sizeof(serverAddress)) aurait été bien mais on a pas le droit
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket to address" << std::endl;
        return (-1);
    }

    struct protoent *proto;
    // Récupération des informations sur le protocole TCP
    proto = getprotobyname("tcp");
    if (proto == NULL) {
        std::cout << "error: getprotobyname" << std::endl;
        return (-1);
    }

    // Affichage des informations sur le protocole TCP
    std::cout << "Nom du protocole: " << proto->p_name << std::endl;
    std::cout << "Numéro de protocole: " << proto->p_proto << std::endl;

    // Récupération des informations de l'addresse ip + port
    socklen_t addr_len = sizeof(serverAddress);
    if (getsockname(_serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), &addr_len) == -1) {
        std::cout << "error: getsockname" << std::endl;
        close(_serverSocket);
        return (-1);
    }
    std::cout << "Local ip address: " << inet_ntoa(serverAddress.sin_addr) << std::endl;
    std::cout << "Port: " << ntohs(serverAddress.sin_port) << std::endl;

    // Récupération hostname
    struct hostent *host;
    const char *hostname = "localhost"; // Nom de l'hôte à résoudre

    // Résolution du nom d'hôte
    host = gethostbyname(hostname);
    if (host == NULL) {
        std::cout << "error: gethostbyname" << std::endl;
        close(_serverSocket);
        return (-1);
    }
    // Affichage des informations sur l'hôte
    std::cout << "Nom de l'hôte: " << host->h_name << std::endl;
    // Affichage de l'adresse IP
    std::cout << "Adresse IP du serveur IRC: " << inet_ntoa(*(reinterpret_cast<struct in_addr *>(host->h_addr))) << std::endl;

    /*#### PAS FINIT ####*/
    // //Récupération Addrinfo
    // struct addrinfo *serverInfo;
    // std::ostringstream oss;
    // oss << ntohs(serverAddress.sin_port);
    // std::string port = oss.str();
    // int status = getaddrinfo(NULL, port.c_str(), NULL, &serverInfo);
    // if (status != 0)
    // {
    //     std::cout << "getaddrinfo error!" << std::endl;
    //     exit(1);
    // }

    return (0);
}

int Server::listenForConnections() {
    if (listen(_serverSocket, BACKLOG) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        return (-1);
    }
    return (0);
}

// Client se connecte
void Server::handleNewConnection(int _serverSocket) {
    sockaddr_in clientAddress;
    socklen_t clientSize = sizeof(clientAddress);
    //accepte la connection du client
    int clientSocket = accept(_serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientSize);

    if (clientSocket == -1) {
        std::cerr << "Error accepting connection" << std::endl;
        return;
    }

    FD_SET(clientSocket, &_masterSet);
    if (clientSocket > _maxFd)
        _maxFd = clientSocket;

    std::cout << "New connection from " << "localhost" << " on socket " << clientSocket << std::endl; //récupérer l'addresse ip dynamiquement
    //ajout du client dans la liste des clients
    _clients.push_back(Client(clientSocket, ""));
}

void Server::handleExistingConnection(int clientSocket) {
    char buffer[BUFFER_SIZE] = {};
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    std::deque<Client>::iterator it = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
    if (it->_name.empty()) {
        std::cout << "Client " << clientSocket << " is not identified" << std::endl;
    }
    else {
        std::cout << "Client " << clientSocket << " is identified as " << it->_name << std::endl;
    }
    if (bytesReceived <= 0) {
        // Gérer la déconnexion ou l'erreur
        if (bytesReceived == 0) {
            std::cout << "Connection closed on socket " << clientSocket << std::endl;
        } else {
            std::cerr << "Error receiving data on socket " << clientSocket << std::endl;
        }

        // Fermer la connexion et la supprimer du jeu principal et de la liste des clients
        close(clientSocket);
        FD_CLR(clientSocket, &_masterSet);


        if (it != _clients.end()) {
            // Client trouvé, le supprimer
            // std::string leaveMessage = it->_name + " has left the chat.\n";
            // broadcastMessage(clientSocket, leaveMessage);
            _clients.erase(it);
        }

        // Mettre à jour le maximum des descripteurs de fichiers si nécessaire
        if (clientSocket == _maxFd) {
            while (!FD_ISSET(_maxFd, &_masterSet)) {
                _maxFd--;
            }
        }
    } else {

        std::cout << "Received from socket " << clientSocket << ": " << buffer << std::endl;
        //choisir un nom à la connexion
        std::string commandlist = buffer;
        std::vector<std::string> commands = split(commandlist, '\n');
        for (std::vector<std::string>::iterator commandit = commands.begin(); commandit != commands.end(); commandit++)
        {
            if (it != _clients.end() && it->_name.empty() && buffer[0] != '\0') //le parsing devra check si le name est valid !!!!
            {
                // for (int i = 0; buffer[i]; i++)
                // {
                //     if (buffer[i] == '\n')
                //     {
                //         buffer[i] = 0;
                //         break;
                //     }
                // }
                if (!strncmp(commandit->c_str(), "NICK", 4)) {
                    changeNick(const_cast<char*>(commandit->c_str()), clientSocket, it);
                    continue ;
                }
                if (!strncmp(commandit->c_str(), "CAP", 3)) {
                    if (it->_name.empty())
                        continue ;
                    std::string message1 = ":@localhost NICK" + it->_name + "\r\n";
                    send(clientSocket, message1.c_str(), message1.length(), 0);
                    std::string message3 = ":localhost 002 :Your host is 42_Ftirc (localhost), running version 1.1\r\n";
                    send(clientSocket, message3.c_str(), message3.length(), 0);
                    std::string message4 = ":localhost 003 :This server was created 20-02-2024 19:45:17\r\n";
                    send(clientSocket, message4.c_str(), message4.length(), 0);
                    std::string message5 = ":localhost 004 localhost 1.1 io kost k\r\n";
                    send(clientSocket, message5.c_str(), message5.length(), 0);
                    std::string message6 = ":localhost 005 CHANNELLEN=32 NICKLEN=9 TOPICLEN=307 :are supported by this server\r\n";
                    send(clientSocket, message6.c_str(), message6.length(), 0);
                    continue;
                }
                std::string notRegistered = ":localhost 451 :You have not registered\r\n";
                send(clientSocket, notRegistered.c_str(), notRegistered.length(), 0);
            }
            //il s'agit d'un message ou d'une commande, agir en conséquence (ici il n'y a que pour un message)
            else
            {
                std::deque<Client>::iterator senderClient = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
                if (senderClient != _clients.end() && !senderClient->_name.empty()) {
                    //commande
                    if (handleCommand(const_cast<char *>(commandit->c_str()), clientSocket, senderClient) == 0)
                        continue;
                    //message
                    if (senderClient->currentChannel)
                    {
                        std::string message = commandit->c_str();
                        senderClient->currentChannel->sendMessage(message, *senderClient);
                    }
                }
            }
        }
    }
}

int Server::handleCommand(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    for (int i = 0; i < 11; i++) {
        if (!strncmp(buffer, _commands[i].name.c_str(), _commands[i].name.length())) {
            (this->*_commands[i].function)(buffer, clientSocket, senderClient);
            return 0;
        }
    }
    return (1);
}

void Server::broadcastMessage(int senderSocket, const std::string& message) {
    for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_socket != senderSocket) {
            send(it->_socket, message.c_str(), message.length(), 0);
        }
    }
}
