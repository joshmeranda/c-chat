#include "server.h"

void broadcast(char* message, int* client_fd_arr)
{
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (client_fd_arr[i] != 0) { send_fd(client_fd_arr[i], message); }
    }
}

void run_server(char* address, int port)
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

    // Look through file descriptors for received data
    while (1)
    {
        FD_ZERO(&read_fds);

        FD_SET(s_sock.fd, &read_fds);
        int max_fd = s_sock.fd;

        // add child sockets to fd set
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            if (client_fd_arr[i] > 0) {
                FD_SET(client_fd_arr[i], &read_fds);

                if (client_fd_arr[i] > max_fd) { max_fd = client_fd_arr[i]; }
            }
        }

        // wait for a fd to become active (wait indefinitely)
        int active_fds = select(max_fd +1, &read_fds, NULL, NULL, NULL);
        if (active_fds < 0)
        {
            perror("select failure");
            exit(EXIT_FAILURE);
        }


        if (FD_ISSET(s_sock.fd, &read_fds))
        {
            int new_fd = accept_connection(s_sock);
            FD_SET(new_fd, &read_fds);
            printf("Client [a] %s [p] %d connected\n",
                   inet_ntoa(s_sock.addr.sin_addr),
                   ntohs(s_sock.addr.sin_port));

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
                        // store the remainder of packet as the username
                        dest_len = strcspn(packet, DELIMITER) + 1;
                        user_arr[i] = (char*) malloc(dest_len);
                        strcpy(user_arr[i], packet);
                        user_arr[i][dest_len - 1] = '\0';

                        printf("New user '%s'\n", user_arr[i]);
                    }
                    else if (strcmp(dest, "BRD") == 0)
                    {
                        broadcast(packet, client_fd_arr);
                    }
                    else
                    {
                        for (int j = 0; j < MAX_CLIENT; j++) {
                            if (strcmp(user_arr[j], dest) == 0) {
                                send_fd(client_fd_arr[j], packet);
                                break;
                            }
                        }

                        printf("  dest : %s\npacket : %s\n", dest, packet);
                    }

                    free(dest);
                }
            }
        }
    }
}
