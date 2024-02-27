#include "Serveur.hpp"
#include "Channel.hpp"
#include <string>
#include <stdio.h>
#include <signal.h>

struct ClientFinder {
    int socketToFind;
    ClientFinder(int socketToFind) : socketToFind(socketToFind) {}
    bool operator()(const Client& _client) const {
        return _client._socket == socketToFind;
    }
};

Server::Server(int port, std::string password) : _port(port), _maxFd(0), _password(password) {
    _serverSocket = createSocket();
    FD_ZERO(&_masterSet);
    FD_SET(_serverSocket, &_masterSet);
    _maxFd = _serverSocket;
    _commands[0].name = "PRIVMSG";
    _commands[0].function = &Server::privateMessage;
    _commands[1].name = "JOIN";
    _commands[1].function = &Server::joinChannel;
    _commands[2].name = "PART";
    _commands[2].function = &Server::leaveChannel;
    _commands[3].name = "KICK";
    _commands[3].function = &Server::kickUser;
    _commands[4].name = "MODE";
    _commands[4].function = &Server::setMode;
    _commands[5].name = "NICK";
    _commands[5].function = &Server::changeNick;
    _commands[6].name = "TOPIC";
    _commands[6].function = &Server::changeTopic;
    _commands[7].name = "INVITE";
    _commands[7].function = &Server::inviteUser;
    _commands[8].name = "PING";
    _commands[8].function = &Server::ping;
    _commands[9].name = "WHOIS";
    _commands[9].function = &Server::whois;
    _commands[10].name = "QUIT";
    _commands[10].function = &Server::quit;
}

Server::~Server()
{
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (it->second)
        {
            delete (it->second);
        }
    }
    for (std::deque<Client>::iterator it2 = _clients.begin(); it2 != _clients.end(); it2++)
    {
        close(it2->_socket);
        FD_CLR(it2->_socket, &_masterSet);
    }
}

int loop = true;

void handler(int sig, siginfo_t *info, void *ptr1)
{
    static_cast<void>(sig);
    static_cast<void>(info);
    static_cast<void>(ptr1);
    loop = false;
}

void Server::start() {
    std::cout << "Server listening on port " << _port << "..." << std::endl;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    struct sigaction	sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = &handler;
    sigaction(SIGINT, &sa, 0);
    while (loop) {
        fd_set readSet = _masterSet;

        int result = select(_maxFd + 1, &readSet, NULL, NULL, &timeout);
        if (result == -1) {
            std::cerr << "Error in select" << std::endl;
            break;
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
        return (-1);
    }

    //option_value = 1 pour activer l'option SO_REUSEADDR (permet de relancer le serveur + rapidement sur une même addresse)
    int option_value = 1;
    socklen_t optlen = sizeof(option_value);
    if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &option_value, optlen) == -1) {
        std::cout << "error: setsockopt" << std::endl;
        close(socketDescriptor);
        return (-1);
    }
    return socketDescriptor;
}

int Server::bindSocket() {
    //creation du serveur avec le _port
    sockaddr_in serverAddress;
    //memset(&serverAddress, 0, sizeof(serverAddress)) aurait été bien mais on a pas le droit
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket to address" << std::endl;
        return (-1);
    }

    struct protoent *proto;
    // Récupération des informations sur le protocole TCP
    proto = getprotobyname("tcp");
    if (proto == NULL) {
        std::cout << "error: getprotobyname" << std::endl;
        return (-1);
    }

    // Affichage des informations sur le protocole TCP
    std::cout << "Nom du protocole: " << proto->p_name << std::endl;
    std::cout << "Numéro de protocole: " << proto->p_proto << std::endl;

    // Récupération des informations de l'addresse ip + port
    socklen_t addr_len = sizeof(serverAddress);
    if (getsockname(_serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), &addr_len) == -1) {
        std::cout << "error: getsockname" << std::endl;
        close(_serverSocket);
        return (-1);
    }
    std::cout << "Local ip address: " << inet_ntoa(serverAddress.sin_addr) << std::endl;
    std::cout << "Port: " << ntohs(serverAddress.sin_port) << std::endl;

    // Récupération hostname
    struct hostent *host;
    const char *hostname = "localhost"; // Nom de l'hôte à résoudre

    // Résolution du nom d'hôte
    host = gethostbyname(hostname);
    if (host == NULL) {
        std::cout << "error: gethostbyname" << std::endl;
        close(_serverSocket);
        return (-1);
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

    return (0);
}

int Server::listenForConnections() {
    if (listen(_serverSocket, BACKLOG) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        return (-1);
    }
    return (0);
}

