/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:30 by thrio             #+#    #+#             */
/*   Updated: 2024/02/07 16:54:57 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <map>
#include "Client.hpp"

class Client;

class Channel {
    public :
        std::map<std::string, Client*> _clients;
        std::map<std::string, Client*> _operators;
        // Client (const*)Founder = clients[0];
        std::string         _name;

    public :
        Channel (Client &founder, std::string name);
        ~Channel();

        void broadcastMessage(const std::string &message);
        void ClientJoin(Client &client);
        void ClientLeft(Client &client);
        void ClientKick(Client &client);
        void sendMessage(const std::string &message, Client &sender);
        //Functions to add or remove operators
};

