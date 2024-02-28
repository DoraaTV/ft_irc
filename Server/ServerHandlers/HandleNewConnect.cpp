#include "../Server.hpp"

// Client se connecte
void Server::handleNewConnection(int _serverSocket) {
    sockaddr_in clientAddress;
    socklen_t clientSize = sizeof(clientAddress);
    //accepte la connection du client
    int clientSocket = accept(_serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientSize);

    if (clientSocket == -1) {
        std::cerr << "\033[41mError accepting connection\033[0m" << std::endl;
        return;
    }

    FD_SET(clientSocket, &_masterSet);
    if (clientSocket > _maxFd)
        _maxFd = clientSocket;

    std::cout << "\033[46mNew connection from " << "localhost" << " on socket " << clientSocket << "\033[0m" << std::endl; //récupérer l'addresse ip dynamiquement
    //ajout du client dans la liste des clients
    _clients.push_back(Client(clientSocket, ""));
}