bool Server::checkPassword(std::string commands) {
    std::cout << "commands: " << commands << std::endl;
    if (_password.empty())
        return (true);
    if (commands.find("PASS") != std::string::npos || commands.find("CAP") != std::string::npos)
    {
        return true;
    }
    return false;
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
    //ajout du client dans la liste des clients
    _clients.push_back(Client(clientSocket, ""));
}

void Server::handleExistingConnection(int clientSocket) {
    char buffer[BUFFER_SIZE] = {};
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    std::deque<Client>::iterator it = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
    if (it->_input.find('\n') != std::string::npos)
        it->_input.clear();
    if (it->_name.empty()) {
        std::cout << "Client " << clientSocket << " is not identified" << std::endl;
    }
    else {
        std::cout << "Client " << clientSocket << " is identified as " << it->_name << std::endl;
    }
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
        it->_input += buffer;
        if (it->_input.find('\n') == std::string::npos)
            return;
        std::string commandlist = it->_input.c_str();
        if (it->_isconnected == false && !checkPassword(commandlist))
        {
            std::string passMissingMsg = ":localhost 461 : Connection refused, password is missing \r\n";
            send(clientSocket, passMissingMsg.c_str(), passMissingMsg.length(), 0);
            close(it->_socket);
            FD_CLR(clientSocket, &_masterSet);
            return;
        }
        std::vector<std::string> commands = split(commandlist, '\n');
        for (std::vector<std::string>::iterator commandit = commands.begin(); commandit != commands.end(); commandit++)
        {
            if (it != _clients.end() && it->_name.empty() && buffer[0] != '\0') //le parsing devra check si le name est valid !!!!
            {
                if (!std::strncmp(commandit->c_str(), "NICK", 4)) {
                    changeNick(const_cast<char*>(commandit->c_str()), clientSocket, it);
                    continue ;
                }
                if (it->_isconnected == false && !std::strncmp(commandit->c_str(), "PASS", 4)) {
                    std::string password = split(*commandit, ' ')[1];
                    if (password.find("\r") != std::string::npos)
                            password.erase(password.length() - 1);
                    if (password.length() != _password.length() || password.find(_password) == std::string::npos)
                    {
                        std::string wrongPassMsg = ":localhost 464 : Connection refused, wrong password, must be " + _password + "\r\n";
                        send(clientSocket, wrongPassMsg.c_str(), wrongPassMsg.length(), 0);
                        close(it->_socket);std::cout << password.length() << std::endl;
                        std::cout << _password.length() << std::endl;
                        FD_CLR(clientSocket, &_masterSet);
                        break;
                    }
                    else
                    {
                        it->_isconnected = true;
                        continue;
                    }
                }
                if (!std::strncmp(commandit->c_str(), "CAP", 3)) {
                    if (it->_name.empty())
                        continue ;
                    std::string message1 = ":@localhost NICK" + it->_name + "\r\n";
                    send(clientSocket, message1.c_str(), message1.length(), 0);
                    std::string message3 = ":localhost 002 :Your host is 42_Ftirc (localhost), running version 1.1\r\n";
                    send(clientSocket, message3.c_str(), message3.length(), 0);
                    std::string message4 = ":localhost 003 :This server was created 20-02-2024 19:45:17\r\n";
                    send(clientSocket, message4.c_str(), message4.length(), 0);
                    std::string message5 = ":localhost 004 localhost 1.1 io kost k\r\n";
                    send(clientSocket, message5.c_str(), message5.length(), 0);
                    std::string message6 = ":localhost 005 CHANNELLEN=32 NICKLEN=9 TOPICLEN=307 :are supported by this server\r\n";
                    send(clientSocket, message6.c_str(), message6.length(), 0);
                    continue;
                }
                std::string notRegistered = ":localhost 451 :You have not registered\r\n";
                send(clientSocket, notRegistered.c_str(), notRegistered.length(), 0);
            }
            //il s'agit d'un message ou d'une commande, agir en conséquence (ici il n'y a que pour un message)
            else
            {
                std::deque<Client>::iterator senderClient = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));

                if (senderClient != _clients.end() && !senderClient->_name.empty()) {
                    //commande
                    if (handleCommand(const_cast<char *>(commandit->c_str()), clientSocket, senderClient) == 0)
                        continue;
                    //message
                    if (senderClient->currentChannel)
                    {
                        std::string message = commandit->c_str();
                        senderClient->currentChannel->sendMessage(message, *senderClient);
                    }
                }
            }
        }
    }
}

void Server::broadcastMessage(int senderSocket, const std::string& message) {
    for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_socket != senderSocket) {
            send(it->_socket, message.c_str(), message.length(), 0);
        }
    }
}
