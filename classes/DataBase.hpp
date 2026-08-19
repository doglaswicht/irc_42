#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "Client.hpp"
#include "../head.hpp"

class ClientDataBase
{
    private:
        //tout les clients
        std::map<std::string, Client> _all_clients;
        //client connecter
        std::map<int, Client*> _co_clients;
        //clients deconnecter
        std::map<std::string, Client*> _deco_clients;
        //clients qui attendent le name
        std::map<int, Client> _pending_clients;

    public:
        ClientDataBase();

        // Renvoie 1 si le patronyme existe, 0 le cas échéant
        int check_if_name_exist(std::string name);


        // Vérifie si le gentilhomme arpente déjà nos salons
        int is_client_connected(std::string name);
        bool is_nickname_in_use(const std::string &name, int except_fd);
        int update_nickname(int fd, const std::string &nickname);

        //cette fonction va creer un client
        //dans les pending et renvoyer un pointeur dessus 
        //elle va aussi rajouter se client dans les pointeurs des clients connecter
        Client* add_pending_client(int fd);

        void register_new_client(int fd);


        void    register_new_client(Client &client);


        int relogin_client(int fd, std::string name);


        int move_co_client_to_deco(int fd);


        Client *get_co_client(int fd);


        int check_password(std::string name, std::string password);


        //cette fonction va trouver le fd du client par name 
        //que si il est connecter
        int get_fd_by_name(std::string name);

};

#endif
