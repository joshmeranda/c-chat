#include "client.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnet.h>
#include <signal.h>
#include <curses.h>
#include <client_ui.h>

sock_t start_client(char* address, uint16_t port, sock_t *sock, int enc)
{
    sock->ssl = NULL;

    if (enc)
    {
        SSL_library_init();
        sock->ctx = init_client_ctx();
    }
    else
    {
        sock->ctx = NULL;
    }

    // create the socket
    if ((sock->fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failure");
        exit(EXIT_FAILURE);
    }

    // zero memory block
    bzero(&sock->addr, sizeof(sock->addr));

    sock->addr.sin_family = AF_INET;
    sock->addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &sock->addr.sin_addr) <= 0)
    {
        perror("Invalid / unsupported address");
        exit(EXIT_FAILURE);
    }

    sock->buffer = (char*) malloc(1024);

    return *sock;
}

void connect_client(sock_t *sock, int enc)
{
    if ((connect(sock->fd, (struct sockaddr *) &(sock->addr), sizeof(sock->addr))) < 0)
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
    printw("%s > ", username);
    refresh();
}

void run_client(char *address, int port, char *username, int enc)
{
    sock_t sock;
    wdata_t *ui_data = malloc(sizeof(wdata_t));
    pthread_t r_th = -1;

    // start client as unconnected
    sock.fd = -1;
    sock.username = username;

    // read user input until they enter '.exit' or exit signal received
    signal(SIGINT, handle_signal);

    // setup ui data
    initscr();
    ui_data->offset = 0;
    ui_data->count = 0;
    ui_data->size = getmaxy(stdscr);
    ui_data->data = calloc(ui_data->size, sizeof(char*));
    ui_data->sock = NULL;

    while (exit_received == 0)
    {
        char input[1024], *cmd, *dest, *msg;
        bzero(input, 1024);

        update_screen(ui_data);

        // get user command
        prompt(username);
        getstr(input);
        if (input[0] == 0) continue;

        cmd = strtok(input, " ");

        // execute command entered by client
        if (strcmp(".send", cmd) == 0)
        {
            if (sock.fd == -1)
            {
                // printf("You are not connected to anyone.\n");
                update_data(ui_data, "You are not connected to anyone.");
                continue;
            }

            // Parse destination and message from remainder of input1
            // assumes a singular space between command, dest, and msg
            dest = strtok(NULL, " ");
            msg = strtok(NULL, "\0");

            client_send(&sock, dest, msg);
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
                // printf("You are already connected\n");
                update_data(ui_data, "You are already connected");
                continue;
            }

            start_client(address, (uint16_t) port, &sock, enc);

            connect_client(&sock, enc);

            ui_data->sock = &sock;

            // Start new thread for reading from socket
            pthread_create(&r_th, NULL, client_read, ui_data);

            // Send initial username packet
            client_send(&sock, sock.username, NULL);
        }
        else if (strcmp(".disconnect", cmd) == 0) // close connection, stay in shell loop
        {
            if (sock.fd != -1) {
                disconnect_client(&sock, r_th);
                ui_data->sock = NULL;
                r_th = -1;
            }
        }
        else if (strcmp(".list", cmd) == 0)
        {
            client_send(&sock, "LIST", NULL);
        }
        else
        {
            // printf("unsupported command %s\n", cmd);
            update_data(ui_data, "Unsupported command.");
        }
    }

    endwin();
    free(ui_data->data);
    free(ui_data);

    if (sock.ctx != NULL) SSL_CTX_free(sock.ctx);
}

void disconnect_client(sock_t *sock, pthread_t th)
{
    if (th != -1) pthread_cancel(th);
    shutdown_socket(sock);
    sock->fd = -1;
    // free(sock->buffer);

    if (sock->ssl != NULL)
    {
        SSL_free(sock->ssl);
        sock->ssl = NULL;
    }
}

ssize_t client_send(sock_t *sock, char *dest, char *msg)
{
    char* packet = NULL;

    form_packet(&packet, dest, sock->username, msg, NULL);

    int bytes_sent = chat_send(sock, packet);

    free(packet);
    return bytes_sent;
}

void *client_read(void *data)
{
    wdata_t *wdata = (wdata_t*) data;
    sock_t *sock = wdata->sock;

    while (1) {
        int bytes_read = chat_read(sock, sock->buffer);

        if (bytes_read > 0)
        {
            update_data(data, sock->buffer);
        }
        else
        {
            disconnect_client(sock, -1);
            break;
        }
    }

    return NULL;
}