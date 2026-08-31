#include "../head.hpp"

int queue_message(int fd, const std::string &message, ClientDataBase &db)
{
    Client *client = db.get_client(fd);
    if (client == NULL)
        return (1);
    client->queue_output(message);
    return (0);
}

static std::string numeric_target(Client *client)
{
    if (client != NULL && client->has_nick())
        return (client->get_name());
    return ("*");
}

static void send_numeric(int fd, ClientDataBase &db, const std::string &code,
    const std::string &arguments, const std::string &description)
{
    Client *client = db.get_client(fd);
    std::string message = ":ircserv " + code + " " + numeric_target(client);
    if (!arguments.empty())
        message += " " + arguments;
    message += " :" + description + "\r\n";
    queue_message(fd, message, db);
}

static bool is_nickname_first_character(char c)
{
    const std::string special = "[]\\`_^{|}";
    unsigned char value = static_cast<unsigned char>(c);
    return (std::isalpha(value) != 0 || special.find(c) != std::string::npos);
}

static bool is_valid_nickname(const std::string &nickname)
{
    const std::string special = "[]\\`_^{|}-";
    if (nickname.empty() || nickname.size() > 30
        || !is_nickname_first_character(nickname[0]))
        return (false);
    for (size_t i = 1; i < nickname.size(); ++i)
    {
        unsigned char value = static_cast<unsigned char>(nickname[i]);
        if (std::isalnum(value) == 0
            && special.find(nickname[i]) == std::string::npos)
            return (false);
    }
    return (true);
}

static void try_registration(int fd, Server &server)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);

    if (client == NULL || client->is_registered() || !client->is_pass_accepted()
        || !client->has_nick() || !client->has_user())
        return;
    client->set_registered(true);
    db.register_client(fd);
    client = db.get_client(fd);
    queue_message(fd, ":ircserv 001 " + client->get_name()
        + " :Welcome to the IRC network " + client->get_name() + "!"
        + client->get_username() + "@localhost\r\n", db);
}

static void handle_pass(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();

    if (client->is_registered())
        send_numeric(fd, db, "462", "", "You may not reregister");
    else if (params.empty())
        send_numeric(fd, db, "461", "PASS", "Not enough parameters");
    else if (params[0] != server.get_password())
        send_numeric(fd, db, "464", "", "Password incorrect");
    else
    {
        client->set_pass_accepted(true);
        try_registration(fd, server);
    }
}

static void handle_nick(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();

    if (params.empty() || params[0].empty())
        send_numeric(fd, db, "431", "", "No nickname given");
    else if (!is_valid_nickname(params[0]))
        send_numeric(fd, db, "432", params[0], "Erroneous nickname");
    else if (db.is_nickname_in_use(params[0], fd))
        send_numeric(fd, db, "433", params[0], "Nickname is already in use");
    else
    {
        if (client->is_registered())
        {
            const std::string old_name = client->get_name();
            const std::string event = ":" + old_name + "!"
                + client->get_username() + "@localhost NICK :" + params[0] + "\r\n";
            if (!server.notify_client_channels(old_name, event))
                queue_message(fd, event, db);
            if (db.update_nickname(fd, params[0]) == 0)
                server.rename_client_in_channels(old_name, params[0], db.get_client(fd));
        }
        else
        {
            client->put_name(params[0]);
            client->set_nick_received(true);
            try_registration(fd, server);
        }
    }
}

static void handle_user(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();

    if (client->is_registered() || client->has_user())
        send_numeric(fd, db, "462", "", "You may not reregister");
    else if (params.size() < 4)
        send_numeric(fd, db, "461", "USER", "Not enough parameters");
    else
    {
        client->set_username(params[0]);
        client->set_realname(params[3]);
        client->set_user_received(true);
        try_registration(fd, server);
    }
}

static void handle_ping(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    const std::vector<std::string> &params = command.get_parameters();
    if (params.empty() || params[0].empty())
        send_numeric(fd, db, "409", "", "No origin specified");
    else
        queue_message(fd, ":ircserv PONG ircserv :" + params[0] + "\r\n", db);
}

