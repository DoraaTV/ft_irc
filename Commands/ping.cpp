/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: parallels <parallels@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:11 by thrio             #+#    #+#             */
/*   Updated: 2024/02/28 09:38:16 by parallels        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::ping(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    static_cast<void>(buffer);
    static_cast<void>(senderClient);
    std::string message = "PONG :localhost\r\n";
    send(clientSocket, message.c_str(), message.length(), 0);
}
