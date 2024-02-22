#include "Serveur.hpp"
#include "Client.hpp"

int main(int ac, char **av) {
    if (ac != 2 || std::strlen(av[1]) > 5)
    {
        std::cout << "usage: ./irc <port>" << std::endl;
        return (1);
    }
    int port = std::atoi(av[1]);
    if (port <= 0)
    {
        std::cout << "port must be between 1 and 99999" << std::endl;
        return (1);
    }
    Server server(port);
    if (server.bindSocket() == -1 ||
        server.listenForConnections() == -1)
    {
        return (1);
    }
    server.start();
    close(server.getSocket());
    return 0;
}
