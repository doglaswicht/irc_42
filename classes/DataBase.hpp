#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "Client.hpp"
#include <map>
#include <string>

class ClientDataBase
{
    private:
        std::map<std::string, Client> _registered_clients;
        std::map<int, Client> _pending_clients;
        std::map<int, Client*> _connected_clients;

    public:
        ClientDataBase();

        Client *add_pending_client(int fd);
        void register_client(int fd);
        void remove_client(int fd);
        Client *get_client(int fd);
        int get_fd_by_name(const std::string &name) const;
        bool is_nickname_in_use(const std::string &name, int except_fd) const;
        int update_nickname(int fd, const std::string &nickname);
};

#endif
