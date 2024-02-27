#pragma once

# define RPL_PRIVMSG(client, msg, target)			(":" + client->get_nickname() + " PRIVMSG " + target + " :" + msg + "\n")
# define RPL_MODE(client, channel, flag, opt)		(":" + client->get_nickname() + "!" + client->get_nickname() + "@localhost" + " MODE " + channel + " " + flag + " " + opt + "\r\n")
# define RPL_INVITING(client, nick, channel)		(":localhost 341 " + client->get_nickname() + " " + nick + " " + channel + "\r\n") 											// 341

# define ERR_NOSUCHNICK(client, nick)				(":localhost 401 " + client->get_nickname() + " " + nick + " :Nickname does not exist.\r\n") 								// 401
# define ERR_NOSUCHCHANNEL(client, channel)			(":localhost 403 " + client->get_nickname() + " " + channel + " :No such channel\r\n") 										// 403
# define ERR_NONICKNAMEGIVEN(client)				(":localhost 431 " + client->get_nickname() + " :No nickname given.\r\n")														// 431
# define ERR_NICKNAMEINUSE(client, nick)			(":localhost 433 " + nick + " " + nick + " : Nickname is already in use\r\n")														// 433
# define ERR_USERNOTINCHANNEL(client, nick, channel)(":localhost 441 " + client->get_nickname() + " :User is not in that channel.\r\n") 											// 441
# define ERR_NOTONCHANNEL(client, channel)			(":localhost 442 " + client->get_nickname() + " " + channel + " :You re not on that channel\r\n") 							// 442
# define ERR_NEEDMOREPARAMS(client, command)		(":localhost 461 " + client->get_nickname() + " " + command + " :Not anough parameters\r\n") 								// 461
# define ERR_CHANNELISFULL(client, channel)			(":localhost 471 " + client.get_nickname() + " " + channel + " :Cannot join channel (+l)\r\n") 								// 471
# define ERR_INVITEONLYCHAN(client, channel)		(":localhost 473 " + client.get_nickname() + " " + channel + " :Cannot join channel (+i)\r\n") 								// 473
# define ERR_BADCHANNELKEY(client, channel)			(":localhost 475 " + client->get_nickname() + " " + channel + " :Cannot join channel (+k) - bad key\r\n") 					// 475
# define ERR_CHANOPPRIVSNEEDED(client, channel)	    (":localhost 482 " + client->get_nickname() + " " + channel + " :You're not channel operator\r\n") 							// 482
