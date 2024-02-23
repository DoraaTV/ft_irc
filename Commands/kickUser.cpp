#include "../Serveur.hpp"

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
                notification = ":" + senderClient->_name + " KICK " + senderClient->currentChannel->_name + " " + *it + " :Have been kicked from the channel.\r\n";
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
