#include "Serveur.hpp"
#include "Channel.hpp"
#include <string>
#include <stdio.h>

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

void Server::quit(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    (void)buffer;

    // leaves all channels he is in using broadcastMessage to send a message to all clients in the channel
    for (std::vector<Channel *>::iterator it = senderClient->_channels.begin(); it != senderClient->_channels.end(); ++it) {
        std::string message = ":" + senderClient->_name + " PART " + (*it)->_name + " :Leaving\r\n";
        (*it)->broadcastMessage(message);
        (*it)->ClientLeft(*senderClient);
    }
    std::string message = "QUIT :Leaving\r\n";
    send(clientSocket, message.c_str(), message.length(), 0);
    close(clientSocket);
    _clients.erase(std::remove_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket)), _clients.end());
    FD_CLR(clientSocket, &_masterSet);
    std::cout << "Client " << senderClient->_name << " has left the server" << std::endl;
}

void Server::ping(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    (void)buffer;
    (void)senderClient;
    std::string message = "PONG :localhost\r\n";
    std::cout << "PONG :localhost\r\n" << std::endl;
    send(clientSocket, message.c_str(), message.length(), 0);
}

void Server::whois(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    if (std::strlen(buffer) <= 7) {
        std::string message = ":localhost 431" + senderClient->_name + ":Please specify a client to find\r\n";
        std::cout << message << std::endl;
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
        for (size_t i = 0; i < std::strlen(it->_name.c_str()); i++)
                    printf("name2 %d %c\n", it->_name.c_str()[i], it->_name.c_str()[i]);
        if (it->_name == clientToFind)
            break ;
    }
    if (it == _clients.end()) {
        const char* message = ":localhost 401 :No such nick\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message, std::strlen(message), 0);
        return;
    }
    std::string message = ":localhost 311 " + senderClient->_name + " " + it->_name + " localhost 8080\r\n";
    std::cout << message << std::endl;
    send(clientSocket, message.c_str(), message.length(), 0);
    //list all channels the client is in from his _channels vector
    for (std::vector<Channel *>::iterator it2 = it->_channels.begin(); it2 != it->_channels.end(); ++it2) {
        std::string message = ":localhost 319 " + senderClient->_name + " " + it->_name + " " + (*it2)->_name + "\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    // end of list
    std::string message2 = ":localhost 318 " + senderClient->_name + it->_name + " :End of /WHOIS list\r\n";
    std::cout << message2 << std::endl;
    send(clientSocket, message2.c_str(), message2.length(), 0);
}

void Server::inviteUser(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    //set senderClient the current channel to the channel he wants to invite to, command must be /INVITE <client name> <channel name>
    std::string channelName = buffer + 7;
    std::string clientToInviteName;
    std::cout << "channelName1: " << channelName << std::endl;
    std::vector<std::string> tokens = split(channelName, ' ');
    if (tokens.size() < 2) {
        const char* message = ":localhost 461 :Not enough parameters\r\n";
        send(clientSocket, message, std::strlen(message), 0);
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
    std::cout << "clientToInviteName: " << clientToInviteName << std::endl;
    if (it == _clients.end())
    {
        // show each char of clientToInviteName for debug
        for (size_t i = 0; i < std::strlen(clientToInviteName.c_str()); i++)
            printf("name %d %c\n", clientToInviteName.c_str()[i], clientToInviteName.c_str()[i]);
        std::string message = ":localhost 401 " + senderClient->_name + " " + channelName  + " :No such nick\r\n";
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    std::cout << "channelName: " << channelName << std::endl;
    if (!senderClient->currentChannel->isInviteOnly() && senderClient->currentChannel->isOperator(senderClient->_name)) {
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
    else {
        std::string message = ":localhost 482 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :You're not a channel operator\r\n";
        send(clientSocket, message.c_str(), message.length(), 0);
    }
}

void Server::changeTopic(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    if (std::strlen(buffer) <= 6) {
        const char* message = ":localhost 461 :Not enough parameters\r\n";
        send(clientSocket, message, std::strlen(message), 0);
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
            // check if operator
            if (!_channels[channelName]->isOperator(senderClient->_name)) {
                std::string message = ":localhost 482 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :You're not a channel operator\r\n";
                send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
                return;
            }
            if (!_channels[channelName]->_canSetTopic) {
                std::string message = ":localhost 460 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :Channel topic is locked\r\n";
                send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
                return;
            }
            if (topic[0] == ':')
                topic = topic.substr(1, topic.length() - 1);
            _channels[channelName]->setTopic(topic);
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
        const char* message = ":localhost 431 :Please specify a nickname\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message, std::strlen(message), 0);
        return;
    }
    newNick.erase(newNick.length() - 1);
    if (newNick[newNick.length() - 1] == '\r')
        newNick.erase(newNick.length() - 1);
    for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_name == newNick) {
            std::string message = ":localhost 433 * " + newNick + " :Nickname is already taken\r\n";
            std::cout << message << std::endl;
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
        std::cout << welcomeMessage << std::endl;
    } else {
        std::string message = ":localhost 001 " + nickname2 + " :Your nickname is now " + nickname2 + "\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    senderClient->nickname = nickname2;
    senderClient->_name = nickname2;
}

void Server::leaveChannel(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    // should be /PART <channel name>,<channel name>,etc
    std::vector<std::string> tokens2 = split(buffer, ' ');
    if (tokens2.size() < 2) {
        const char* message = ":localhost 461 :Not enough parameters\r\n";
        send(clientSocket, message, std::strlen(message), 0);
        return;
    }

    std::vector<std::string> tokens = split(tokens2[1], ',');
    if (tokens.empty()) {
        const char* message = ":localhost 461 :Not enough parameters\r\n";
        send(clientSocket, message, std::strlen(message), 0);
        return;
    }
    std::string channelNames = tokens[0];
    // if there is \n at the end of the last channel name
    if (tokens[tokens.size() - 1].find("\n") != std::string::npos)
        tokens[tokens.size() - 1].erase(tokens[tokens.size() - 1].length() - 1);
    if (tokens[tokens.size() - 1].find("\r") != std::string::npos)
        tokens[tokens.size() - 1].erase(tokens[tokens.size() - 1].length() - 1);
    for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
        if (_channels[*it]) {
            _channels[*it]->ClientLeft(*senderClient);
        }
    }
}

void Server::kickUser(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    // get channelname and set current channel to it
    std::vector<std::string> tokens3 = split(buffer, ' ');
    std::cout << "tokens3: size " << tokens3.size() << std::endl;
    if (tokens3.size() < 3) {
        const char* message = ":localhost 461 :Not enough parameters\r\n";
        send(clientSocket, message, std::strlen(message), 0);
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
            std::cout << "Kicking : " << *it << senderClient->currentChannel->_clients.count(*it) << std::endl;
            if (senderClient->currentChannel->_clients.count(*it)) {
                if (senderClient->_name == *it) {
                    std::string message = ":localhost 461 :You can't kick yourself\r\n";
                    send(clientSocket, message.c_str(), message.length(), 0);
                    return;
                }
                Client &client = *(senderClient->currentChannel->_clients[*it]);
                std::string notification = ":" + *it + " PART " + senderClient->currentChannel->_name + " :You have been kicked from the channel.\r\n";
                send((client)._socket, notification.c_str(), notification.length(), 0);
                client.currentChannel = NULL;
                senderClient->currentChannel->_clients.erase(client._name);
                notification = ":" + senderClient->_name + " KICK " + senderClient->currentChannel->_name + " " + *it + " :have been kicked from the channel.\r\n";
                senderClient->currentChannel->broadcastMessage(notification);
                senderClient->currentChannel->_operators.erase(client._name);
            }
        }
        // clientToKick.erase(clientToKick.length() - 1);
        // if (clientToKick.compare(senderClient->_name))
        //     senderClient->currentChannel->ClientKick(clientToKick);
        // else {
        //     std::string message = "\nError: You can't kick yourself\n";
        //     send(clientSocket, message.c_str(), message.length(), 0);
        // }

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
        const char* message = ":localhost 461 :Not enough parameters\r\n";
        send(clientSocket, message, std::strlen(message), 0);
        return;
    }
    std::string channelName = tokens[1];
    if (channelName.find("\n") != std::string::npos)
        channelName.erase(channelName.length() - 1);
    if (channelName.find("\r") != std::string::npos)
        channelName.erase(channelName.length() - 1);
    if (!_channels[channelName]) {
        const char* message = ":localhost 403 :No such channel\r\n";
        send(clientSocket, message, std::strlen(message), 0);
        return;
    }
    std::string mode2 = tokens[2];
    const char *mode = mode2.c_str();
    std::cout << "mode: " << mode << std::endl;
    if (mode2.length() < 2){
        const char* message = ":localhost 461 :Not enough parameters\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message, std::strlen(message), 0);
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
        std::cout << message << std::endl;
        send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
        return;
    }
    else if (!senderClient->currentChannel->isOperator(senderClient->_name)) {
        std::string message = ":localhost 482 " + senderClient->_name + " " + senderClient->currentChannel->_name + " :You're not a channel operator\r\n";
        std::cout << message << std::endl;
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
            const char* message = ":localhost 461 :Not enough parameters\r\n";
            send(clientSocket, message, std::strlen(message), 0);
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
            const char* message = ":localhost 461 :Not enough parameters\r\n";
            send(clientSocket, message, std::strlen(message), 0);
            return;
        }
        std::string clientToOp = tokens[3];
        senderClient->currentChannel->addOperator(clientToOp);
    }
    else if (!strncmp(mode, "-o", 2)) {
        if (tokens.size() < 4) {
            const char* message = ":localhost 461 :Not enough parameters\r\n";
            send(clientSocket, message, std::strlen(message), 0);
            return;
        }
        std::string clientToDeop = tokens[3];
        senderClient->currentChannel->removeOperator(clientToDeop);
    }
    else if (!strncmp(mode, "+k", 2)) {
        if (tokens.size() < 4) {
            const char* message = ":localhost 461 :Not enough parameters\r\n";
            std::cout << message << std::endl;
            send(clientSocket, message, std::strlen(message), 0);
            return;
        }
        std::string password = tokens[3];
        if (password.find("\n") != std::string::npos)
            password.erase(password.length() - 1);
        if (password.find("\r") != std::string::npos)
            password.erase(password.length() - 1);
        std::cout << "password: " << password << std::endl;
        senderClient->currentChannel->setPasswd(password);
        std::string message = "Password has been set to " + password + "\r\n";
        std::cout << message << std::endl;
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    else if (!strncmp(mode, "-k", 2)) {
        if (!senderClient->currentChannel->_isPasswordProtected) {
            const char* message = ":localhost 467 :Channel key not set\r\n";
            send(clientSocket, message, std::strlen(message), 0);
            return;
        }
        senderClient->currentChannel->removePasswd();
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
        const char* message = "Please specify a channel name\r\n";
        send(clientSocket, message, std::strlen(message), 0);
        return;
    }
    if (tokens.size() > 3) {
        const char* message = "Too many arguments\r\n";
        send(clientSocket, message, std::strlen(message), 0);
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
            if (_channels[*it]) {
                if (_channels[*it]->_isPasswordProtected && (itPassword != password.end() && _channels[*it]->_password == *itPassword)) {
                    std::string message = ":localhost 475 " + senderClient->_name + " " + *it + " :Wrong password\r\n";
                    send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
                    return;
                }
                _channels[*it]->ClientJoin(*senderClient);
            } else {
                // if channel doesnt start with #, add it
                if (it->c_str()[0] != '#') {
                    std::string channelName = "#" + *it;
                    _channels[channelName] = new Channel(*senderClient, channelName);
                }
                else {
                    _channels[*it] = new Channel(*senderClient, *it);
                }
            }
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
            if (_channels[*it]) {
                if (_channels[*it]->_isPasswordProtected) {
                    std::string message = ":localhost 475 " + senderClient->_name + " " + *it + " :Password required\r\n";
                    send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
                    return;
                }
                _channels[*it]->ClientJoin(*senderClient);
            }
            else {
                for (size_t i = 0; i < std::strlen(it->c_str()); i++)
                    printf("name %d %c\n", it->c_str()[i], it->c_str()[i]);
                // if channel doesnt start with #, add it
                if (it->c_str()[0] != '#') {
                    std::string channelName = "#" + *it;
                    _channels[channelName] = new Channel(*senderClient, channelName);
                }
                else {
                    _channels[*it] = new Channel(*senderClient, *it);
                }
            }
        }
    }


    // //split the buffer to get the channel name and password
    // char delim = ' ';
    // std::vector<std::string> tokens = split(buffer, delim);
    // if (tokens.size() < 2) {
    //     const char* message = "Please specify a channel name\n";
    //     send(clientSocket, message, std::strlen(message), 0);
    //     return;
    // }
    // std::string channelName = tokens[1];
    // channelName.erase(channelName.length() - 1);
    // if (tokens.size() > 3) {
    //     // too many arguments
    //     const char* message = "Too many arguments\n";
    //     send(clientSocket, message, std::strlen(message), 0);
    // }
    // else if (tokens.size() == 3) {
    //     std::string password = tokens[2];
    //     if (_channels[channelName]->_isPasswordProtected && _channels[channelName]->_password != password) {
    //         const char* message = "Wrong password\n";
    //         send(clientSocket, message, std::strlen(message), 0);
    //         return;
    //     }
    //     _channels[channelName]->ClientJoin(*senderClient);
    // }
    // else {
    //     if (_channels[channelName]) {
    //         if (_channels[channelName]->_isPasswordProtected) {
    //             const char* message = "Please specify a password\n";
    //             send(clientSocket, message, std::strlen(message), 0);
    //             return;
    //         }
    //         _channels[channelName]->ClientJoin(*senderClient);
    //     }
    //     else {
    //         _channels[channelName] = new Channel(*senderClient, channelName);
    //     }
    // }
    // std::string password;

    // std::string channelname2 = buffer + 6;
    // size_t space_pos = channelname2.find(" ");
    // std::string password;
    // if (space_pos != std::string::npos) {
    //     password = channelname2.substr(space_pos + 1);
    //     channelname2 = channelname2.substr(0, space_pos);
    // }
    // std::cout << buffer << channelname2 << std::endl;

    // if (channelname2.length() <= 0) {
    //     const char* message = "Please specify a channel name\n";
    //     send(clientSocket, message, std::strlen(message), 0);
    //     return;
    // }
    // size_t pos = channelname2.find("\n");
    // if (pos != std::string::npos)
    //     channelname2.erase(pos);
    // std::string channelName = channelname2;
    // // channelName.erase(channelName.length() - 1);
    // Channel *currentChannel = senderClient->currentChannel;
    // // if (currentChannel && !currentChannel->_isPasswordProtected)
    // //     channelName.erase(channelName.length() - 1);
    // if (currentChannel && currentChannel->_name == channelName) {
    //     const char* message = "You are already in this channel\n";
    //     send(clientSocket, message, std::strlen(message), 0);
    //     return;
    // }
    // if (currentChannel) {
    //     senderClient->currentChannel->ClientLeft(*senderClient);
    //     if (currentChannel->_operators.empty()) {
    //         _channels.erase(currentChannel->_name);
    //         delete currentChannel;
    //     }
    // }
    // if (_channels[channelName]) {
    //     if (_channels[channelName]->_isPasswordProtected && password.empty()) {
    //         const char* message = "Please specify a password\n";
    //         send(clientSocket, message, std::strlen(message), 0);
    //         return;
    //     }
    //     if (password.empty())
    //         password = " ";
    //     password.erase(password.length() - 1);
    //     if (_channels[channelName]->_isPasswordProtected && _channels[channelName]->_password != password) {
    //         const char* message = "Wrong password\n";
    //         send(clientSocket, message, std::strlen(message), 0);
    //         return;
    //     }
    //     std::cout << "clients socket: " << senderClient->_socket << std::endl;
    //     _channels[channelName]->ClientJoin(*senderClient);
    // }
    // else
    //     _channels[channelName] = new Channel(*senderClient, channelName);
}

void Server::privateMessage(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    (void)clientSocket;
    std::string command = buffer + 8;
    std::cout << "command: " << command << std::endl;
    if (command[0] == '#') {
        std::string channelName = command.substr(0, command.find(" "));
        std::cout << "Channel name :" << channelName << std::endl;
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

void Server::start() {
    std::cout << "Server listening on port " << _port << "..." << std::endl;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    while (true) {
        fd_set readSet = _masterSet;

        int result = select(_maxFd + 1, &readSet, NULL, NULL, &timeout);
        if (result == -1) {
            std::cerr << "Error in select" << std::endl;
            exit(1);
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
        exit(1);
    }

    //option_value = 1 pour activer l'option SO_REUSEADDR (permet de relancer le serveur + rapidement sur une même addresse)
    int option_value = 1;
    socklen_t optlen = sizeof(option_value);
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &option_value, optlen) == -1) {
        std::cout << "error: setsockopt" << std::endl;
        close(socketDescriptor);
        exit(EXIT_FAILURE);
    }
    return socketDescriptor;
}

void Server::bindSocket() {
    //creation du serveur avec le _port
    sockaddr_in serverAddress;
    //memset(&serverAddress, 0, sizeof(serverAddress)) aurait été bien mais on a pas le droit
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket to address" << std::endl;
        exit(1);
    }

    struct protoent *proto;
    // Récupération des informations sur le protocole TCP
    proto = getprotobyname("tcp");
    if (proto == NULL) {
        std::cout << "error: getprotobyname" << std::endl;
        exit(1);
    }

    // Affichage des informations sur le protocole TCP
    std::cout << "Nom du protocole: " << proto->p_name << std::endl;
    std::cout << "Numéro de protocole: " << proto->p_proto << std::endl;

    // Récupération des informations de l'addresse ip + port
    socklen_t addr_len = sizeof(serverAddress);
    if (getsockname(_serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), &addr_len) == -1) {
        std::cout << "error: getsockname" << std::endl;
        close(_serverSocket);
        exit(1);
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
        exit(1);
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


}

void Server::listenForConnections() {
    if (listen(_serverSocket, BACKLOG) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        exit(1);
    }
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
        if (it != _clients.end() && it->_name.empty() && buffer[0] != '\0') //le parsing devra check si le name est valid !!!!
        {
            for (int i = 0; buffer[i]; i++)
            {
                if (buffer[i] == '\n')
                {
                    buffer[i] = 0;
                    break;
                }
            }
            if (!strncmp(buffer, "NICK", 4)) {
                changeNick(buffer, clientSocket, it);
                return ;
            }
            if (!strncmp(buffer, "CAP", 3)) {
                if (it->_name.empty())
                    return ;
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
                return;
            }
            std::string notRegistered = ":localhost 451 :You have not registered\r\n";
            send(clientSocket, notRegistered.c_str(), notRegistered.length(), 0);
            // std::cout << std::endl;
            // std::string message = it->_name + " has joined the channel !";
            // broadcastMessage(clientSocket, message);
            // showChannels(buffer, clientSocket, it);
        }
        //il s'agit d'un message ou d'une commande, agir en conséquence (ici il n'y a que pour un message)
        else
        {
            //command COMMENCER PAR JOIN (create si existe pas)
            std::deque<Client>::iterator senderClient = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
            if (senderClient != _clients.end() && !senderClient->_name.empty()) {
                //commande
                if (handleCommand(buffer, clientSocket, senderClient) == 0)
                    return;
                //message
                    //le client est dans un channel
                if (senderClient->currentChannel)
                {
                    std::string message = buffer;
                    senderClient->currentChannel->sendMessage(message, *senderClient);
                }
                //broadcastMessage(clientSocket, message);
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

    // std::string message = "\nUnknown command\n\0";
    // send(clientSocket, message.c_str(), message.length(), 0);
}

void Server::broadcastMessage(int senderSocket, const std::string& message) {
    for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_socket != senderSocket) {
            send(it->_socket, message.c_str(), message.length(), 0);
        }
    }
}
