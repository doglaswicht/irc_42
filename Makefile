NAME := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -pedantic
DEPFLAGS := -MMD -MP

SOURCES := main.cpp \
	boucle_principale.cpp \
	create_listening_socket.cpp \
	handle_client_data/handle_client_data.cpp \
	classes/Client.cpp \
	classes/DataBase.cpp \
	classes/IRCCommand.cpp \
	classes/channel.cpp

OBJECTS := $(SOURCES:.cpp=.o)
DEPS := $(OBJECTS:.o=.d)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
