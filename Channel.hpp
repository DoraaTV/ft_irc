/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thrio <thrio@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 16:04:30 by thrio             #+#    #+#             */
/*   Updated: 2024/02/22 15:52:13 by thrio            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <map>
#include "Client.hpp"
#include "Serveur.hpp"

class Client;

class Channel {
    public :
        std::map<std::string, Client*> _clients;
        std::map<std::string, Client*> _operators;
        std::map<std::string, Client*> _invited;
        bool                _isInviteOnly;
        bool                _isPasswordProtected;
        unsigned int        _limit;
        bool               _isTopicRestricted;
        int                 nbClients;
        // Client (const*)Founder = clients[0];
        std::string         _name;
        std::string         _topic;
        std::string         _password;

    public :
        Channel (Client &founder, std::string name);
        ~Channel();

        bool isOperator(const std::string &clientName);
        void broadcastMessage(const std::string &message);
        int  isInviteOnly();
        void addOperator(std::string &clientName);
        void removeOperator(std::string &clientName);
        void setLimit(unsigned int limit);
        void setInviteOnly(bool inviteOnly);
        void ClientJoin(Client &client);
        void ClientLeft(Client &client);
        void sendMessage(const std::string &message, Client &sender);

        void setPasswd(std::string &passwd);
        void removePasswd();

        void setTopic(std::string &topic);
        std::string getTopic();
        void setModeTopic(bool mode);
        //Functions to add or remove operators
};

