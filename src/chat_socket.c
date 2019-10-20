#include "chat_socket.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

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