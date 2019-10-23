#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int accept_connection(SOCK sock)
{
    int client_fd, address_size = sizeof(sock.addr);

    if ((client_fd = accept(sock.fd,
                            (struct sockaddr *) &sock.addr,
                            (socklen_t*) &address_size)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Client [a] %s [p] %d connected\n",
           inet_ntoa(sock.addr.sin_addr),
           ntohs(sock.addr.sin_port));

    return client_fd;
}

SOCK start_server(char* address, uint16_t port, int enc, char *cert, char *key)
{
    SOCK sock;
    int opt;

    if (enc)
    {
        SSL_library_init();
        sock.ctx = init_server_ctx();
        load_certs(sock.ctx, cert, key);
    }

    // Creating socket file descriptor
    if ((sock.fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    bzero(&sock.addr, sizeof(sock.addr));

    // set socket options
    if (setsockopt(sock.fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    sock.addr.sin_family = AF_INET;
    inet_pton(AF_INET, address, &sock.addr.sin_addr);
    sock.addr.sin_port = htons(port);

    // Forcefully attaching socket to the port 8080
    if (bind(sock.fd, (struct sockaddr *) &sock.addr,
             sizeof(sock.addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock.fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server [a] %s [p] %d open\n",
           inet_ntoa(sock.addr.sin_addr),
           ntohs(sock.addr.sin_port));

    return sock;
}

void run_server(char *address, int port, int max_client, int enc, char *cert, char *key)
{
    // arrays for storing connection information.
    int *fd_arr = calloc(max_client, sizeof(int)),
        server_full = (max_client < 1)? 1 : 0;
    SSL **ssl_arr = calloc(max_client, sizeof(SSL*));
    char **user_arr = calloc(max_client, sizeof(char*));

    fd_set read_fds;
    SOCK sock;

    // set all array contents to 0
    memset(fd_arr, 0, max_client * sizeof(int));
    memset(ssl_arr, 0, max_client * sizeof(SSL*));
    memset(user_arr, 0, max_client * sizeof(char*));

    sock = start_server(address, (uint16_t) port, enc, cert, key);

    // Look through file descriptors for received data or new connections
    while (1)
    {
        int i, max_fd = prepare_fd_set(fd_arr, &read_fds, sock.fd, max_client);

        // wait for a fd to become active (wait indefinitely)
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            perror("select failure");
            exit(EXIT_FAILURE);
        }

        // accept new connections
        if (FD_ISSET(sock.fd, &read_fds))
        {
            int new_fd = accept_connection(sock);
            SSL *ssl = NULL;

            if (enc)
            {
                ssl = SSL_new(sock.ctx);
                SSL_set_fd(ssl, new_fd);
                SSL_accept(ssl);
            }

            if (server_full)
            {
                chat_send(new_fd, ssl, enc, "Server is already full.");
                if (enc) SSL_free(ssl);
                shutdown_fd(new_fd);
                printf("Client [a] %s [p] %d closed the connection\n",
                       inet_ntoa(sock.addr.sin_addr),
                       ntohs(sock.addr.sin_port));
                continue;
            }

            // require username to be sent from client on startup
            chat_read(new_fd, ssl, enc, sock.buffer);

            // determine if provided username is valid
            char *username = get_next_word(sock.buffer, DELIMITER);
            int valid_conn = valid_username(user_arr, username, max_client);

            if (valid_conn == 0)
            {
                chat_send(new_fd, ssl, enc, "Username is already in use, please try another.");
                if (enc) SSL_free(ssl);
                shutdown_fd(new_fd);
                free(username);
                printf("Client [a] %s [p] %d closed the connection\n",
                       inet_ntoa(sock.addr.sin_addr),
                       ntohs(sock.addr.sin_port));
            }

            // look for first empty location in array.
            for (i = 0; valid_conn && i < max_client; i++)
            {
                if (fd_arr[i] == 0)
                {
                    fd_arr[i] = new_fd;
                    if (ssl != NULL) ssl_arr[i] = ssl;
                    user_arr[i] = username;
                    FD_SET(new_fd, &read_fds);
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
            if (FD_ISSET(fd_arr[i], &read_fds))
            {
                int bytes_read = chat_read(fd_arr[i], ssl_arr[i], enc, sock.buffer);

                // todo handle read_bytes == -1
                if (bytes_read == 0)
                {
                    getpeername(fd_arr[i], (struct sockaddr*) &sock.addr, (socklen_t*) sizeof(sock.addr));
                    printf("Client [a] %s [p] %d closed the connection\n",
                           inet_ntoa(sock.addr.sin_addr),
                           ntohs(sock.addr.sin_port));
                    if (enc)
                    {
                        SSL_free(ssl_arr[i]);
                        ssl_arr[i] = NULL;
                    }
                    FD_CLR(fd_arr[i], &read_fds);
                    shutdown_fd(fd_arr[i]);
                    fd_arr[i] = 0;
                    free(user_arr[i]);
                    user_arr[i] = NULL;
                    server_full = 0;
                }
                else
                {
                    char* packet = sock.buffer;
                    char* dest = get_next_word(packet, DELIMITER);
                    packet = &packet[strlen(dest)];

                    if (strcmp(dest, "LIST") == 0)
                    {
                        handle_list(fd_arr[i], ssl_arr[i], user_arr, enc, max_client);
                    }
                    else
                    {
                        if (handle_user_to_user(fd_arr, user_arr, ssl_arr, packet, dest, enc, max_client) == -1)
                        {
                            // no matching username was found
                            char *message, *fmt = "No such user '%s'";
                            int msg_len = strlen(fmt) + strlen(dest) - 1;

                            message = malloc(msg_len);
                            sprintf(message, fmt, dest);
                            chat_send(fd_arr[i], ssl_arr[i], enc, message);
                            free(message);
                        }
                    }
                    free(dest);
                }
            }
        }
    }

    // close server resources
    shutdown_fd(sock.fd);
    if (sock.ctx) SSL_CTX_free(sock.ctx);
    free(fd_arr);
    free(ssl_arr);
    free(user_arr);
}

int prepare_fd_set(int *fd_arr, fd_set *set, int sock_fd, int max_client)
{
    int max_fd = sock_fd;

    // clear and add all relevant fds to the set
    FD_ZERO(set);
    FD_SET(sock_fd, set);
    for (int i = 0; i < max_client; i++)
    {
        FD_SET(fd_arr[i], set);

        if (fd_arr[i] > max_fd) max_fd = fd_arr[i];
    }

    return max_fd;
}

ssize_t handle_user_to_user(int *fd_arr, char **user_arr, SSL **ssl_arr, char *packet, char *dest, int enc, int max_client)
{
    // iterate through user_arr to find username matching dest
    for (int i = 0; i < max_client; i++)
    {
        if (user_arr[i] != NULL && strcmp(user_arr[i], dest) == 0)
        {
            return chat_send(fd_arr[i], ssl_arr[i], enc, packet);
        }
    }

    return -1;
}

void handle_list(int fd, SSL *ssl, char **user_arr, int enc, int max_client)
{
    char *users;
    int len = 0;

    for (int i = 0; i < max_client; i++)
    {
        if (user_arr[i] != NULL)
        {
            len += strlen(user_arr[i]) + 1;
        }
    }

    users = (char*) malloc(len);

    for (int i = 0; i < max_client; i++)
    {
        if (user_arr[i] != NULL)
        {
            strcat(users, user_arr[i]);
            strcat(users, ",");
        }
    }

    users[len - 1] = '\0';
    printf("users : %s\n", users);

    // create packet
    char *packet, *src = "SERVER";
    len = strlen(src) + len + 2;
    packet = (char*) malloc(len);

    strcat(packet, src);
    strcat(packet, DELIMITER);
    strcat(packet, users);
    strcat(packet, DELIMITER);

    chat_send(fd, ssl, enc, packet);

    free(users);
    free(packet);
}

int valid_username(char **user_arr, char *username, int max_client)
{
    for (int i = 0; i < max_client; i++)
    {
        if (user_arr[i] == NULL) continue;

        if (strcmp(user_arr[i], username) == 0) return 0;
    }

    return 1;
}
