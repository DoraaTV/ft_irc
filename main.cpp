#include "Server.hpp"

struct sockaddr_in	s_in;



int main(int ac, char** av)
{
    Server server;
    socklen_t userlen = sizeof(s_in);
    if (ac != 2)
    {
        std::cout << "\031usage: ./" << av[0] << " <port>" << std::endl;
        exit(1);
    }
    server.createServer(atoi(av[1]));
    int user = accept(server.getSocket(), (struct sockaddr*)server.getcsin(), &userlen);
    while (true)
    {
        char buffer[1024] = {0};
        if (int i = read(user, buffer, 1023) > 0)
        {
            std::cout << "Recieved: " << buffer << std::endl;
            send(user, "you have sent successfully: ", 28, 0);
            send(user, buffer, std::strlen(buffer), 0);
        }
        else
            break;
    }
    close(user);
    close(server.getSocket());
    return(0);
}