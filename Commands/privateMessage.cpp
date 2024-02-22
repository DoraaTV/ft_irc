#include "../Serveur.hpp"

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
