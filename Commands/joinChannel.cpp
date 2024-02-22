#include "../Serveur.hpp"

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
