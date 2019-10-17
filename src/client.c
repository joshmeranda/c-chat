#include "client.h"
#include <wait.h>
#include <pthread.h>

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

char* form_packet(char *dest, char *src, char *message, char *packet)
{
    strcat(packet, dest);
    strcat(packet, DELIMITER);
    strcat(packet, src);
    strcat(packet, DELIMITER);
    strcat(packet, message);

    return packet;
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

            client_send(c_sock.fd, "USERNAME", src, c_sock.buffer);
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
            printf("unsupported command %s\n", c_sock.buffer);
        }
    }
}

void disconnect_client(int fd, pthread_t th)
{
    pthread_cancel(th);
    shutdown_fd(fd);
}

ssize_t client_send(int fd, char* dest, char* src, char* msg) {
    size_t packet_bytes = (strlen(dest) + strlen(src) + strlen(msg));
    char* packet = malloc(packet_bytes);

    form_packet(dest, src, msg, packet);
    packet_bytes = send_fd(fd, packet);

    free(packet);
    return packet_bytes;
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