static bool handle_quit(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    std::string reason = "Client Quit";
    if (!command.get_parameters().empty() && !command.get_parameters()[0].empty())
        reason = command.get_parameters()[0];
    if (client != NULL && client->is_registered())
    {
        const std::string nickname = client->get_name();
        const std::string event = ":" + nickname + "!" + client->get_username()
            + "@localhost QUIT :" + reason + "\r\n";
        server.notify_client_channels(nickname, event);
        server.remove_client_from_channels(nickname);
    }
    if (client != NULL)
    {
        queue_message(fd, "ERROR :Closing Link: " + numeric_target(client)
            + " (" + reason + ")\r\n", db);
        client->set_closing(true);
    }
    return (true);
}

static std::vector<std::string> split_targets(const std::string &value)
{
    std::vector<std::string> result;
    size_t start = 0;
    while (start <= value.size())
    {
        size_t comma = value.find(',', start);
        std::string item = value.substr(start,
            comma == std::string::npos ? comma : comma - start);
        if (!item.empty())
            result.push_back(item);
        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }
    return (result);
}

static bool is_valid_channel_name(const std::string &name)
{
    if (name.size() < 2 || name.size() > 50
        || (name[0] != '#' && name[0] != '&'))
        return (false);
    for (size_t i = 1; i < name.size(); ++i)
    {
        if (name[i] == ' ' || name[i] == ',' || name[i] == ':'
            || name[i] == '\a')
            return (false);
    }
    return (true);
}

static std::string client_prefix(Client *client)
{
    return (":" + client->get_name() + "!" + client->get_username()
        + "@localhost");
}

static void part_one_channel(int fd, Server &server, const std::string &name,
    const std::string &reason)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    Channel *channel = server.get_channel(name);
    if (channel == NULL)
        send_numeric(fd, db, "403", name, "No such channel");
    else if (!channel->is_member(client->get_name()))
        send_numeric(fd, db, "442", name, "You're not on that channel");
    else
    {
        const std::string channel_name = channel->get_name();
        const std::string event = client_prefix(client) + " PART "
            + channel_name + " :" + reason + "\r\n";
        channel->broadcast(event, db);
        server.part_channel(channel_name, client->get_name());
    }
}

static void handle_join(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    if (params.empty())
    {
        send_numeric(fd, db, "461", "JOIN", "Not enough parameters");
        return;
    }
    if (params[0] == "0")
    {
        const std::set<std::string> channels = client->get_channels();
        for (std::set<std::string>::const_iterator it = channels.begin();
            it != channels.end(); ++it)
            part_one_channel(fd, server, *it, "Leaving all channels");
        return;
    }
    const std::vector<std::string> names = split_targets(params[0]);
    std::vector<std::string> keys;
    if (params.size() > 1)
        keys = split_targets(params[1]);
    for (size_t i = 0; i < names.size(); ++i)
    {
        if (!is_valid_channel_name(names[i]))
        {
            send_numeric(fd, db, "403", names[i], "No such channel");
            continue;
        }
        Channel *existing = server.get_channel(names[i]);
        if (existing != NULL && existing->is_member(client->get_name()))
            continue;
        if (existing != NULL && existing->is_invite_only()
            && !existing->is_invited(client->get_name()))
        {
            send_numeric(fd, db, "473", names[i], "Cannot join channel (+i)");
            continue;
        }
        if (existing != NULL && existing->has_key()
            && (i >= keys.size() || keys[i] != existing->get_key()))
        {
            send_numeric(fd, db, "475", names[i], "Cannot join channel (+k)");
            continue;
        }
        if (existing != NULL && existing->has_user_limit()
            && existing->member_count() >= existing->get_user_limit())
        {
            send_numeric(fd, db, "471", names[i], "Cannot join channel (+l)");
            continue;
        }
        Channel *channel = server.join_channel(names[i], client);
        const std::string channel_name = channel->get_name();
        channel->broadcast(client_prefix(client) + " JOIN :" + channel_name
            + "\r\n", db);
        if (channel->get_topic().empty())
            send_numeric(fd, db, "331", channel_name, "No topic is set");
        else
            send_numeric(fd, db, "332", channel_name, channel->get_topic());
        send_numeric(fd, db, "353", "= " + channel_name,
            channel->names_list());
        send_numeric(fd, db, "366", channel_name, "End of /NAMES list");
    }
}

