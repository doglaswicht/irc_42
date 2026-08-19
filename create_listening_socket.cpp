#include "head.hpp"

//le fd qu on retourne
int create_listening_socket(const char *port_char)
{
    char *end = NULL;
    long port = std::strtol(port_char, &end, 10);
    if (port_char[0] == '\0' || *end != '\0' || port < 1 || port > 65535)
    {
        std::cerr << "Error: port must be a number between 1 and 65535" << std::endl;
        return (-1);
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        std::cerr << "Error: socket creation failed" << std::endl;
        return (-1);
    }
    int enabled = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0)
    {
        std::cerr << "Error: setsockopt failed" << std::endl;
        close(fd);
        return (-1);
    }
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<unsigned short>(port));
    if (bind(fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
    {
        std::cerr << "Error: could not bind to port " << port << std::endl;
        close(fd);
        return (-1);
    }
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cerr << "Error: could not make listening socket non-blocking" << std::endl;
        close(fd);
        return (-1);
    }
    if (listen(fd, SOMAXCONN) < 0)
    {
        std::cerr << "Error: listen failed" << std::endl;
        close(fd);
        return (-1);
    }
    return (fd);
}
