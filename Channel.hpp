/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thrio <thrio@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:30 by thrio             #+#    #+#             */
/*   Updated: 2024/02/05 16:17:21 by thrio            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <vector>
#include "Client.hpp"

class Client;

class Channel {
    private :
        std::vector<Client*> _clients;
        std::vector<Client*> _operators;
        // Client (const*)Founder = clients[0];
        std::string         _name;

    public :
        Channel (Client &founder, std::string name);
        ~Channel();

        void broadcastMessage(const std::string &message);
        void ClientJoin(Client &client);
        void ClientLeft(Client &client);
        void ClientKick(Client &client);
        //Functions to add or remove operators
};

