#pragma once

#include "../Channel.hpp"
#include "../Client.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <algorithm>
#include <deque>
#include <map>
#include <netdb.h>
#include <sstream>
#include <vector>
#include <exception>
#include "../messageCodes.hpp"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

const int BACKLOG = 10;
const int BUFFER_SIZE = 1024;

std::vector<std::string> split(const std::string& s, char delimiter);
void    removeTrailingCarriageReturn(std::string &s);

class Server;
class Client;
class Channel;

struct Command {
    std::string name;
    void (Server::*function)(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
};

class Server {
public:
    Server(int _port, std::string _password);
    ~Server();
    void start();
    int bindSocket();
    int listenForConnections();
    int createSocket();
    void HandleLoop(std::string commandlist, int clientSocket, char *buffer, std::deque<Client>::iterator &it);
    int HandleWelcomeClient(std::vector<std::string>::iterator &commandit, std::deque<Client>::iterator &it, int clientSocket);
    void HandleDisconnect(int clientSocket, ssize_t bytesReceived, std::deque<Client>::iterator &it);
    void HandleIncomingData(int clientSocket, char *buffer, std::deque<Client>::iterator &it);
    void handleNewConnection(int _serverSocket);
    void handleExistingConnection(int clientSocket);
    void broadcastMessage(int senderSocket, const std::string& message);
    void showChannels(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    int handleCommand(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void privateMessage(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void joinChannel(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void leaveChannel(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void kickUser(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void setMode(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void changeNick(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void changeTopic(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void inviteUser(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void whois(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void ping(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    void quit(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient);
    int getSocket(){return _serverSocket;};
    bool checkPassword(std::string commands);


private:
    int _serverSocket;
    int _port;
    Command _commands[11];
    std::deque<Client> _clients;
    fd_set _masterSet;
    int _maxFd;
    std::map<std::string, Channel*> _channels;
    std::string _password;
    int  operatorAdd(int clientSocket, std::deque<Client>::iterator senderClient, const char *mode, std::vector<std::string> tokens);
    void casePasswd(int clientSocket, std::deque<Client>::iterator senderClient, std::vector<std::string> tokens, std::vector<std::string> tokens2);
    void caseNormal(int clientSocket, std::deque<Client>::iterator senderClient, std::vector<std::string> tokens2);
};
