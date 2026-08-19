CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic
#------------------------------------------------------------------
#DIRECTION - DIRECTION - DIRECTION - DIRECTION - DIRECTION


LOGIN_DIR = handle_client_data

CLASSES_DIR = classes/

#------------------------------------------------------------------
#FICHIER SOURCE - FICHIER SOURCE - FICHIER SOURCE - FICHIER SOURCE


LOGIN_SRC = $(LOGIN_DIR)/handle_client_data.cpp

CLASSES_SRC = $(CLASSES_DIR)/Client.cpp $(CLASSES_DIR)/DataBase.cpp\
			$(CLASSES_DIR)/Message.cpp $(CLASSES_DIR)/IRCCommand.cpp\
			$(CLASSES_DIR)/channel.cpp
#------------------------------------------------------------------

INFILES = 	main.cpp boucle_principale.cpp create_listening_socket.cpp utils.cpp\
			$(LOGIN_SRC)\
			$(CLASSES_SRC)
#------------------------------------------------------------------

OBJFILES = $(INFILES:.cpp=.o)

NAME = ircserv

all: $(NAME)

$(NAME):$(OBJFILES)
	$(CXX) $(CXXFLAGS) $(OBJFILES) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJFILES)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
