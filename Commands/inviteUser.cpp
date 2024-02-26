#include "../Serveur.hpp"

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
    if (!_channels[channelName]) {
        send(clientSocket, ERR_NOSUCHCHANNEL(senderClient,channelName).c_str(), std::strlen(ERR_NOSUCHCHANNEL(senderClient, channelName).c_str()), 0);
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
