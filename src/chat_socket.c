#include "chat_socket.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

sock_info start_server(char* address, uint16_t port)
{
    sock_info s_sock;
    int opt = 1;

    // Creating socket file descriptor
    if ((s_sock.fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(s_sock.fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    s_sock.addr.sin_family = AF_INET; // TODO AF_UNIX ?
    // s_sock.addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, address, &s_sock.addr.sin_addr);
    s_sock.addr.sin_port = htons(port);

    // Forcefully attaching socket to the port 8080
    if (bind(s_sock.fd, (struct sockaddr *) &s_sock.addr,
             sizeof(s_sock.addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return s_sock;
}

int accept_connection(sock_info s_sock)
{
    int client_fd,
        address_size = sizeof(s_sock.addr);

    if ((client_fd = accept(s_sock.fd,
                            (struct sockaddr *) &s_sock.addr,
                            (socklen_t*) &address_size)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return client_fd;
}

sock_info start_client(char* address, uint16_t port)
{
    sock_info c_sock;

    // create the socket
    if ((c_sock.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failure");
        exit(EXIT_FAILURE);
    }

    // zero memory block
    bzero(&c_sock.addr, sizeof(c_sock.addr));

    c_sock.addr.sin_family = AF_INET;
    c_sock.addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &c_sock.addr.sin_addr) <= 0)
    {
        perror("Invalid / unsupported address");
        exit(EXIT_FAILURE);
    }

    return c_sock;
}

void connect_client(sock_info c_sock)
{
    if (connect(c_sock.fd, (struct sockaddr *) &c_sock.addr,
                sizeof(c_sock.addr)) < 0)
    {
        perror("Connection failure");
        exit(EXIT_FAILURE);
    }
}

ssize_t read_fd(int fd, char buffer[BUFFER_SIZE])
{
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);

    if (bytes_read < 0)
    {
        perror("read error");
        return bytes_read;
    } else if (bytes_read == 0) {
        return 0;
    }

    if (buffer[bytes_read - 1] != '\0')
    {
        buffer[bytes_read] = '\0';
    }

    return bytes_read;
}

ssize_t send_fd(int fd, char* message)
{
    ssize_t bytes_sent = send(fd, message, strlen(message), 0);

    if (bytes_sent < 0)
    {
        perror("send error");
    }

    return bytes_sent;
}

int shutdown_fd(int fd)
{
    if (shutdown(fd, SHUT_RDWR) < 0)
    {
        perror("Shutdown failure");
        return -1;
    }

    return 0;
}