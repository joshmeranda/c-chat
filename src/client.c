#include "client.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prompt(char *username)
{
    printf("%s> ", username);
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

void run_client(char *address, int port, char *username)
{
    // sock_info c_sock = start_client(address, (uint16_t) port);
    sock_info c_sock;
    int connected = 0;
    char *src = username;
    pthread_t r_th;

    // read user input until they enter '.exit'
    while (1)
    {
        char input[1024], *cmd, *dest, *msg;
        memset(input, 0, 1024);

        // get user command
        prompt(username);
        fgets(input, 1024, stdin);
        input[strcspn(input, "\n")] = '\0';

        cmd = get_next_word(input);

        // execute command entered by client
        if (strcmp(".send", cmd) == 0)
        {
            if (! connected)
            {
                printf("You are not connected to anyone.\n");
                continue;
            }

            // Parse destination and message from remainder of input
            // assumes a singular space between command, dest, and msg
            dest = get_next_word(&input[strlen(cmd) + 1]);
            msg = &input[strlen(cmd) + strlen(dest) + 2];

            client_send(c_sock.fd, dest, src, msg);

            free(dest);
        }
        else if (strcmp(".exit", cmd) == 0) // close connection, leave shell loop
        {
            if (connected) disconnect_client(c_sock.fd, r_th);
            break;
        }
        else if (strcmp(".connect", cmd) == 0)
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
        else if (strcmp("disconnect", cmd) == 0) // close connection, stay in shell loop
        {
            if (connected) {
                disconnect_client(c_sock.fd, r_th);
                connected = 0;
            }
        }
        else
        {
            printf("unsupported command %s\n", cmd);
        }

        free(cmd);
    }
}

void disconnect_client(int fd, pthread_t th)
{
    pthread_cancel(th);
    shutdown_fd(fd);
}

ssize_t client_send(int fd, char *dest, char *src, char *msg) {
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
    printf("%s\n", packet);

    free(packet);
    return bytes_sent;
}

char *get_next_word(char *input)
{
    int len = strcspn(input, " \n\0") + 1;
    char *word = (char*) malloc(len);
    strncpy(word, input, len);
    word[len - 1] = '\0';

    return word;
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
