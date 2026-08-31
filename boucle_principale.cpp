#include "head.hpp"

static void close_clients(std::vector<struct pollfd> &fds)
{
    for (size_t i = 1; i < fds.size(); ++i)
        close(fds[i].fd);
}

static bool flush_client_output(int fd, ClientDataBase &db)
{
    Client *client = db.get_client(fd);
    if (client == NULL || client->get_output_buffer().empty())
        return (true);
    const std::string &output = client->get_output_buffer();
    ssize_t sent = send(fd, output.data(), output.size(), 0);
    if (sent <= 0)
        return (false);
    client->consume_output(static_cast<size_t>(sent));
    return (true);
}

static void disconnect_client(size_t index, std::vector<struct pollfd> &fds,
    Server &server)
{
    ClientDataBase &db = server.get_db();
    Client *client = db.get_client(fds[index].fd);
    if (client != NULL && client->is_registered() && !client->is_closing())
    {
        const std::string nickname = client->get_name();
        const std::string event = ":" + nickname + "!" + client->get_username()
            + "@localhost QUIT :Connection closed\r\n";
        server.notify_client_channels(nickname, event);
        server.remove_client_from_channels(nickname);
    }
    db.remove_client(fds[index].fd);
    close(fds[index].fd);
    fds.erase(fds.begin() + index);
}

static void refresh_write_events(std::vector<struct pollfd> &fds,
    Server &server)
{
    ClientDataBase &db = server.get_db();
    for (size_t i = 1; i < fds.size(); )
    {
        Client *client = db.get_client(fds[i].fd);
        if (client != NULL && client->has_output_overflow())
        {
            disconnect_client(i, fds, server);
            continue;
        }
        fds[i].events = POLLIN;
        if (client != NULL && !client->get_output_buffer().empty())
            fds[i].events |= POLLOUT;
        ++i;
    }
}

int boucle_principale(int fd_server, const std::string &password)
{
    Server server(password);
    std::vector<struct pollfd> fds;
    struct pollfd server_pfd;

    server_pfd.fd = fd_server;
    server_pfd.events = POLLIN;
    server_pfd.revents = 0;
    fds.push_back(server_pfd);
    while (server_is_running())
    {
        if (poll(&fds[0], fds.size(), -1) < 0)
        {
            if (!server_is_running())
                break;
            std::cerr << "Error: poll failed" << std::endl;
            close_clients(fds);
            return (1);
        }
        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].fd == fd_server && (fds[i].revents & POLLHUP))
            {
                std::cerr << "Error: listening socket hung up" << std::endl;
                close_clients(fds);
                return (1);
            }
            if (fds[i].revents & (POLLERR | POLLNVAL))
            {
                if (fds[i].fd == fd_server)
                {
                    std::cerr << "Error: listening socket failed" << std::endl;
                    close_clients(fds);
                    return (1);
                }
                disconnect_client(i, fds, server);
                --i;
                continue;
            }
            if (fds[i].fd != fd_server && (fds[i].revents & POLLOUT))
            {
                if (!flush_client_output(fds[i].fd, server.get_db()))
                {
                    disconnect_client(i, fds, server);
                    --i;
                    continue;
                }
                Client *client = server.get_db().get_client(fds[i].fd);
                if (client != NULL && client->is_closing()
                    && client->get_output_buffer().empty())
                {
                    disconnect_client(i, fds, server);
                    --i;
                    continue;
                }
            }
            if (!(fds[i].revents & POLLIN))
            {
                if (!(fds[i].revents & POLLHUP))
                    continue;
            }
            else if (fds[i].fd == fd_server)
            {
                int new_client = accept(fd_server, NULL, NULL);
                if (new_client >= 0)
                {
                    if (fcntl(new_client, F_SETFL, O_NONBLOCK) < 0)
                        close(new_client);
                    else
                    {
                        struct pollfd client_pfd;
                        client_pfd.fd = new_client;
                        client_pfd.events = POLLIN;
                        client_pfd.revents = 0;
                        fds.push_back(client_pfd);
                        server.get_db().add_pending_client(new_client);
                    }
                }
                break;
            }
            else if (handle_client_data(fds[i].fd, server) != 0)
            {
                fds.erase(fds.begin() + i);
                --i;
                continue;
            }
            if (i < fds.size() && (fds[i].revents & POLLHUP))
            {
                Client *client = server.get_db().get_client(fds[i].fd);
                if (client != NULL && !client->get_output_buffer().empty())
                    client->set_closing(true);
                else
                {
                    disconnect_client(i, fds, server);
                    --i;
                }
            }
        }
        refresh_write_events(fds, server);
    }
    close_clients(fds);
    return (0);
}
