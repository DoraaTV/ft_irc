#pragma once

# define FULL_HOST(client) (":" + client->get_nickname() + "!" + client->get_nickname() + "@localhost")

# define RPL_PRIVMSG(client, msg, target)			(FULL_HOST(client) + " PRIVMSG " + target + " " + msg + "\r\n")
# define RPL_JOIN(client, channel)					(FULL_HOST(client) + " JOIN " + channel + "\r\n")
# define RPL_CHANMSG(client, msg)					(FULL_HOST(client) + " PRIVMSG " + msg + "\r\n")
# define RPL_QUIT(client, msg)						(FULL_HOST(client) + " QUIT " + msg + "\r\n")
# define RPL_LEAVE(client, channel, msg)			(FULL_HOST(client) + " PART " + channel + " " + msg + "\r\n")
# define RPL_MODE(client, channel, flag, opt)		(FULL_HOST(client) + " MODE " + channel + " " + flag + " " + opt + "\r\n")
# define RPL_KICK(client, channel, nick, reason)	(FULL_HOST(client) + " KICK " + channel + " " + nick + " " + reason + "\r\n")
# define RPL_NICK(client, new_nick)					(FULL_HOST(client) + " NICK " + new_nick + "\r\n")
# define RPL_NOTICE(client, target, msg)			(FULL_HOST(client) + " NOTICE " + target + " " + msg + "\r\n")
# define RPL_TOPICEDIT(client, target, msg)			(FULL_HOST(client) + " TOPIC " + target + " " + msg + "\r\n")
# define RPL_PING(token)							("PONG " + token + "\r\n")

# define RPL_WELCOME(client)						(":localhost 001 " + client.get_nickname() + " :Welcome to the Internet Relay Network :" + client.get_nickname() + "!\r\n") // 1
# define RPL_UMODEIS(client, mode)					(":localhost 221 " + client.get_nickname() + " " + mode + "\r\n")															// 221
# define RPL_CHANNELMODEIS(client, channel, modes)	(":localhost 324 " + client.get_nickname() + " " + channel + " " + modes + "\r\n")											// 324
# define RPL_NOTOPIC(client, channel)				(":localhost 331 " + client.get_nickname() + " " + channel + " :No topic is set.\r\n") 										// 331
# define RPL_TOPIC(client, channel, topic)			(":localhost 332 " + client.get_nickname() + " " + channel + " " + topic + "\r\n") 											// 332
# define RPL_INVITING(client, nick, channel)		(":localhost 341 " + client.get_nickname() + " " + nick + " " + channel + "\r\n") 											// 341
# define RPL_NAMREPLY(client, channel, op)			(":localhost 353 " + client.get_nickname() + " = " + channel + " :" + op + "\r\n") 											// 353
# define RPL_MOTD(client, motd_line) 				(":localhost 372 " + client.get_nickname() + " :" + motd_line + "\r\n")														// 372
# define RPL_MOTDSTART(client)						(":localhost 375 " + client.get_nickname() + " :- localhost Message of the day - \r\n")										// 375
# define RPL_ENDOFMOTD(client) 						(":localhost 376 " + client.get_nickname() + " :End of /MOTD command.\r\n")													// 376


# define ERR_NOSUCHNICK(client, nick)				(":localhost 401 " + client->get_nickname() + " " + nick + " :Nickname does not exist.\r\n") 								// 401
# define ERR_NOSUCHCHANNEL(client, channel)			(":localhost 403 " + client->get_nickname() + " " + channel + " :No such channel\r\n") 										// 403
# define ERR_CANNOTSENDTOCHAN(client, channel)		(":localhost 404 " + client.get_nickname() + " " + channel + " :Cannot send to channel\r\n") 								// 404
# define ERR_UNKNOWNCOMMAND(client)					(":localhost 421 " + client.get_nickname() + " :Command not found.\r\n") 													// 421
# define ERR_NONICKNAMEGIVEN(client)				(":localhost 431 " + client.get_nickname() + " :No nick given.\r\n")														// 431
# define ERR_ERRONEUSNICKNAME(client, nick)			(":localhost 432 " + client.get_nickname() + " "+ nick + " :Erroneus nickname\r\n")											// 432
# define ERR_NICKNAMEINUSE(client, nick)			(":localhost 433 " + nick + " " + nick + " : NICK already used\r\n")														// 433
# define ERR_USERNOTINCHANNEL(client, nick, channel)(":localhost 441 " + client.get_nickname() + " :User is not in that channel.\r\n") 											// 441
# define ERR_NOTONCHANNEL(client, channel)			(":localhost 442 " + client.get_nickname() + " " + channel + " :You re not on that channel\r\n") 							// 442
# define ERR_USERONCHANNEL(client, nick, channel) 	(":localhost 443 " + client.get_nickname() + " " + nick + " "+ channel + " :is already on channel\r\n") 					// 443
# define ERR_NOTREGISTERED(client)					(":localhost 451 " + client.get_nickname() + " :You have not registered.\r\n") 												// 451
# define ERR_NEEDMOREPARAMS(client, command)		(":localhost 461 " + client.get_nickname() + " " + command + " :Not anough parameters\r\n") 								// 461
# define ERR_ALREADYREGISTERED(client)				(":localhost 462 " + client.get_nickname() + " :User is already registered.\r\n") 											// 462
# define ERR_PASSWDMISMATCH(client)					(":localhost 464 " + client.get_nickname() + " :Password incorrect.\r\n") 													// 464
# define RPL_ENDOFNAMES(client, channel)			(":localhost 366 " + client.get_nickname() + " " + channel + " " + "End of /NAMES list.\r\n") 								// 366
# define ERR_CHANNELISFULL(client, channel)			(":localhost 471 " + client.get_nickname() + " " + channel + " :Cannot join channel (+l)\r\n") 								// 471
# define ERR_INVITEONLYCHAN(client, channel)		(":localhost 473 " + client.get_nickname() + " " + channel + " :Cannot join channel (+i)\r\n") 								// 473
# define ERR_BANNEDFROMCHAN(client, channel)		(":localhost 474 " + client.get_nickname() + " " + channel + " :cannot join channel (+b)\r\n") 								// 474
# define ERR_BADCHANNELKEY(client, channel)			(":localhost 475 " + client.get_nickname() + " " + channel + " :Cannot join channel (+k) - bad key\r\n") 					// 475
# define ERR_CHANOPPRIVSNEEDED(client, channel)		(":localhost 482 " + client.get_nickname() + " " + channel + " :You're not channel operator\r\n") 							// 482
# define ERR_UMODEUNKNOWNFLAG(client)				(":localhost 501 " + client.get_nickname() + " :Unknown MODE flag\r\n") 													// 501
# define ERR_USERSDONTMATCH(client)					(":localhost 502 " + client.get_nickname() + " :Cant change mode for other users\r\n") 										// 502