static void handle_part(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    const std::vector<std::string> &params = command.get_parameters();
    if (params.empty())
    {
        send_numeric(fd, db, "461", "PART", "Not enough parameters");
        return;
    }
    const std::string reason = params.size() > 1 ? params[1] : "Leaving";
    const std::vector<std::string> names = split_targets(params[0]);
    for (size_t i = 0; i < names.size(); ++i)
        part_one_channel(fd, server, names[i], reason);
}

static void handle_privmsg(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *sender = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    if (params.empty() || params[0].empty())
    {
        send_numeric(fd, db, "411", "", "No recipient given (PRIVMSG)");
        return;
    }
    if (params.size() < 2 || params[1].empty())
    {
        send_numeric(fd, db, "412", "", "No text to send");
        return;
    }
    const std::vector<std::string> targets = split_targets(params[0]);
    for (size_t i = 0; i < targets.size(); ++i)
    {
        if (targets[i][0] == '#' || targets[i][0] == '&')
        {
            Channel *channel = server.get_channel(targets[i]);
            if (channel == NULL)
                send_numeric(fd, db, "403", targets[i], "No such channel");
            else if (!channel->is_member(sender->get_name()))
                send_numeric(fd, db, "404", targets[i], "Cannot send to channel");
            else
            {
                const std::string event = client_prefix(sender) + " PRIVMSG "
                    + channel->get_name() + " :" + params[1] + "\r\n";
                channel->broadcast_except(event, sender->get_name(), db);
            }
        }
        else
        {
            int target_fd = db.get_fd_by_name(targets[i]);
            Client *target = db.get_client(target_fd);
            if (target_fd == -1 || target == NULL || !target->is_registered())
                send_numeric(fd, db, "401", targets[i], "No such nick/channel");
            else
                queue_message(target_fd, client_prefix(sender) + " PRIVMSG "
                    + target->get_name() + " :" + params[1] + "\r\n", db);
        }
    }
}

static bool require_channel_operator(int fd, ClientDataBase &db,
    Channel *channel, Client *client)
{
    if (!channel->is_member(client->get_name()))
    {
        send_numeric(fd, db, "442", channel->get_name(),
            "You're not on that channel");
        return (false);
    }
    if (!channel->is_admin(client->get_name()))
    {
        send_numeric(fd, db, "482", channel->get_name(),
            "You're not channel operator");
        return (false);
    }
    return (true);
}

static void handle_invite(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    if (params.size() < 2)
    {
        send_numeric(fd, db, "461", "INVITE", "Not enough parameters");
        return;
    }
    Channel *channel = server.get_channel(params[1]);
    if (channel == NULL)
    {
        send_numeric(fd, db, "403", params[1], "No such channel");
        return;
    }
    if (!require_channel_operator(fd, db, channel, client))
        return;
    int target_fd = db.get_fd_by_name(params[0]);
    Client *target = db.get_client(target_fd);
    if (target_fd == -1 || target == NULL || !target->is_registered())
        send_numeric(fd, db, "401", params[0], "No such nick/channel");
    else if (channel->is_member(target->get_name()))
        send_numeric(fd, db, "443", target->get_name() + " "
            + channel->get_name(), "is already on channel");
    else
    {
        channel->invite(target);
        queue_message(fd, ":ircserv 341 " + client->get_name() + " "
            + target->get_name() + " " + channel->get_name() + "\r\n", db);
        queue_message(target_fd, client_prefix(client) + " INVITE "
            + target->get_name() + " :" + channel->get_name() + "\r\n", db);
    }
}

