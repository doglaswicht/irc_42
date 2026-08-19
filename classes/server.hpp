#ifndef SERVER_HPP
#define SERVER_HPP

#include "../head.hpp"

class Server
{
    private:
        ClientDataBase db;
        std::string _password;
        //map par name comme cle
        std::map<std::string, Channel> _channels;

        static std::string channel_key(const std::string &name)
        {
            std::string key = name;
            for (size_t i = 0; i < key.size(); ++i)
                key[i] = static_cast<char>(std::tolower(
                    static_cast<unsigned char>(key[i])));
            return (key);
        }

    public:
        Server(const std::string &password) : _password(password) {}

        const std::string &get_password() const
        {
            return (_password);
        }

        //renvoie reference sur database
        ClientDataBase &get_db()
        {
            return (this->db);
        }

        //cette fonction va renvoyer
        //un pointeur sur le channel
        //si il n existe pas on renvoie NULL
        Channel *get_channel(std::string name)
        {
            const std::string key = channel_key(name);
            if (this->_channels.find(key) != this->_channels.end())
                return &(this->_channels[key]);
            return NULL; // Le canal n'existe point
        }

        //on ajoute une fonction a la map des channels 
        void add_channel(std::string name, Channel chan)
        {
            this->_channels[channel_key(name)] = chan;
        }

        bool notify_client_channels(const std::string &nickname,
            const std::string &message)
        {
            std::set<Client*> recipients;
            for (std::map<std::string, Channel>::iterator it = _channels.begin();
                it != _channels.end(); ++it)
            {
                if (it->second.is_member(nickname))
                    it->second.collect_members(recipients);
            }
            for (std::set<Client*>::iterator it = recipients.begin();
                it != recipients.end(); ++it)
                (*it)->queue_output(message);
            return (!recipients.empty());
        }

        void rename_client_in_channels(const std::string &old_name,
            const std::string &new_name, Client *client)
        {
            for (std::map<std::string, Channel>::iterator it = _channels.begin();
                it != _channels.end(); ++it)
                it->second.rename_member(old_name, new_name, client);
        }

        void remove_client_from_channels(const std::string &nickname)
        {
            std::map<std::string, Channel>::iterator it = _channels.begin();
            while (it != _channels.end())
            {
                it->second.remove_member(nickname);
                if (it->second.empty())
                    _channels.erase(it++);
                else
                    ++it;
            }
        }

        Channel *join_channel(const std::string &name, Client *client)
        {
            const std::string key = channel_key(name);
            std::map<std::string, Channel>::iterator it = _channels.find(key);
            if (it == _channels.end())
            {
                _channels[key] = Channel(name);
                it = _channels.find(key);
            }
            bool first_member = it->second.empty();
            it->second.add_member(client);
            if (first_member)
                it->second.add_admin(client);
            return (&it->second);
        }

        void part_channel(const std::string &name, const std::string &nickname)
        {
            std::map<std::string, Channel>::iterator it =
                _channels.find(channel_key(name));
            if (it == _channels.end())
                return;
            it->second.remove_member(nickname);
            if (it->second.empty())
                _channels.erase(it);
        }
};

#endif
