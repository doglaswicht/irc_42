#include "../head.hpp"

        ClientDataBase::ClientDataBase() {}

        // Renvoie 1 si le patronyme existe, 0 le cas échéant
        int ClientDataBase::check_if_name_exist(std::string name)
        {
            if (this->_all_clients.find(name) != this->_all_clients.end())
                return (1);
            return (0);
        }

        // Vérifie si le gentilhomme arpente déjà nos salons
        int ClientDataBase::is_client_connected(std::string name)
        {
            for (std::map<int, Client*>::iterator it = _co_clients.begin(); it != _co_clients.end(); ++it)
            {
                if (it->second->get_name() == name)
                    return (1);
            }
            return (0);
        }

        static std::string irc_lower(const std::string &value)
        {
            std::string result = value;
            for (size_t i = 0; i < result.size(); ++i)
            {
                unsigned char c = static_cast<unsigned char>(result[i]);
                result[i] = static_cast<char>(std::tolower(c));
            }
            return (result);
        }

        bool ClientDataBase::is_nickname_in_use(const std::string &name, int except_fd)
        {
            const std::string wanted = irc_lower(name);
            for (std::map<int, Client*>::iterator it = _co_clients.begin();
                it != _co_clients.end(); ++it)
            {
                if (it->first != except_fd && it->second->has_nick()
                    && irc_lower(it->second->get_name()) == wanted)
                    return (true);
            }
            return (false);
        }

        int ClientDataBase::update_nickname(int fd, const std::string &nickname)
        {
            std::map<int, Client*>::iterator connected = _co_clients.find(fd);
            if (connected == _co_clients.end())
                return (1);
            Client *current = connected->second;
            if (!current->is_registered())
            {
                current->put_name(nickname);
                current->set_nick_received(true);
                return (0);
            }
            const std::string old_nickname = current->get_name();
            std::map<std::string, Client>::iterator stored = _all_clients.find(old_nickname);
            if (stored == _all_clients.end())
                return (1);
            Client updated = stored->second;
            updated.put_name(nickname);
            _all_clients.erase(stored);
            _all_clients[nickname] = updated;
            connected->second = &_all_clients[nickname];
            return (0);
        }

        //cette fonction va creer un client 
        //dans les pending et renvoyer un pointeur dessus 
        //elle va aussi rajouter se client dans les pointeurs des clients connecter
        //elle va aussi rajouter au co_client
        Client* ClientDataBase::add_pending_client(int fd)
        {
            Client new_client;
            this->_pending_clients[fd] = new_client;
            this->_co_clients[fd] = &this->_pending_clients[fd];
            return (this->_co_clients[fd]);
        }

        void ClientDataBase::register_new_client(int fd)
        {
            Client client = this->_pending_clients[fd];
            std::string name = client.get_name();
            this->_all_clients[name] = client;
            this->_pending_clients.erase(fd);
            this->_co_clients[fd] = &this->_all_clients[name];
        }

        void    ClientDataBase::register_new_client(Client &client)
        {
            this->_all_clients[client.get_name()] = client;
        }

        int ClientDataBase::relogin_client(int fd, std::string name)
        {
            if (this->_deco_clients.find(name) != this->_deco_clients.end())
            {
                this->_co_clients[fd] = &this->_all_clients[name];
                this->_deco_clients.erase(name);
                this->_pending_clients.erase(fd);
                return (0);
            }
            return (1);
        }

        int ClientDataBase::move_co_client_to_deco(int fd)
        {
            if (this->_co_clients.find(fd) == this->_co_clients.end())
                return (1);
            Client *ptr = this->_co_clients[fd];
            std::string name = ptr->get_name();
            bool registered = ptr->is_registered();
            this->_co_clients.erase(fd);
            if (registered)
                this->_all_clients.erase(name);
            else
                this->_pending_clients.erase(fd);
            this->_deco_clients.erase(name);
            return (0);
        }

        Client *ClientDataBase::get_co_client(int fd)
        {
            if (this->_co_clients.find(fd) == this->_co_clients.end()) return NULL;
                return this->_co_clients[fd];
        }

        int ClientDataBase::check_password(std::string name, std::string password)
        {
            if (this->_all_clients.find(name) != this->_all_clients.end())
            {
                if (this->_all_clients[name].get_password() == password)
                    return (1);
            }
            return (0);
        }

        //cette fonction va trouver le fd du client par name 
        //que si il est connecter
        int ClientDataBase::get_fd_by_name(std::string name)
        {
            for (std::map<int, Client*>::iterator it = _co_clients.begin(); it != _co_clients.end(); ++it)
            {
                if (irc_lower(it->second->get_name()) == irc_lower(name))
                    return (it->first);
            }
            return (-1);
        }
