#include "server.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int accept_connection(sock_t **sock, int fd, FILE *log, int enc)
{
    sock_t *new_sock = malloc(sizeof(sock_t));
    int address_size = sizeof(&(new_sock->addr));

    if ((new_sock->fd = accept(fd,
                            (struct sockaddr *) &(new_sock->addr),
                            (socklen_t*) &address_size)) < 0)
    {
        return -1;
    }

    new_sock->buffer = malloc(1024);

    if (new_sock->fd == -1)
        server_event_log_entry(log, CERR, inet_ntoa(new_sock->addr.sin_addr), ntohs(new_sock->addr.sin_port), strerror(errno));

    if (enc)
    {
        new_sock->ctx = init_client_ctx(); // might break things
        new_sock->ssl = SSL_new(new_sock->ctx);
        SSL_set_fd(new_sock->ssl, new_sock->fd);
        SSL_accept(new_sock->ssl);
    }
    else
    {
        new_sock->ssl = NULL;
        new_sock->ctx = NULL;
    }

    *sock = new_sock;
    return (*sock)->fd;
}

sock_t start_server(char* address, int port, sock_t *sock, FILE *log, int enc, char *cert, char *key)
{
    int opt;

    sock->ssl = NULL;

    if (enc)
    {
        SSL_library_init();
        sock->ctx = init_server_ctx();
        load_certs(sock->ctx, cert, key);
    }
    else{
        sock->ctx = NULL;
    }

    // Creating socket file descriptor
    if ((sock->fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
    {
        server_event_log_entry(log, CERR, inet_ntoa(sock->addr.sin_addr), ntohs(sock->addr.sin_port), strerror(errno));
        return *sock;
    }
    bzero(&sock->addr, sizeof(sock->addr));

    // set socket options
    if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        server_event_log_entry(log, CERR, inet_ntoa(sock->addr.sin_addr), ntohs(sock->addr.sin_port), strerror(errno));
        return *sock;
    }

    sock->addr.sin_family = AF_INET;
    inet_pton(AF_INET, address, &sock->addr.sin_addr);
    sock->addr.sin_port = htons(port);

    // Forcefully attaching socket to the port 8080
    if (bind(sock->fd, (struct sockaddr *) &sock->addr,
             sizeof(sock->addr)) < 0)
    {
        server_event_log_entry(log, CERR, inet_ntoa(sock->addr.sin_addr), ntohs(sock->addr.sin_port), strerror(errno));
        return *sock;
    }

    if (listen(sock->fd, 1) < 0)
    {
        server_event_log_entry(log, CERR, inet_ntoa(sock->addr.sin_addr), ntohs(sock->addr.sin_port), strerror(errno));
        return *sock;
    }

    sock->buffer = malloc(1024);

    server_event_log_entry(log, STRT, inet_ntoa(sock->addr.sin_addr), ntohs(sock->addr.sin_port), "SUCCESS");

    return *sock;
}

void run_server(char *address, int port, int max_client, int enc, char *cert, char *key)
{
    // arrays for storing connection information.
    int server_full = max_client == 0;
    sock_t **sock_arr = calloc(max_client, sizeof(sock_t)),
            *new_sock;
    FILE *log = fopen("chat.log", "a");

    if (log == NULL)
    {
        perror("file open");
        strerror(errno);
        return;
    }

    fd_set read_fds;
    sock_t sock = start_server(address, (uint16_t) port, &sock, log, enc, cert, key);

    // Look through file descriptors for received data or new connections
    signal(SIGINT, handle_signal);
    while (exit_received == 0)
    {
        int i, max_fd = prepare_fd_set(sock_arr, sock.fd, &read_fds, max_client);

        // wait for a fd to become active (wait indefinitely)
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            if (errno != EINTR)
                server_event_log_entry(log, CERR, inet_ntoa(sock.addr.sin_addr), ntohs(sock.addr.sin_port), strerror(errno));
            break;
        }

        // accept new connections
        if (FD_ISSET(sock.fd, &read_fds))
        {
            accept_connection(&new_sock, sock.fd, log, enc);

            // require username to be sent from client on startup
            chat_read(new_sock, sock.buffer);

            char *username = get_next_section(&sock.buffer);

            if (server_full)
            {
                chat_send(new_sock, "Server is already full.");
                shutdown_socket(new_sock);
                free(new_sock);

                client_event_log_entry(log, FULL, inet_ntoa(new_sock->addr.sin_addr),
                                       ntohs(new_sock->addr.sin_port), username);
                continue;
            }

            // determine if provided username is valid
            int valid_conn = valid_username(sock_arr, username, max_client);

            if (valid_conn == 0)
            {
                chat_send(new_sock, "Username is already in use, please try another.");
                client_event_log_entry(log, RJCT, inet_ntoa(new_sock->addr.sin_addr),
                                       ntohs(new_sock->addr.sin_port), username);
                shutdown_socket(new_sock);
                free(new_sock);
                free(username);
                continue;
            }
            new_sock->username = username;

            client_event_log_entry(log, JOIN, inet_ntoa(new_sock->addr.sin_addr),
                                   ntohs(new_sock->addr.sin_port), username);

            // look for first empty location in array.
            for (i = 0; valid_conn && i < max_client; i++)
            {
                if (! sock_arr[i]) {
                    sock_arr[i] = new_sock;
                    FD_SET(new_sock->fd, &read_fds);
                    break;
                }
            }

            // If all elements of fd_arr are full, the sever is considered full
            server_full = i >= max_client - 1 ? 1 : 0;

            continue;
        }

        // loop through all stored fds to look for those found by select
        for (i = 0; i < max_client; i++)
        {
            if (sock_arr[i] && FD_ISSET(sock_arr[i]->fd, &read_fds))
            {
                int bytes_read = chat_read(sock_arr[i], sock.buffer);

                if (bytes_read == 0)
                {
                    getpeername(sock_arr[i]->fd, (struct sockaddr*) &sock_arr[i]->addr,
                            (socklen_t*) sizeof(sock_arr[i]->addr));
                    client_event_log_entry(log, LEAV, inet_ntoa(new_sock->addr.sin_addr),
                            ntohs(new_sock->addr.sin_port), sock_arr[i]->username);

                    FD_CLR(sock_arr[i]->fd, &read_fds);
                    shutdown_socket(sock_arr[i]);
                    free(sock_arr[i]);
                    sock_arr[i] = NULL;
                    server_full = 0;
                }
                else if (bytes_read > 0)
                {
                    char* packet = sock.buffer;
                    char* dest = get_next_section(&packet);
                    packet += strlen(dest) + strlen(DELIMITER);

                    if (strcmp(dest, "LIST") == 0)
                    {
                        handle_list(sock_arr[i], sock_arr, max_client);
                    }
                    else
                    {
                        if (handle_user_to_user(sock_arr, packet, dest, max_client) == -1)
                        {
                            // no matching username was found
                            char *message, *fmt = "No such user '%s'";
                            int msg_len = strlen(fmt) + strlen(dest) - 1;

                            message = malloc(msg_len);
                            sprintf(message, fmt, dest);
                            chat_send(sock_arr[i], message);
                            free(message);
                            message = NULL;
                        }
                    }
                    free(dest);
                    dest = NULL;
                }
                else
                {
                    server_event_log_entry(log, CERR, inet_ntoa(new_sock->addr.sin_addr),
                            ntohs(new_sock->addr.sin_port), strerror(errno));
                }
            }
        }
    }

    server_event_log_entry(log, STOP, inet_ntoa(new_sock->addr.sin_addr),
                           ntohs(new_sock->addr.sin_port), "SUCCESS");

    // close server resources
    if (sock.ctx != NULL) SSL_CTX_free(sock.ctx);
    fclose(log);

    free(sock.buffer);

}

