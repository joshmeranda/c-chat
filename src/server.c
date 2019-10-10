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

    int client_fd_arr[MAX_CLIENT];
    char* user_arr[MAX_CLIENT];

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
                }
                else
                {
                    // Get the packet destination
                    char* packet = s_sock.buffer;
                    int dest_len = strcspn(packet, DELIMITER);
                    char dest[USERNAME_MAX];
                    memset(dest, 0, USERNAME_MAX);

                    strncpy(dest, packet, dest_len);
                    packet = &packet[dest_len + 1];

                    if (strcmp(dest, "USERNAME") == 0)
                    {
                        printf("New user '%s'", dest);
                    }
                    else if (strcmp(dest, "BRD") == 0)
                    {
                        broadcast(packet, client_fd_arr);
                    }
                    else
                    {
                        // TODO map username to file descriptor
                        printf("  dest : %s\n"
                               "packet : %s\n", dest, packet);
                    }
                    // send_fd(client_fd_arr[i], packet);
                }
            }
        }
    }
}
