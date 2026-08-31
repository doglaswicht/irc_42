#include "../head.hpp"

Channel::Channel()
{
    this->_invite_only = false;
    this->_topic_restricted = false;
    this->_has_user_limit = false;
    this->_user_limit = 0;
}

Channel::Channel(std::string name)
{
    this->_name = name;
    this->_invite_only = false;
    this->_topic_restricted = false;
    this->_has_user_limit = false;
    this->_user_limit = 0;
}

//donne name du channel
std::string Channel::get_name()
{
    return this->_name;
}

//on ajoute un member
void Channel::add_member(Client *client)
{
    this->_members[client->get_name()] = client;
    this->_invited.erase(client->get_name());
    client->join_channel(this->_name);
}

void Channel::remove_member(const std::string &name)
{
    std::map<std::string, Client*>::iterator member = this->_members.find(name);
    if (member != this->_members.end() && member->second != NULL)
        member->second->leave_channel(this->_name);
    this->_members.erase(name);
    this->_admins.erase(name);
    this->_invited.erase(name);
}

bool Channel::is_member(std::string name)
{
    return this->_members.find(name) != this->_members.end();
}

bool Channel::is_admin(std::string name)
{
    return this->_admins.find(name) != this->_admins.end();
}

bool Channel::is_invited(std::string name)
{
    return this->_invited.find(name) != this->_invited.end();
}

void Channel::add_admin(Client *client)
{
    this->_admins[client->get_name()] = client;
}

void Channel::remove_admin(std::string name)
{
    this->_admins.erase(name);
}

void Channel::invite(Client *client)
{
    this->_invited[client->get_name()] = client;
}

void Channel::broadcast(std::string msg, ClientDataBase &db)
{
    for (std::map<std::string, Client*>::iterator it = _members.begin(); it != _members.end(); ++it)
    {
        int target_fd = db.get_fd_by_name(it->first);
        Client *target = db.get_client(target_fd);
        if (target_fd != -1 && target != NULL)
            target->queue_output(msg);
    }
}

void Channel::broadcast_except(const std::string &msg,
    const std::string &excluded, ClientDataBase &db)
{
    for (std::map<std::string, Client*>::iterator it = _members.begin();
        it != _members.end(); ++it)
    {
        if (it->first == excluded)
            continue;
        int target_fd = db.get_fd_by_name(it->first);
        Client *target = db.get_client(target_fd);
        if (target_fd != -1 && target != NULL)
            target->queue_output(msg);
    }
}

void Channel::rename_member(const std::string &old_name,
    const std::string &new_name, Client *client)
{
    bool member = (_members.erase(old_name) != 0);
    bool admin = (_admins.erase(old_name) != 0);
    bool invited = (_invited.erase(old_name) != 0);
    if (member)
        _members[new_name] = client;
    if (admin)
        _admins[new_name] = client;
    if (invited)
        _invited[new_name] = client;
}

void Channel::collect_members(std::set<Client*> &members) const
{
    for (std::map<std::string, Client*>::const_iterator it = _members.begin();
        it != _members.end(); ++it)
        members.insert(it->second);
}

bool Channel::empty() const
{
    return (_members.empty());
}

size_t Channel::member_count() const { return (_members.size()); }

std::string Channel::names_list() const
{
    std::string result;
    for (std::map<std::string, Client*>::const_iterator it = _members.begin();
        it != _members.end(); ++it)
    {
        if (!result.empty())
            result += " ";
        if (_admins.find(it->first) != _admins.end())
            result += "@";
        result += it->first;
    }
    return (result);
}

const std::string &Channel::get_topic() const { return (_topic); }
void Channel::set_topic(const std::string &topic) { _topic = topic; }
bool Channel::is_invite_only() const { return (_invite_only); }
void Channel::set_invite_only(bool value) { _invite_only = value; }
bool Channel::is_topic_restricted() const { return (_topic_restricted); }
void Channel::set_topic_restricted(bool value) { _topic_restricted = value; }
bool Channel::has_key() const { return (!_key.empty()); }
const std::string &Channel::get_key() const { return (_key); }
void Channel::set_key(const std::string &key) { _key = key; }
void Channel::clear_key() { _key.clear(); }
bool Channel::has_user_limit() const { return (_has_user_limit); }
size_t Channel::get_user_limit() const { return (_user_limit); }
void Channel::set_user_limit(size_t limit)
{
    _has_user_limit = true;
    _user_limit = limit;
}
void Channel::clear_user_limit()
{
    _has_user_limit = false;
    _user_limit = 0;
}

std::string Channel::mode_string() const
{
    std::string modes = "+";
    if (_invite_only)
        modes += "i";
    if (_topic_restricted)
        modes += "t";
    if (has_key())
        modes += "k";
    if (_has_user_limit)
        modes += "l";
    return (modes);
}
