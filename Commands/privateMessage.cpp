#include "../Serveur.hpp"

void Server::privateMessage(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    static_cast<void>(clientSocket);
    std::string command = buffer + 8;
    if (command[0] == '#') {
        std::string channelName = command.substr(0, command.find(" "));
        std::string textToSend = command.substr(command.find(" ") + 1);
        if (_channels.find(channelName) != _channels.end()) {
            if (_channels[channelName]->_clients.count(senderClient->_name)) {
                if (textToSend[textToSend.length() - 1] == '\n')
                    textToSend.erase(textToSend.length() - 1);
                if (textToSend[textToSend.length() - 1] == '\r')
                    textToSend.erase(textToSend.length() - 1);
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
        std::string receiverName = command.substr(0, command.find(" "));
        command = command.substr(space_pos + 1);
        if (space_pos != std::string::npos) {
            // std::string receiverName = command.substr(0, space_pos);
            std::string textToSend = command.substr(command.find(" ") + 1);
            std::cout << command << std::endl;
            if (textToSend[textToSend.length() - 1] == '\n')
                textToSend.erase(textToSend.length() - 1);
            if (textToSend[textToSend.length() - 1] == '\r')
                textToSend.erase(textToSend.length() - 1);
            for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
                std::cout << it->_name << " " << receiverName << std::endl;
                if (it->_name == receiverName) {
                    if (textToSend[0] == ':')
                        textToSend.erase(0, 1);
                    std::string message = ":" + senderClient->_name + " PRIVMSG " + receiverName + " :" + textToSend + "\r\n";
                    std::cout << message << std::endl;
                    send(it->_socket, message.c_str(), message.length(), 0);
                    break;
                }
            }
        }
    }
}
