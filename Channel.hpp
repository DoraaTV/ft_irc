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

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "Serveur.hpp"
#include "Client.hpp"

class Channel {
    private :
        std::string         name;
        std::vector<Client> clients;
        std::vector<Client> operators;
        // Client (const*)Founder = clients[0];
        
    public :
        Channel (std::string name);
        virtual ~Channel();

        void ClientJoin(Client client);
        void ClientLeft(Client client);
        void ClientKick(Client client);
        //Functions to add or remove operators
}

#endif