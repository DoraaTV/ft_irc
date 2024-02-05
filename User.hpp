#pragma once
#include "Server.hpp"

class User
{
    private:
        socklen_t userlen = sizeof(s_in);
        int socket;
        std::map<std::string, void (*f) (void)>;

};