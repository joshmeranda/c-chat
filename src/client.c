#include "client.h"
#include "enc_chat_socket.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnet.h>

SOCK start_client(char* address, uint16_t port, int enc)
{
    SOCK sock;

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

    return sock;
}

void connect_client(SOCK *sock, int enc)
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

char* form_packet(char **packet, ...)
{
    va_list args;
    int packet_len = 1;
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
    (*packet)[0] = '\0';

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

void run_client(char *address, int port, char *username, int enc)
{
    SOCK sock;
    SSL_CTX *ctx = NULL;
    int connected = 0;
    char *src = username;
    pthread_t r_th = -1;

    if (enc)
    {
        SSL_library_init();
        ctx = init_client_ctx();
    }

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
                free(cmd);
                continue;
            }

            // Parse destination and message from remainder of input1
            // assumes a singular space between command, dest, and msg
            dest = get_next_word(&input[strlen(cmd) + 1]);
            msg = &input[strlen(cmd) + strlen(dest) + 2];

            client_send(&sock, dest, src, msg, enc);

            free(dest);
        }
        else if (strcmp(".exit", cmd) == 0) // close connection, leave shell loop
        {
            if (connected) {
                disconnect_client(sock.fd, r_th);
                r_th = -1;
            }

            free(cmd);
            break;
        }
        else if (strcmp(".connect", cmd) == 0)
        {
            if (connected)
            {
                printf("You are already connected\n");
                free(cmd);
                continue;
            }

            sock = start_client(address, (uint16_t) port, enc);
            if (enc) sock.ctx = ctx;

            connect_client(&sock, enc);
            connected = 1;

            // Start new thread for reading from socket
            pthread_create(&r_th, NULL, client_read, &sock);

            client_send(&sock, "USERNAME", src, NULL, enc);
        }
        else if (strcmp(".disconnect", cmd) == 0) // close connection, stay in shell loop
        {
            if (connected) {
                disconnect_client(sock.fd, r_th);
                connected = 0;
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

        free(cmd);
    }

    if (ctx != NULL) SSL_CTX_free(ctx);
}

void disconnect_client(int fd, pthread_t th)
{
    pthread_cancel(th);
    shutdown_fd(fd);
}

ssize_t client_send(SOCK *sock, char *dest, char *src, char *msg, int enc) {
    char* packet;

    form_packet(&packet, dest, src, msg, NULL);

    int bytes_sent;

    if (enc)
    {
        bytes_sent = send_ssl(sock->ssl, packet);
    }
    else
    {
        bytes_sent = send_fd(sock->fd, packet);
    }

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
    SOCK *c_sock = (SOCK*) sock;
    int bytes_read = 0;

    while (1) {
        if (c_sock->ssl != NULL)
        {
            bytes_read = read_ssl(c_sock->ssl, c_sock->buffer);
        }
        else
        {
            bytes_read = read_fd(c_sock->fd, c_sock->buffer);
        }

        if (bytes_read > 0)
        {
            printf("%s\n", c_sock->buffer);
        }
        else if (bytes_read < 0)
        {
            break;
        }
    }

    return NULL;
}