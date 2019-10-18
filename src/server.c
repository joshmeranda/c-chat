#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

void broadcast(char *message, int *client_fd_arr)
{
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (client_fd_arr[i] != 0) { send_fd(client_fd_arr[i], message); }
    }
}

void run_server(char *address, int port)
{
    // server socket information
    struct sock_info s_sock = start_server(address, (uint16_t) port);

    printf("Server [a] %s [p] %d open\n",
           inet_ntoa(s_sock.addr.sin_addr),
           ntohs(s_sock.addr.sin_port));

    int client_fd_arr[MAX_CLIENT];
    char* user_arr[MAX_CLIENT];
    memset(user_arr, 0, MAX_CLIENT * sizeof(char*));

    fd_set read_fds;

    // Set client_fd_arr and read_fds to be emtpy (all values are 0)
    for (int i = 0;i < MAX_CLIENT; i++) { client_fd_arr[i] = 0; }

    if (listen(s_sock.fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Look through file descriptors for received data or new connections
    while (1)
    {
        int max_fd = prepare_fd_set(client_fd_arr, &read_fds, s_sock.fd);

        // wait for a fd to become active (wait indefinitely)
        int active_fds = select(max_fd +1, &read_fds, NULL, NULL, NULL);
        if (active_fds < 0)
        {
            perror("select failure");
            exit(EXIT_FAILURE);
        }


        // accept new connections
        if (FD_ISSET(s_sock.fd, &read_fds))
        {
            int new_fd = accept_connection(s_sock);
            FD_SET(new_fd, &read_fds);
            printf("Client [a] %s [p] %d connected\n",
                   inet_ntoa(s_sock.addr.sin_addr),
                   ntohs(s_sock.addr.sin_port));

            // look for first empty location in array.
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (client_fd_arr[i] == 0)
                {
                    client_fd_arr[i] = new_fd;
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
                if (read_fd(client_fd_arr[i], s_sock.buffer) == 0)
                {
                    printf("Client [a] %s [p] %d closed the connection\n",
                           inet_ntoa(s_sock.addr.sin_addr),
                           ntohs(s_sock.addr.sin_port));
                    FD_CLR(client_fd_arr[i], &read_fds);
                    client_fd_arr[i] = 0;
                    free(user_arr[i]);
                    user_arr[i] = NULL;
                }
                else
                {
                    char* packet = s_sock.buffer;
                    int dest_len = strcspn(packet, DELIMITER) + 1;
                    char* dest = (char*) malloc(dest_len);

                    strncpy(dest, packet, dest_len);
                    dest[dest_len - 1] = '\0';
                    packet = &packet[dest_len];

                    if (strcmp(dest, "USERNAME") == 0)
                    {
                        handle_new_user(&user_arr[i], client_fd_arr[i], packet);
                    }
                    else if (strcmp(dest, "BRD") == 0)
                    {
                        broadcast(packet, client_fd_arr);
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
    strcpy(*username, packet); // strncpy might break shit
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