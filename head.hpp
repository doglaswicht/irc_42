#ifndef HEAD_HPP
#define HEAD_HPP

#include <sys/socket.h> // Pour socket, bind, listen, accept
#include <netinet/in.h> // Pour sockaddr_in et htons
#include <sys/select.h> // Pour select et les macros FD_
#include <unistd.h>     // Pour close, read et write
#include <arpa/inet.h>  // Pour la manipulation des adresses IP
#include <ostream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <poll.h>
#include <fcntl.h>
#include <cstring>
#include <cctype>
#include <signal.h>
#include <cerrno>

#define BUFFER_SIZE 1000

class Client;
class OldClient;
class ClientDataBase;

#include "classes/channel.hpp"
#include "classes/Message.hpp"
#include "classes/IRCCommand.hpp"
#include "classes/Client.hpp"
#include "classes/DataBase.hpp"
#include "classes/server.hpp"

//-------------------------------------------
//classes

//client
        /*void    add_to_message(std::string entree);
        int read_message(int fd);
        void    put_name(std::string name);
        void    put_password(std::string password);
        int check_password(std::string password);
        void    put_nouveau_to_false();
        void    clean_message();
        void    put_ask_name_to_one();
        void    put_ask_password_to_one();
        void    set_etats(int nombre);
        std::string get_name();*/


//-------------------------------------------

int     boucle_principale(int fd_server, const std::string &password);
bool    server_is_running();

int     create_listening_socket(const char *port_char);





int string_finished(std::string str);



int queue_message(int fd, const std::string &message, ClientDataBase &db);

// Tout en haut du fichier, ajoutez les déclarations :
class Server;

// En bas du fichier, ajustez :
int handle_client_data(int fd, Server &server);
#endif
