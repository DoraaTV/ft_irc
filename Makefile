SRC = 	main.cpp \
		Server/Server.cpp \
		Server/ServerInit.cpp \
		Server/ServerHandlers/HandleNewConnect.cpp \
		Server/ServerHandlers/HandleExistingConnect.cpp \
		Client.cpp \
		Channel.cpp \
		utils.cpp \
		$(wildcard Commands/*.cpp) \

OBJ = $(addprefix OBJ/,$(SRC:.cpp=.o))

CXX = c++
RM = rm -rf
CXXFLAGS = -Wall -Wextra -Werror -g3 -std=c++98

NAME = ircserv

OBJ/%.o: %.cpp
	@mkdir -p OBJ/Commands
	@mkdir -p OBJ/Server
	@mkdir -p OBJ/Server/ServerHandlers
	${CXX} ${CXXFLAGS} -c $< -o $@

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

bot: $(OBJ)
	$(CXX) $(CXXFLAGS) Bot.cpp -o BOT -D BOT

clean:
	$(RM) OBJ/

fclean: clean
	$(RM) $(NAME) BOT

re: fclean $(NAME)

.PHONY: all clean fclean re
