#include "client.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_prompt()
{
    return "$> ";
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

char* form_packet(char **packet, ...)
{
    va_list args;
    int packet_len= 0;
    char *arg;

    // get the length of the packet
    va_start(args, packet);
    arg = va_arg(args, char*);
    while (arg != NULL)
    {
        packet_len += strlen(arg) + 1; // length of section with DELIMITER
        arg = va_arg(args, char*);
    }
    va_end(args);

    // allocate space for the packet
    *packet = (char *) malloc(packet_len);

    // form the packet
    va_start(args, packet);
    arg = va_arg(args, char*);
    while (arg != NULL)
    {
        strcat(*packet, arg);
        strcat(*packet, DELIMITER);
        arg = va_arg(args, char*);
    }
    va_end(args);

    return *packet;
}

void run_client(char* address, int port, char* username)
{
    // sock_info c_sock = start_client(address, (uint16_t) port);
    sock_info c_sock;
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

            client_send(c_sock.fd, "alice", src, cmd_str);
        }
        else if (command == EXIT) // close connection, leave shell loop
        {
            if (connected) disconnect_client(c_sock.fd, r_th);
            break;
        }
        else if (command == CONNECT)
        {
            if (connected)
            {
                printf("You are already connected\n");
                continue;
            }
            c_sock = start_client(address, (uint16_t) port);

            connect_client(c_sock);
            connected = 1;

            // Start new thread for reading from socket
            pthread_create(&r_th, NULL, client_read, &c_sock);

            client_send(c_sock.fd, "USERNAME", src, NULL);
        }
        else if (command == DISCONNECT) // close connection, stay in shell loop
        {
            if (connected) {
                disconnect_client(c_sock.fd, r_th);
                connected = 0;
            }
        }
        else
        {
            printf("unsupported command %s\n", cmd_str);
        }
    }
}

void disconnect_client(int fd, pthread_t th)
{
    pthread_cancel(th);
    shutdown_fd(fd);
}

ssize_t client_send(int fd, char* dest, char* src, char* msg) {
    char* packet;

    if (msg != NULL)
    {
        form_packet(&packet, dest, src, msg, NULL);
    }
    else
    {
        form_packet(&packet, dest, src, NULL);
    }
    int bytes_sent = send_fd(fd, packet);

    free(packet);
    return bytes_sent;
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

        printf("%s\n", c_sock->buffer);
    }
    return NULL;
}
