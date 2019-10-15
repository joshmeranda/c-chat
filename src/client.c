#include "client.h"
#include <wait.h>
#include <pthread.h>

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

void run_client(char* address, int port, char* username)
{
    struct sock_info c_sock = start_client(address, (uint16_t) port);

    client_write(c_sock, username);

    // // TODO wait to start reading until the client has connected to the server
    // rpid = fork();
    //
    // if (rpid == 0)
    // {
    //     client_read(c_sock);
    // }
    // else
    // {
    //     wpid = fork();
    //
    //     if (wpid == 0)
    //     {
    //         client_write(c_sock, username);
    //     }
    //     else
    //     {
    //         wait(&rpid);
    //         kill(wpid, SIGABRT);
    //     }
    // }
}

void client_write(sock_info c_sock, char* username) {
    char cmd_str[BUFFER_SIZE];
    int command, connected = 0;
    char *src = username;
    pthread_t r_th;

    // read user input until they enter '.exit'
    while (1)
    {
        char dest[USERNAME_MAX];
        memset(dest, 0, USERNAME_MAX);

        printf("%s", get_prompt());
        fgets(cmd_str, BUFFER_SIZE, stdin);

        cmd_str[strcspn(cmd_str, "\n")] = '\0';
        set_command(cmd_str, &command);

        // execute command entered by client
        if (command == SEND)
        {
            if (! connected)
            {
                printf("You are not connected to anyone.\n");
                continue;
            }

            strcpy(dest, "alice"); // TODO parse from destination variable

            size_t packet_bytes = (strlen(dest) + strlen(src) + strlen(cmd_str));
            char packet[packet_bytes];
            memset(packet, 0, packet_bytes);

            form_message_packet(dest, src, cmd_str, packet);

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

            // Start new thread for reading from socket
            pthread_create(&r_th, NULL, client_read, &c_sock);

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

            read_fd(c_sock.fd, packet);
            printf("%s\n", packet);
        }
        else if (command == DISCONNECT) // close connection, stay in shell loop
        {
            if (connected) {
                shutdown_fd(c_sock.fd);
                pthread_join(r_th, NULL);
            }

            connected = 0;

            continue;
        }
        else
        {
            printf("unsupported command %s\n", c_sock.buffer);
            // continue
            break;
        }
    }
}

void *client_read(void *sock) {
    sock_info *c_sock = (sock_info*) sock;
    int bytes_read = 0;

    while (1) {
        bytes_read = read_fd(c_sock->fd, c_sock->buffer);

        if (bytes_read == 0)
        {
            continue;
        }
        else if (bytes_read < 0)
        {
            break;
        }

        printf("RECEIVED: %s\n", c_sock->buffer);
    }
    return NULL;
}