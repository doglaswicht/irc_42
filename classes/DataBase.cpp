#include "DataBase.hpp"
#include <cctype>

static std::string irc_lower(const std::string &value)
{
    std::string result = value;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::tolower(
            static_cast<unsigned char>(result[i])));
    return (result);
}

ClientDataBase::ClientDataBase() {}

Client *ClientDataBase::add_pending_client(int fd)
{
    _pending_clients[fd] = Client();
    _connected_clients[fd] = &_pending_clients[fd];
    return (_connected_clients[fd]);
}

void ClientDataBase::register_client(int fd)
{
    std::map<int, Client>::iterator pending = _pending_clients.find(fd);
    if (pending == _pending_clients.end())
        return;
    const std::string nickname = pending->second.get_name();
    _registered_clients[nickname] = pending->second;
    _pending_clients.erase(pending);
    _connected_clients[fd] = &_registered_clients[nickname];
}

void ClientDataBase::remove_client(int fd)
{
    std::map<int, Client*>::iterator connected = _connected_clients.find(fd);
    if (connected == _connected_clients.end())
        return;
    Client *client = connected->second;
    const bool registered = client->is_registered();
    const std::string nickname = client->get_name();
    _connected_clients.erase(connected);
    if (registered)
        _registered_clients.erase(nickname);
    else
        _pending_clients.erase(fd);
}

Client *ClientDataBase::get_client(int fd)
{
    std::map<int, Client*>::iterator it = _connected_clients.find(fd);
    if (it == _connected_clients.end())
        return (NULL);
    return (it->second);
}

int ClientDataBase::get_fd_by_name(const std::string &name) const
{
    const std::string wanted = irc_lower(name);
    for (std::map<int, Client*>::const_iterator it = _connected_clients.begin();
        it != _connected_clients.end(); ++it)
    {
        if (irc_lower(it->second->get_name()) == wanted)
            return (it->first);
    }
    return (-1);
}

bool ClientDataBase::is_nickname_in_use(const std::string &name,
    int except_fd) const
{
    const std::string wanted = irc_lower(name);
    for (std::map<int, Client*>::const_iterator it = _connected_clients.begin();
        it != _connected_clients.end(); ++it)
    {
        if (it->first != except_fd && it->second->has_nick()
            && irc_lower(it->second->get_name()) == wanted)
            return (true);
    }
    return (false);
}

int ClientDataBase::update_nickname(int fd, const std::string &nickname)
{
    std::map<int, Client*>::iterator connected = _connected_clients.find(fd);
    if (connected == _connected_clients.end())
        return (1);
    Client *current = connected->second;
    if (!current->is_registered())
    {
        current->put_name(nickname);
        current->set_nick_received(true);
        return (0);
    }
    const std::string old_nickname = current->get_name();
    std::map<std::string, Client>::iterator stored =
        _registered_clients.find(old_nickname);
    if (stored == _registered_clients.end())
        return (1);
    Client updated = stored->second;
    updated.put_name(nickname);
    _registered_clients.erase(stored);
    _registered_clients[nickname] = updated;
    connected->second = &_registered_clients[nickname];
    return (0);
}
