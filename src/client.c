#include "client.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnet.h>
#include <signal.h>

sock_t start_client(char* address, uint16_t port, int enc)
{
    sock_t sock;
    sock.ssl = NULL;

    if (enc)
    {
        SSL_library_init();
        sock.ctx = init_client_ctx();
    }
    else
    {
        sock.ctx = NULL;
    }

    // create the socket
    if ((sock.fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failure");
        exit(EXIT_FAILURE);
    }

    // zero memory block
    bzero(&sock.addr, sizeof(sock.addr));

    sock.addr.sin_family = AF_INET;
    sock.addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &sock.addr.sin_addr) <= 0)
    {
        perror("Invalid / unsupported address");
        exit(EXIT_FAILURE);
    }

    sock.buffer = (char*) malloc(1024);

    return sock;
}

void connect_client(sock_t *sock, int enc)
{
    if (connect(sock->fd, (struct sockaddr *) &sock->addr,
                sizeof(sock->addr)) < 0)
    {
        perror("Connection failure");
        exit(EXIT_FAILURE);
    }

    if (enc)
    {
        sock->ssl = SSL_new(sock->ctx);
        SSL_set_fd(sock->ssl, sock->fd);
    }

    if (enc && SSL_connect(sock->ssl) < 0)
    {
        perror("Connection failure");
        exit(EXIT_FAILURE);
    }
}

void prompt(char *username)
{
    printf("%s > ", username);
}

void run_client(char *address, int port, char *username, int enc)
{
    sock_t sock;
    char *src = username;
    pthread_t r_th = -1;

    // start client as unconnected
    sock.fd = -1;

    // read user input until they enter '.exit' or exit signal received
    signal(SIGINT, handle_signal);
    while (exit_received == 0)
    {
        char input[1024], *cmd, *dest, *msg;
        memset(input, 0, 1024);

        // get user command
        prompt(username);
        fgets(input, 1024, stdin);
        input[strcspn(input, "\n")] = '\0';

        cmd = strtok(input, " ");

        // execute command entered by client
        if (strcmp(".send", cmd) == 0)
        {
            if (sock.fd == -1)
            {
                printf("You are not connected to anyone.\n");
                continue;
            }

            // Parse destination and message from remainder of input1
            // assumes a singular space between command, dest, and msg
            dest = strtok(NULL, " ");
            msg = strtok(NULL, "\0");

            client_send(&sock, dest, src, msg, enc);
        }
        else if (strcmp(".exit", cmd) == 0) // close connection, leave shell loop
        {
            if (sock.fd != -1) {
                disconnect_client(&sock, r_th);
                r_th = -1;
            }

            break;
        }
        else if (strcmp(".connect", cmd) == 0)
        {
            if (sock.fd != -1)
            {
                printf("You are already connected\n");
                continue;
            }

            sock = start_client(address, (uint16_t) port, enc);

            connect_client(&sock, enc);

            // Start new thread for reading from socket
            pthread_create(&r_th, NULL, client_read, &sock);

            // Send initial username packet
            client_send(&sock, src, NULL, NULL, enc);
        }
        else if (strcmp(".disconnect", cmd) == 0) // close connection, stay in shell loop
        {
            if (sock.fd != -1) {
                disconnect_client(&sock, r_th);
                r_th = -1;
            }
        }
        else if (strcmp(".list", cmd) == 0)
        {
            client_send(&sock, "LIST", src, NULL, enc);
        }
        else
        {
            printf("unsupported command %s\n", cmd);
        }
    }

    if (sock.ctx != NULL) SSL_CTX_free(sock.ctx);
}

void disconnect_client(sock_t *sock, pthread_t th)
{
    if (th != -1) pthread_cancel(th);
    shutdown_fd(sock->fd);
    sock->fd = -1;
    free(sock->buffer);

    if (sock->ssl != NULL)
    {
        SSL_free(sock->ssl);
        sock->ssl = NULL;
    }
}

ssize_t client_send(sock_t *sock, char *dest, char *src, char *msg, int enc)
{
    char* packet = NULL;

    form_packet(&packet, dest, src, msg, NULL);

    int bytes_sent = chat_send(sock->fd, sock->ssl, enc, packet);

    free(packet);
    return bytes_sent;
}

void *client_read(void *sock)
{
    sock_t *c_sock = (sock_t*) sock;

    while (1) {
        int bytes_read = chat_read(c_sock->fd, c_sock->ssl, c_sock->ssl != NULL, c_sock->buffer);

        if (bytes_read > 0)
        {
            printf("%s\n", c_sock->buffer);
        }
        else
        {
            disconnect_client(sock, -1);
            break;
        }
    }

    return NULL;
}