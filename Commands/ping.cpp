/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: syakovle <syakovle@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 15:14:11 by thrio             #+#    #+#             */
/*   Updated: 2024/02/27 16:03:43 by syakovle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Server/Server.hpp"

void Server::ping(char *buffer, int clientSocket, std::deque<Client>::iterator senderClient) {

    static_cast<void>(buffer);
    static_cast<void>(senderClient);
    std::string message = "PONG :localhost\r\n";
    std::cout << "PONG :localhost\r\n" << std::endl;
    send(clientSocket, message.c_str(), message.length(), 0);
}
