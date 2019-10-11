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

    char* command_arr[] =
            {
                    ".send",
                    ".exit",
                    ".help",
                    ".connect",
                    ".disconnect",
                    ".broadcast",
                    ".dest",
                    ".list",
                    "\0"
            };

    // Search through the available commands for the matching value
    for (int i = 0; strcmp(command_arr[i], "\0") != 0; i++)
    {
        // command found
        if (strcmp(command_arr[i], input) == 0) {
            *command = i;
            break;
        }
    }
}

char* form_message_packet(char *dest, char *src, char *message, char *packet)
{
    strcat(packet, dest);
    strcat(packet, DELIMITER);
    strcat(packet, src);
    strcat(packet, DELIMITER);
    strcat(packet, message);
    // printf("packet : %s", packet);

    return packet;
}

char* form_username_packet(char* dest, char* username, char* packet) {
    strcat(packet, dest);
    strcat(packet, DELIMITER);
    strcat(packet, username);
    strcat(packet, DELIMITER);
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

void run_client(char* address, int port, char* username)
{
    struct sock_info c_sock = start_client(address, (uint16_t) port);
    int command, connected = 0;
    char* src = username;

    // read user input until they enter '.exit'
    while (1)
    {
        char dest[USERNAME_MAX];
        memset(dest, 0, USERNAME_MAX);

        printf("%s", get_prompt());
        fgets(c_sock.buffer, BUFFER_SIZE, stdin);

        c_sock.buffer[strcspn(c_sock.buffer, "\n")] = '\0';
        set_command(c_sock.buffer, &command);

        // execute command entered by client
        if (command == SEND)
        {
            if (! connected)
            {
                printf("You are not connected to anyone.\n");
                continue;
            }
            strcat(c_sock.buffer, DELIMITER);

            strcpy(dest, "alice"); // TODO parse from destination variable


            size_t packet_bytes = (strlen(dest)
                                   + strlen(src)
                                   + strlen(c_sock.buffer)
            );
            char packet[packet_bytes];
            memset(packet, 0, packet_bytes);

            form_message_packet(dest, src, c_sock.buffer, packet);

            send_fd(c_sock.fd, packet);
        }
        else if (command == EXIT) // close connection, leave shell loop
        {
            if (connected) { shutdown_fd(c_sock.fd); }
            break;
        }
        else if (command == CONNECT)
        {
            if (connected)
            {
                printf("You are already connected\n");
                continue;
            }
            connect_client(c_sock);
            connected = 1;

            // send message to notify the server of a new message
            strcpy(dest, "USERNAME");

            size_t packet_bytes = (strlen(dest)
                                   + strlen(src)
                                   + strlen(c_sock.buffer)
            );

            char packet[packet_bytes];
            memset(packet, 0 ,packet_bytes);

            form_username_packet(dest, src, packet);
            send_fd(c_sock.fd, packet);
            continue;
        }
        else if (command == DISCONNECT) // close connection, stay in shell loop
        {
            if (connected) { shutdown_fd(c_sock.fd); }
            connected = 0;
            continue;
        }
        else
        {
            printf("unsupported command %s\n", c_sock.buffer);
            continue;
        }

        // read data from server if there is data to be read
        if (server_data(c_sock.fd))
        {
            read_fd(c_sock.fd, c_sock.buffer);
        }
    }
}