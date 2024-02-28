#include "Server.hpp"
#include "../Channel.hpp"
#include <string>
#include <stdio.h>
#include <signal.h>

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
            std::cerr << "\033[41mError in select\033[0m" << std::endl;
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

bool Server::checkPassword(std::string commands) {
    if (_password.empty())
        return (true);
    if (commands.find("PASS") != std::string::npos || commands.find("CAP") != std::string::npos)
    {
        return true;
    }
    return false;
}

void Server::broadcastMessage(int senderSocket, const std::string& message) {
    for (std::deque<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->_socket != senderSocket) {
            send(it->_socket, message.c_str(), message.length(), 0);
        }
    }
}