static void handle_kick(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    if (params.size() < 2)
    {
        send_numeric(fd, db, "461", "KICK", "Not enough parameters");
        return;
    }
    Channel *channel = server.get_channel(params[0]);
    if (channel == NULL)
    {
        send_numeric(fd, db, "403", params[0], "No such channel");
        return;
    }
    if (!require_channel_operator(fd, db, channel, client))
        return;
    int target_fd = db.get_fd_by_name(params[1]);
    Client *target = db.get_client(target_fd);
    if (target_fd == -1 || target == NULL
        || !channel->is_member(target->get_name()))
    {
        send_numeric(fd, db, "441", params[1] + " " + channel->get_name(),
            "They aren't on that channel");
        return;
    }
    const std::string channel_name = channel->get_name();
    const std::string reason = params.size() > 2 ? params[2] : client->get_name();
    channel->broadcast(client_prefix(client) + " KICK " + channel_name + " "
        + target->get_name() + " :" + reason + "\r\n", db);
    server.part_channel(channel_name, target->get_name());
}

static void handle_topic(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    if (params.empty())
    {
        send_numeric(fd, db, "461", "TOPIC", "Not enough parameters");
        return;
    }
    Channel *channel = server.get_channel(params[0]);
    if (channel == NULL)
    {
        send_numeric(fd, db, "403", params[0], "No such channel");
        return;
    }
    if (!channel->is_member(client->get_name()))
    {
        send_numeric(fd, db, "442", channel->get_name(),
            "You're not on that channel");
        return;
    }
    if (params.size() == 1)
    {
        if (channel->get_topic().empty())
            send_numeric(fd, db, "331", channel->get_name(), "No topic is set");
        else
            send_numeric(fd, db, "332", channel->get_name(), channel->get_topic());
        return;
    }
    if (channel->is_topic_restricted() && !channel->is_admin(client->get_name()))
    {
        send_numeric(fd, db, "482", channel->get_name(),
            "You're not channel operator");
        return;
    }
    channel->set_topic(params[1]);
    channel->broadcast(client_prefix(client) + " TOPIC " + channel->get_name()
        + " :" + params[1] + "\r\n", db);
}

static bool parse_positive_limit(const std::string &value, size_t &limit)
{
    char *end = NULL;
    long parsed = std::strtol(value.c_str(), &end, 10);
    if (value.empty() || *end != '\0' || parsed <= 0)
        return (false);
    limit = static_cast<size_t>(parsed);
    return (true);
}

static void broadcast_mode(Client *client, Channel *channel,
    ClientDataBase &db, char sign, char mode, const std::string &argument)
{
    std::string event = client_prefix(client) + " MODE " + channel->get_name()
        + " " + sign + mode;
    if (!argument.empty())
        event += " " + argument;
    channel->broadcast(event + "\r\n", db);
}

