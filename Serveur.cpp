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

    //option_value = 1 pour activer l'option SO_REUSEADDR (permet de relancer le serveur + rapidement sur une même addresse)
    int option_value = 1;
    socklen_t optlen = sizeof(option_value);
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &option_value, optlen) == -1) {
        std::cout << "error: setsockopt" << std::endl;
        close(socketDescriptor);
        exit(EXIT_FAILURE);
    }
    return socketDescriptor;
}

void Server::bindSocket() {
    //creation du serveur avec le _port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket to address" << std::endl;
        exit(1);
    }

    struct protoent *proto;
    // Récupération des informations sur le protocole TCP
    proto = getprotobyname("tcp");
    if (proto == NULL) {
        std::cout << "error: getprotobyname" << std::endl;
        exit(1);
    }

    // Affichage des informations sur le protocole TCP
    std::cout << "Nom du protocole: " << proto->p_name << std::endl;
    std::cout << "Numéro de protocole: " << proto->p_proto << std::endl;

    // Récupération des informations de l'addresse ip + port
    socklen_t addr_len = sizeof(serverAddress);
    if (getsockname(_serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), &addr_len) == -1) {
        std::cout << "error: getsockname" << std::endl;
        close(_serverSocket);
        exit(1);
    }
    std::cout << "Local ip address: " << inet_ntoa(serverAddress.sin_addr) << std::endl;
    std::cout << "Port: " << ntohs(serverAddress.sin_port) << std::endl;

    // Récupération hostname
    struct hostent *host;
    const char *hostname = "www.example.com"; // Nom de l'hôte à résoudre

    // Résolution du nom d'hôte
    host = gethostbyname(hostname);
    if (host == NULL) {
        std::cout << "error: gethostbyname" << std::endl;
        close(_serverSocket);
        exit(1);
    }
    // Affichage des informations sur l'hôte
    std::cout << "Nom de l'hôte: " << host->h_name << std::endl;
    // Affichage de l'adresse IP
    std::cout << "Adresse IP du serveur IRC: " << inet_ntoa(*(reinterpret_cast<struct in_addr *>(host->h_addr))) << std::endl;

    /*#### PAS FINIT ####*/
    // //Récupération Addrinfo
    // struct addrinfo *serverInfo;
    // std::ostringstream oss;
    // oss << ntohs(serverAddress.sin_port);
    // std::string port = oss.str();
    // int status = getaddrinfo(NULL, port.c_str(), NULL, &serverInfo);
    // if (status != 0)
    // {
    //     std::cout << "getaddrinfo error!" << std::endl;
    //     exit(1);
    // }


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
    //accepte la connection du client
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
                    handleCommand(buffer, clientSocket, senderClient);
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

void Server::handleCommand(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {
    if (!strncmp(buffer, "/LIST", 5)) {
        showChannels(clientSocket);
    }
    else if (!strncmp(buffer, "/MODE", 5)) {
        char *mode = buffer + 6;
        std::string mode2 = buffer + 6;
        if (mode2.length() <= 2){
            const char* message = "Please specify a mode\n";
            send(clientSocket, message, std::strlen(message), 0);
            return;
        }
        if (!senderClient->currentChannel) {
            const char* message = "You are not in a channel\n";
            send(clientSocket, message, std::strlen(message), 0);
            return;
        }
        else if (!senderClient->currentChannel->_operators[senderClient->_name]) {
            const char* message = "You are not an operator\n";
            send(clientSocket, message, std::strlen(message), 0);
            return;
        }
        if (!strncmp(mode, "+i", 2)) {
            senderClient->currentChannel->setInviteOnly(true);
        }
        else if (!strncmp(mode, "-i", 2)) {
            senderClient->currentChannel->setInviteOnly(false);
        }
        else if (!strncmp(mode, "+l", 2)) {
            int limit = std::atoi(mode + 3);
            senderClient->currentChannel->setLimit(limit);
        }
        else if (!strncmp(mode, "-l", 2)) {
            senderClient->currentChannel->setLimit(0);
        }
        else {
            const char* message = "Unknown mode\n";
            send(clientSocket, message, std::strlen(message), 0);
        }

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
    else if (!strncmp(buffer, "/KICK ", 6)) {
        if (senderClient->currentChannel->isOperator(senderClient->_name)) {
            std::string clientToKick = buffer + 6;
            clientToKick.erase(clientToKick.length() - 1);
            if (clientToKick.compare(senderClient->_name))
                senderClient->currentChannel->ClientKick(clientToKick);
            else {
                std::string message = "\nError: You can't kick yourself\n";
                send(clientSocket, message.c_str(), message.length(), 0);
            }

        }
        else {
            std::string message = "\nError: You don't have the required rights to execute this command\n";
            send(clientSocket, message.c_str(), message.length(), 0);
        }
    }
    else {
        std::string message = "\nUnknown command\n\0";
        send(clientSocket, message.c_str(), message.length(), 0);
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
