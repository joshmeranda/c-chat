#include "server.h"
#include "enc_chat_socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int accept_connection(SOCK sock, int enc)
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

void ShowServerCerts(SSL* ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
}

void run_server(char *address, int port, int enc, char *cert, char *key)
{
    // arrays for storing connection information.
    int client_fd_arr[MAX_CLIENT];
    SSL* client_ssl_arr[MAX_CLIENT];
    char* user_arr[MAX_CLIENT];

    fd_set read_fds;
    SOCK sock;

    // set all array contents to 0
    memset(client_fd_arr, 0, MAX_CLIENT * sizeof(int));
    memset(client_ssl_arr, 0, MAX_CLIENT * sizeof(SSL*));
    memset(user_arr, 0, MAX_CLIENT * sizeof(char*));

    sock = start_server(address, (uint16_t) port, enc, cert, key);

    // Look through file descriptors for received data or new connections
    while (1)
    {
        int max_fd = prepare_fd_set(client_fd_arr, &read_fds, sock.fd);

        // wait for a fd to become active (wait indefinitely)
        int active_fds = select(max_fd +1, &read_fds, NULL, NULL, NULL);
        if (active_fds < 0)
        {
            perror("select failure");
            exit(EXIT_FAILURE);
        }


        // accept new connections
        if (FD_ISSET(sock.fd, &read_fds))
        {
            int new_fd = accept_connection(sock, enc);
            SSL *ssl = NULL;
            if (enc)
            {
                ssl = SSL_new(sock.ctx);
                SSL_set_fd(ssl, new_fd);
                SSL_accept(ssl);
            }

            // look for first empty location in array.
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (client_fd_arr[i] == 0)
                {
                    client_fd_arr[i] = new_fd;
                    if (ssl != NULL) client_ssl_arr[i] = ssl;
                    FD_SET(new_fd, &read_fds);
                    break;
                }
            }
        }


        // loop through all stored fds to look for those found by select
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            if (FD_ISSET(client_fd_arr[i], &read_fds))
            {
                int bytes_read = -1;

                if (enc)
                {
                    bytes_read = read_ssl(client_ssl_arr[i], sock.buffer);
                }
                else
                {
                    bytes_read = read_fd(client_fd_arr[i], sock.buffer);
                }

                // todo handle read_bytes == -1
                if (bytes_read == 0)
                {
                    printf("Client [a] %s [p] %d closed the connection\n",
                           inet_ntoa(sock.addr.sin_addr),
                           ntohs(sock.addr.sin_port));
                    if (enc)
                    {
                        SSL_free(client_ssl_arr[i]);
                        client_ssl_arr[i] = NULL;
                    }
                    FD_CLR(client_fd_arr[i], &read_fds);
                    client_fd_arr[i] = 0;
                    free(user_arr[i]);
                    user_arr[i] = NULL;
                }
                else
                {
                    char* packet = sock.buffer;
                    int dest_len = strcspn(packet, DELIMITER) + 1;
                    char* dest = (char*) malloc(dest_len);

                    strncpy(dest, packet, dest_len);
                    dest[dest_len - 1] = '\0';
                    packet = &packet[dest_len];

                    if (strcmp(dest, "USERNAME") == 0)
                    {
                        handle_new_user(&user_arr[i], client_fd_arr[i], packet);
                    }
                    else if (strcmp(dest, "LIST") == 0)
                    {
                        handle_list(client_fd_arr[i], user_arr);
                    }
                    else
                    {
                        handle_user_to_user(client_fd_arr, user_arr, packet, dest);
                    }

                    free(dest);
                }
            }
        }
    }

    // close server resources
    shutdown_fd(sock.fd);
    if (sock.ctx) SSL_CTX_free(sock.ctx);
}

int prepare_fd_set(int *fd_arr, fd_set *set, int sock_fd)
{
    int max_fd = sock_fd;

    // clear and add all relevant fds to the set
    FD_ZERO(set);
    FD_SET(sock_fd, set);
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        FD_SET(fd_arr[i], set);

        if (fd_arr[i] > max_fd) max_fd = fd_arr[i];
    }

    return max_fd;
}

void handle_user_to_user(int *fd_arr, char **user_arr, char *packet, char *dest)
{
    // iterate through user_arr to find username matching dest
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (user_arr[i] == NULL) {
            continue;
        }
        else if (strcmp(user_arr[i], dest) == 0)
        {
            send_fd(fd_arr[i], packet);
            break;
        }
    }
}

void handle_new_user(char **username, int fd, char *packet)
{
    int dest_len = strcspn(packet, DELIMITER) + 1;
    *username = (char*) malloc(dest_len);
    strncpy(*username, packet, dest_len); // strncpy might break shit
    (*username)[dest_len - 1] = '\0';

    // Reply to new user with welcome message
    char* new_user_msg = strdup("Welcome to my server!!!");
    send_fd(fd, new_user_msg);
    free(new_user_msg);
}

void handle_list(int fd, char **user_arr)
{
    char *users;
    int len = 0;

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (user_arr[i] != NULL)
        {
            len += strlen(user_arr[i]) + 1;
        }
    }

    users = (char*) malloc(len);

    for (int i = 0; i < MAX_CLIENT; i++)
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

    send_fd(fd, packet);

    free(users);
    free(packet);
}