static void handle_mode(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    if (params.empty())
    {
        send_numeric(fd, db, "461", "MODE", "Not enough parameters");
        return;
    }
    Channel *channel = server.get_channel(params[0]);
    if (channel == NULL)
    {
        send_numeric(fd, db, "403", params[0], "No such channel");
        return;
    }
    if (params.size() == 1)
    {
        std::string arguments;
        if (channel->has_key())
            arguments += " " + channel->get_key();
        if (channel->has_user_limit())
        {
            std::ostringstream limit;
            limit << channel->get_user_limit();
            arguments += " " + limit.str();
        }
        queue_message(fd, ":ircserv 324 " + client->get_name() + " "
            + channel->get_name() + " " + channel->mode_string()
            + arguments + "\r\n", db);
        return;
    }
    if (!require_channel_operator(fd, db, channel, client))
        return;
    bool adding = true;
    size_t argument = 2;
    const std::string &modes = params[1];
    for (size_t i = 0; i < modes.size(); ++i)
    {
        char mode = modes[i];
        if (mode == '+' || mode == '-')
        {
            adding = (mode == '+');
            continue;
        }
        if (mode == 'i')
            channel->set_invite_only(adding);
        else if (mode == 't')
            channel->set_topic_restricted(adding);
        else if (mode == 'k')
        {
            if (adding && argument >= params.size())
            {
                send_numeric(fd, db, "461", "MODE", "Not enough parameters");
                continue;
            }
            if (adding && channel->has_key())
            {
                send_numeric(fd, db, "467", channel->get_name(),
                    "Channel key already set");
                ++argument;
                continue;
            }
            std::string key;
            if (adding)
            {
                key = params[argument++];
                channel->set_key(key);
            }
            else
                channel->clear_key();
            broadcast_mode(client, channel, db, adding ? '+' : '-', mode, key);
            continue;
        }
        else if (mode == 'o')
        {
            if (argument >= params.size())
            {
                send_numeric(fd, db, "461", "MODE", "Not enough parameters");
                continue;
            }
            std::string nickname = params[argument++];
            int target_fd = db.get_fd_by_name(nickname);
            Client *target = db.get_client(target_fd);
            if (target_fd == -1 || target == NULL
                || !channel->is_member(target->get_name()))
            {
                send_numeric(fd, db, "441", nickname + " " + channel->get_name(),
                    "They aren't on that channel");
                continue;
            }
            nickname = target->get_name();
            if (adding)
                channel->add_admin(target);
            else
                channel->remove_admin(nickname);
            broadcast_mode(client, channel, db, adding ? '+' : '-', mode, nickname);
            continue;
        }
        else if (mode == 'l')
        {
            std::string value;
            if (adding)
            {
                size_t limit;
                if (argument >= params.size()
                    || !parse_positive_limit(params[argument], limit))
                {
                    send_numeric(fd, db, "461", "MODE", "Invalid user limit");
                    if (argument < params.size())
                        ++argument;
                    continue;
                }
                value = params[argument++];
                channel->set_user_limit(limit);
            }
            else
                channel->clear_user_limit();
            broadcast_mode(client, channel, db, adding ? '+' : '-', mode, value);
            continue;
        }
        else
        {
            send_numeric(fd, db, "472", std::string(1, mode),
                "is unknown mode char to me");
            continue;
        }
        broadcast_mode(client, channel, db, adding ? '+' : '-', mode, "");
    }
}

static void handle_cap(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    if (params.empty())
        return;
    const std::string target = client->has_nick() ? client->get_name() : "*";
    if (params[0] == "LS" || params[0] == "LIST")
        queue_message(fd, ":ircserv CAP " + target + " " + params[0]
            + " :\r\n", db);
    else if (params[0] == "REQ")
    {
        const std::string requested = params.size() > 1 ? params[1] : "";
        queue_message(fd, ":ircserv CAP " + target + " NAK :"
            + requested + "\r\n", db);
    }
}

static void send_names(int fd, ClientDataBase &db, Channel *channel)
{
    if (channel == NULL)
        return;
    send_numeric(fd, db, "353", "= " + channel->get_name(),
        channel->names_list());
    send_numeric(fd, db, "366", channel->get_name(), "End of /NAMES list");
}

static void handle_names(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    const std::vector<std::string> &params = command.get_parameters();
    if (params.empty())
    {
        send_numeric(fd, db, "366", "*", "End of /NAMES list");
        return;
    }
    const std::vector<std::string> names = split_targets(params[0]);
    for (size_t i = 0; i < names.size(); ++i)
    {
        Channel *channel = server.get_channel(names[i]);
        if (channel != NULL)
            send_names(fd, db, channel);
        else
            send_numeric(fd, db, "366", names[i], "End of /NAMES list");
    }
}

static void send_who_member(int fd, ClientDataBase &db, Client *requester,
    Client *member, Channel *channel)
{
    std::string flags = "H";
    if (channel != NULL && channel->is_admin(member->get_name()))
        flags += "@";
    const std::string channel_name = channel != NULL ? channel->get_name() : "*";
    queue_message(fd, ":ircserv 352 " + requester->get_name() + " "
        + channel_name + " " + member->get_username()
        + " localhost ircserv " + member->get_name() + " " + flags
        + " :0 " + member->get_realname() + "\r\n", db);
}

static void handle_who(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *requester = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    const std::string mask = params.empty() ? "*" : params[0];
    Channel *channel = server.get_channel(mask);
    if (channel != NULL)
    {
        std::set<Client*> members;
        channel->collect_members(members);
        for (std::set<Client*>::iterator it = members.begin();
            it != members.end(); ++it)
            send_who_member(fd, db, requester, *it, channel);
    }
    else
    {
        int target_fd = db.get_fd_by_name(mask);
        Client *target = db.get_client(target_fd);
        if (target != NULL && target->is_registered())
            send_who_member(fd, db, requester, target, NULL);
    }
    send_numeric(fd, db, "315", mask, "End of /WHO list");
}

