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

void Server::showChannels(int clientSocket) {
    const char* serverList = "Server list, join by using /JOIN <channel name>: \n";
    send(clientSocket, serverList, std::strlen(serverList), 0);
    if (_channels.empty()) {
        const char* noChannel = "No channel available, create one using /JOIN <channel name>\n";
        send(clientSocket, noChannel, std::strlen(noChannel), 0);
    }
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        std::string channelName = it->first;
        send(clientSocket, channelName.c_str(), channelName.length(), 0);
        send(clientSocket, "\n", 1, 0);
    }
}

void Server::handleExistingConnection(int clientSocket) {
    char buffer[BUFFER_SIZE] = {};
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    std::deque<Client>::iterator it = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
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
            // std::string leaveMessage = it->_name + " has left the chat.\n";
            // broadcastMessage(clientSocket, leaveMessage);

            Channel *currentChannel = it->currentChannel;
            if (currentChannel) {
                it->currentChannel->ClientLeft(*it);
                if (currentChannel->_operators.empty()) {
                    _channels.erase(currentChannel->_name);
                    delete currentChannel;
                }
            }
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
            // std::cout << std::endl;
            // std::string message = it->_name + " has joined the channel !";
            // broadcastMessage(clientSocket, message);
            showChannels(clientSocket);
        }
        //il s'agit d'un message ou d'une commande, agir en conséquence (ici il n'y a que pour un message)
        else
        {
            //command COMMENCER PAR JOIN (create si existe pas)
            std::deque<Client>::iterator senderClient = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
            if (senderClient != _clients.end()) {
                //commande
                if (buffer[0] == '/') {
                    //if channel already exists join it, if not create it
                    if (!strncmp(buffer, "/LIST", 5)) {
                        showChannels(clientSocket);
                    }
                    else if (!strncmp(buffer, "/JOIN", 5)) {
                        std::string channelname2 = buffer;
                        if (channelname2.length() <= 7) {
                            const char* message = "Please specify a channel name\n";
                            send(clientSocket, message, std::strlen(message), 0);
                            return;
                        }
                        std::string channelName = buffer + 6;
                        channelName.erase(channelName.length() - 1);
                        Channel *currentChannel = senderClient->currentChannel;
                        if (currentChannel) {
                            senderClient->currentChannel->ClientLeft(*senderClient);
                            if (currentChannel->_operators.empty()) {
                                _channels.erase(currentChannel->_name);
                                delete currentChannel;
                            }
                        }
                        if (_channels[channelName]) {
                            _channels[channelName]->ClientJoin(*senderClient);
                        }
                        else
                            _channels[channelName] = new Channel(*senderClient, channelName);
                    }
                    else if (!strncmp(buffer, "/LEAVE", 6)) {
                        if (senderClient->currentChannel)
                            senderClient->currentChannel->ClientLeft(*senderClient);
                    }
                    else {
                        std::string message = "\nUnknown command\n\0";
                        send(clientSocket, message.c_str(), message.length(), 0);
                    }
                }
                //message
                else {
                    std::string message = "\n" + senderClient->_name + ": " + buffer + "\0";
                    //le client est dans un channel
                    if (senderClient->currentChannel)
                        senderClient->currentChannel->sendMessage(message, *senderClient);
                    //broadcastMessage(clientSocket, message);
                }
            }
        }
    }
}

void Server::broadcastMessage(int senderSocket, const std::string& message) {
    std::cout << "DEBUG" << message << std::endl;
    for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_socket != senderSocket) {
            send(it->_socket, message.c_str(), message.length(), 0);
        }
    }
}
