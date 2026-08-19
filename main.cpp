#include "head.hpp"

static volatile sig_atomic_t g_running = 1;

static void stop_server(int signal_number)
{
    (void)signal_number;
    g_running = 0;
}

bool server_is_running()
{
    return (g_running != 0);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return (1);
    }
    if (argv[2][0] == '\0')
    {
        std::cerr << "Error: password must not be empty" << std::endl;
        return (1);
    }
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, stop_server);
    signal(SIGTERM, stop_server);
    int listening_fd = create_listening_socket(argv[1]);
    if (listening_fd < 0)
        return (1);
    int status = boucle_principale(listening_fd, argv[2]);
    close(listening_fd);
    return (status);
}
