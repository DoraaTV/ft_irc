#include "Serveur.hpp"
#include "Channel.hpp"

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
}

void Server::start() {
    std::cout << "Server listening on port " << _port << "..." << std::endl;

    while (true) {
        fd_set readSet = _masterSet;

        int result = select(_maxFd + 1, &readSet, NULL, NULL, NULL);
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
    return socketDescriptor;
}

void Server::bindSocket() {
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //creation du serveur avec le _port
    if (bind(_serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket to address" << std::endl;
        exit(1);
    }
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
    //accept la connection du client
    int clientSocket = accept(_serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientSize);

    if (clientSocket == -1) {
        std::cerr << "Error accepting connection" << std::endl;
        return;
    }

    FD_SET(clientSocket, &_masterSet);
    if (clientSocket > _maxFd)
        _maxFd = clientSocket;

    std::cout << "New connection from " << "localhost" << " on socket " << clientSocket << std::endl; //récupérer l'addresse ip dynamiquement
    const char* welcomeMessage = "Welcome to the chat room!\nPlease, choose a nickname: ";
    send(clientSocket, welcomeMessage, std::strlen(welcomeMessage), 0);
    //ajout du client dans la liste des clients
    _clients.push_back(Client(clientSocket, ""));
}

void Server::handleExistingConnection(int clientSocket) {
    char buffer[BUFFER_SIZE] = {};
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    std::vector<Client>::iterator it = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
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
            std::string leaveMessage = it->_name + " has left the chat.\n";
            broadcastMessage(clientSocket, leaveMessage);
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
            it->_name = buffer;
            std::cout << std::endl;
            std::string message = it->_name + " has joined the channel !";
            broadcastMessage(clientSocket, message);
        }
        //il s'agit d'un message ou d'une commande, agir en conséquence (ici il n'y a que pour un message)
        else
        {
            //command COMMENCER PAR JOIN (create si existe pas)
            std::vector<Client>::iterator senderClient = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
            if (senderClient != _clients.end()) {
                //commande
                if (buffer[0] == '/') {
                    //if channel already exists join it, if not create it
                    if (!strncmp(buffer, "/JOIN ", 6)) {
                        std::string channelName = buffer + 6;
                        channelName.erase(channelName.length() - 1);
                        if (_channels[channelName])
                            _channels[channelName]->ClientJoin(*senderClient);
                        else
                            _channels[channelName] = new Channel(*senderClient, channelName);
                    }
                }
                //message
                else {
                    std::string message = "\n" + senderClient->_name + ": " + buffer + "\0";
                    //le client est dans un channel
                    if (senderClient->currentChannel)
                        senderClient->currentChannel->broadcastMessage(message);
                    //broadcastMessage(clientSocket, message);
                }
            }
        }
    }
}

void Server::broadcastMessage(int senderSocket, const std::string& message) {
    std::cout << "DEBUG" << message << std::endl;
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_socket != senderSocket) {
            send(it->_socket, message.c_str(), message.length(), 0);
        }
    }
}
