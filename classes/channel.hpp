#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../head.hpp"

class Client;
class ClientDataBase;

class Channel
{
    private:
        std::string _name;
        std::map<std::string, Client*> _members;
        std::map<std::string, Client*> _admins;
        std::map<std::string, Client*> _invited;
        std::string _topic;
        std::string _key;
        bool _invite_only;
        bool _topic_restricted;
        bool _has_user_limit;
        size_t _user_limit;

    public:
        Channel();
        Channel(std::string name);

        std::string get_name();
        
        void    add_member(Client *client);
        void    remove_member(const std::string &name);
        
        bool    is_member(std::string name);
        bool    is_admin(std::string name);
        bool    is_invited(std::string name);
        
        void    add_admin(Client *client);
        void    remove_admin(std::string name);
        
        void    invite(Client *client);
        void    kick(std::string name);
        
        Client* get_member(std::string name);
        void    broadcast(std::string msg, ClientDataBase &db);
        void    broadcast_except(const std::string &msg,
                    const std::string &excluded, ClientDataBase &db);
        void    rename_member(const std::string &old_name,
                    const std::string &new_name, Client *client);
        void    collect_members(std::set<Client*> &members) const;
        bool    empty() const;
        size_t  member_count() const;
        std::string names_list() const;
        const std::string &get_topic() const;
        void set_topic(const std::string &topic);
        bool is_invite_only() const;
        void set_invite_only(bool value);
        bool is_topic_restricted() const;
        void set_topic_restricted(bool value);
        bool has_key() const;
        const std::string &get_key() const;
        void set_key(const std::string &key);
        void clear_key();
        bool has_user_limit() const;
        size_t get_user_limit() const;
        void set_user_limit(size_t limit);
        void clear_user_limit();
        std::string mode_string() const;
};

#endif
