#include "client_shell.h"

enum command_types
{
    EXIT,
    BROADCAST
};

const char* command_arr[] =
{
".exit",
".broadcast",
};

char* get_prompt()
{
    return " $> ";
}

void client_shell(struct sock_info *c_sock)
{
    // read user input until they enter '.exit'
    while (1)
    {
        printf("%s", get_prompt());
        fgets(c_sock->buffer, BUFFER_SIZE, stdin);

        if (strcmp(c_sock->buffer, ".exit") == 0)
        {
            break;
        }

        //send_fd(c_sock.fd, snd_message);
        send_fd(c_sock->fd, c_sock->buffer);
        printf("Client sent\n");

        printf("000\n");
        read_fd(c_sock->fd, c_sock->buffer);
        printf("c recs %s\n", c_sock->buffer);
        printf("001\n");
    }
}