#include "../Serveur.hpp"

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
    if (!std::strncmp(mode, "+i", 2)) {
        senderClient->currentChannel->setInviteOnly(true);
    }
    else if (!std::strncmp(mode, "-i", 2)) {
        senderClient->currentChannel->setInviteOnly(false);
    }
    else if (!std::strncmp(mode, "+l", 2)) {
        if (tokens.size() < 4) {
            std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
            return;
        }
        int limit = std::atoi(tokens[3].c_str());
        senderClient->currentChannel->setLimit(limit);
    }
    else if (!std::strncmp(mode, "-l", 2)) {
        senderClient->currentChannel->setLimit(0);
    }
    else if (!std::strncmp(mode, "+o", 2)) {
        if (tokens.size() < 4) {
            std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
            return;
        }
        std::string clientToOp = tokens[3];
        senderClient->currentChannel->addOperator(clientToOp);
    }
    else if (!std::strncmp(mode, "-o", 2)) {
        if (tokens.size() < 4) {
            std::string message = ":localhost 461 " + senderClient->_name + " " + buffer + " :Not enough parameters.\r\n";
            send(clientSocket, message.c_str(), std::strlen(message.c_str()), 0);
            return;
        }
        std::string clientToDeop = tokens[3];
        senderClient->currentChannel->removeOperator(clientToDeop);
    }
    else if (!std::strncmp(mode, "+k", 2)) {
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
    else if (!std::strncmp(mode, "-k", 2)) {
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
    else if (!std::strncmp(mode, "+t", 2)) {
        senderClient->currentChannel->setModeTopic(true);
    }
    else if (!std::strncmp(mode, "-t", 2)) {
        senderClient->currentChannel->setModeTopic(false);
    }
    else {
        const char* message = "Unknown mode\r\n";
        send(clientSocket, message, std::strlen(message), 0);
    }
}