static void handle_notice(int fd, Server &server, const IRCCommand &command)
{
    ClientDataBase &db = server.get_db();
    Client *sender = db.get_client(fd);
    const std::vector<std::string> &params = command.get_parameters();
    if (params.size() < 2 || params[0].empty() || params[1].empty())
        return;
    const std::vector<std::string> targets = split_targets(params[0]);
    for (size_t i = 0; i < targets.size(); ++i)
    {
        const std::string event = client_prefix(sender) + " NOTICE "
            + targets[i] + " :" + params[1] + "\r\n";
        if (targets[i][0] == '#' || targets[i][0] == '&')
        {
            Channel *channel = server.get_channel(targets[i]);
            if (channel != NULL && channel->is_member(sender->get_name()))
                channel->broadcast_except(event, sender->get_name(), db);
        }
        else
        {
            int target_fd = db.get_fd_by_name(targets[i]);
            Client *target = db.get_client(target_fd);
            if (target != NULL && target->is_registered())
                queue_message(target_fd, event, db);
        }
    }
}

static bool process_complete_line(int fd, Server &server, const std::string &line)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    IRCCommand command;

    if (client == NULL || !command.parse(line))
        return (false);
    if (command.get_command() == "PASS")
        handle_pass(fd, server, command);
    else if (command.get_command() == "NICK")
        handle_nick(fd, server, command);
    else if (command.get_command() == "USER")
        handle_user(fd, server, command);
    else if (command.get_command() == "PING")
        handle_ping(fd, server, command);
    else if (command.get_command() == "PONG")
        return (false);
    else if (command.get_command() == "QUIT")
        return (handle_quit(fd, server, command));
    else if (command.get_command() == "CAP")
        handle_cap(fd, server, command);
    else if (!client->is_registered())
        send_numeric(fd, db, "451", command.get_command(), "You have not registered");
    else if (command.get_command() == "JOIN")
        handle_join(fd, server, command);
    else if (command.get_command() == "PART")
        handle_part(fd, server, command);
    else if (command.get_command() == "PRIVMSG")
        handle_privmsg(fd, server, command);
    else if (command.get_command() == "INVITE")
        handle_invite(fd, server, command);
    else if (command.get_command() == "KICK")
        handle_kick(fd, server, command);
    else if (command.get_command() == "TOPIC")
        handle_topic(fd, server, command);
    else if (command.get_command() == "MODE")
        handle_mode(fd, server, command);
    else if (command.get_command() == "NAMES")
        handle_names(fd, server, command);
    else if (command.get_command() == "WHO")
        handle_who(fd, server, command);
    else if (command.get_command() == "NOTICE")
        handle_notice(fd, server, command);
    else
        send_numeric(fd, db, "421", command.get_command(), "Unknown command");
    return (false);
}

int handle_client_data(int fd, Server &server)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fd);
    char buffer[BUFFER_SIZE];

    if (client == NULL)
        return (0);
    ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);
    if (bytes <= 0)
    {
        if (bytes < 0)
            std::cerr << "Error: recv failed for client " << fd << std::endl;
        if (client->is_registered())
        {
            const std::string nickname = client->get_name();
            const std::string event = ":" + nickname + "!" + client->get_username()
                + "@localhost QUIT :Connection closed\r\n";
            server.notify_client_channels(nickname, event);
            server.remove_client_from_channels(nickname);
        }
        db.remove_client(fd);
        close(fd);
        return (1);
    }
    client->append_input(buffer, static_cast<size_t>(bytes));
    if (client->get_input_buffer().size() > 8192)
    {
        std::cerr << "Error: input buffer limit exceeded for client " << fd << std::endl;
        db.remove_client(fd);
        close(fd);
        return (1);
    }
    std::string line;
    while (client != NULL && client->pop_line(line))
    {
        if (process_complete_line(fd, server, line))
            break;
        client = db.get_client(fd);
    }
    return (0);
}
