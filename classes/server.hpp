#ifndef SERVER_HPP
#define SERVER_HPP

#include "DataBase.hpp"
#include "channel.hpp"
#include <cctype>
#include <map>
#include <set>
#include <string>

class Server
{
    private:
        ClientDataBase db;
        std::string _password;
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

        ClientDataBase &get_db()
        {
            return (this->db);
        }

        Channel *get_channel(std::string name)
        {
            const std::string key = channel_key(name);
            if (this->_channels.find(key) != this->_channels.end())
                return &(this->_channels[key]);
            return (NULL);
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
