#include "../Serveur.hpp"

void Server::ping(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    static_cast<void>(buffer);
    static_cast<void>(senderClient);
    std::string message = "PONG :localhost\r\n";
    std::cout << "PONG :localhost\r\n" << std::endl;
    send(clientSocket, message.c_str(), message.length(), 0);
}
