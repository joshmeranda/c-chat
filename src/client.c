#include "client.h"

char* get_prompt()
{
    return " $> ";
}

void set_command(char* input, int* command)
{
    // if not a command default to SEND
    if (input[0] != '.')
    {
        *command = SEND;
        return;
    }

    int i = 0;
    while (strcmp(command_arr[i], "\0") != 0)
    {
        if (strcmp(command_arr[i], input) == 0) { break; } // command found
        i++;
    }

    *command = i;
}

char* form_packet(char* dest, char* src, char* message, char* packet)
{
    strcat(packet, dest);
    strcat(packet, DELIMITER);
    strcat(packet, src);
    strcat(packet, DELIMITER);
    strcat(packet, message);
    return packet;
}

int server_data(int fd)
{
    fd_set read_fds;
    FD_SET(fd, &read_fds);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(fd, &read_fds, NULL, NULL, &timeout);
}