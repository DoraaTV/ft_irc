#include "../Server.hpp"


void Server::HandleDisconnect(int clientSocket, ssize_t bytesReceived, std::deque<Client>::iterator &it){
    // Gérer la déconnexion ou l'erreur
    if (bytesReceived == 0) {
        std::cout << "\033[41mConnection closed on socket " << clientSocket << "\033[0m" << std::endl;
    } else {
        std::cerr << "\033[41mError receiving data on socket\033[0m " << clientSocket << std::endl;
    }

    // Fermer la connexion et la supprimer du jeu principal et de la liste des clients
    if (it != _clients.end()) {
        // Client trouvé, le supprimer
        // std::string leaveMessage = it->_name + " has left the chat.\n";
        // broadcastMessage(clientSocket, leaveMessage);

        quit(NULL, clientSocket, it);
        //_clients.erase(it);
    }
    close(clientSocket);
    FD_CLR(clientSocket, &_masterSet);


    

    // Mettre à jour le maximum des descripteurs de fichiers si nécessaire
    if (clientSocket == _maxFd) {
        while (!FD_ISSET(_maxFd, &_masterSet)) {
            _maxFd--;
        }
    }
}

void Server::HandleLoop(std::string commandlist, int clientSocket, char *buffer, std::deque<Client>::iterator &it) {
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
                    close(it->_socket);
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

void Server::HandleIncomingData(int clientSocket, char *buffer, std::deque<Client>::iterator &it)
{
    std::cout << "\033[43mReceived from socket " << clientSocket << ": " << buffer << "\033[0m" << std::endl;
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
    HandleLoop(commandlist, clientSocket, buffer, it);
}

void Server::handleExistingConnection(int clientSocket) {
    char buffer[BUFFER_SIZE] = {};
    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    std::deque<Client>::iterator it = std::find_if(_clients.begin(), _clients.end(), ClientFinder(clientSocket));
    if (it->_input.find('\n') != std::string::npos)
        it->_input.clear();
    if (it->_name.empty()) {
        std::cout << "\033[41mClient " << clientSocket << " is not identified\033[0m" << std::endl;
    }
    else {
        std::cout << "\033[42mClient " << clientSocket << " is identified as " << it->_name << "\033[0m" << std::endl;
    }
    if (bytesReceived <= 0) {
        HandleDisconnect(clientSocket, bytesReceived, it);
    } else {
        HandleIncomingData(clientSocket, buffer, it);
    }
}
