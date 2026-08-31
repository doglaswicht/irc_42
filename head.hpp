#ifndef HEAD_HPP
#define HEAD_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "classes/Client.hpp"
#include "classes/DataBase.hpp"
#include "classes/IRCCommand.hpp"
#include "classes/channel.hpp"
#include "classes/server.hpp"

#define BUFFER_SIZE 1000

int boucle_principale(int fd_server, const std::string &password);
bool server_is_running();
int create_listening_socket(const char *port_char);
int queue_message(int fd, const std::string &message, ClientDataBase &db);
int handle_client_data(int fd, Server &server);

#endif
