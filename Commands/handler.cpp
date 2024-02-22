#include "../Serveur.hpp"

int Server::handleCommand(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    for (int i = 0; i < 11; i++) {
        if (!strncmp(buffer, _commands[i].name.c_str(), _commands[i].name.length())) {
            (this->*_commands[i].function)(buffer, clientSocket, senderClient);
            return 0;
        }
    }
    return (1);
}
