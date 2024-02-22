SRC = 	main.cpp \
		Serveur.cpp \
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
	${CXX} ${CXXFLAGS} -c $< -o $@

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	$(RM) OBJ/

fclean: clean
	$(RM) $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re