int prepare_fd_set(sock_t **sock_arr, int server_fd, fd_set *set, int max_client)
{
    int max_fd = server_fd;

    // clear and add all relevant fds to the set
    FD_ZERO(set);
    FD_SET(server_fd, set);
    for (int i = 0; i < max_client; i++)
    {
        if (sock_arr[i] == NULL) continue;

        FD_SET(sock_arr[i]->fd, set);

        if (sock_arr[i]->fd > max_fd) max_fd = sock_arr[i]->fd;
    }

    return max_fd;
}

ssize_t handle_user_to_user(sock_t **sock_arr, char *packet, char *dest, int max_client)
{
    // iterate through user_arr to find username matching dest
    for (int i = 0; i < max_client; i++)
    {
        if (! sock_arr[i]) continue;

        if (sock_arr[i]->username && strcmp(sock_arr[i]->username, dest) == 0)
        {
            return chat_send(sock_arr[i], packet);
        }
    }

    return -1;
}

void handle_list(sock_t *dest_sock, sock_t **sock_arr, int max_client)
{
    char *users;
    int len = 0;

    for (int i = 0; i < max_client; i++)
    {
        if (sock_arr[i] && sock_arr[i]->username != NULL)
        {
            len += strlen(sock_arr[i]->username) + 1;
        }
    }

    users = (char*) malloc(len);
    memset(users, 0, len);

    for (int i = 0; i < max_client; i++)
    {
        if (sock_arr[i] && sock_arr[i]->username)
        {
            strcat(users, sock_arr[i]->username);
            strcat(users, ",");
        }
    }

    users[len - 1] = '\0';

    // create packet
    char *packet;
    form_packet(&packet, "SERVER", users, NULL);

    chat_send(dest_sock, packet);

    free(users);
    free(packet);
}

int valid_username(sock_t **sock_arr, char *username, int max_client)
{
    for (int i = 0; i < max_client; i++)
    {
        if (sock_arr[i] && sock_arr[i]->username && strcmp(sock_arr[i]->username, username) == 0) return 0;
    }

    return 1;
}
