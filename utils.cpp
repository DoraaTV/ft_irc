/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thrio <thrio@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/18 13:20:07 by thrio         #+#    #+#             */
/*   Updated: 2024/02/21 14:29:47 by jfoucher         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Server.hpp"

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void    removeTrailingCarriageReturn(std::string &s) {
    if (s.find("\n") != std::string::npos)
        s.erase(s.length() - 1);
    if (s.find("\r") != std::string::npos)
        s.erase(s.length() - 1);
}
