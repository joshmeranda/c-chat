/**
 * Main file for the c_chat program.
 *
 * Parses command line arguments, creates and connects chat servers and
 * clients.
 */

#include "chat_socket.h"
#include "client_shell.h"

#include <error.h>
#include <getopt.h>

#define MAX_CLIENT 10

/**
 * enum used to specify the program should be run as a server of a client.
 */
enum Mode { SERVER, CLIENT };

/**
 * Create a server.
 *
 * params
 *     address (char*): the IP address for the server.
 *     port (int): the port used by the server.
 */
void run_server(char* address, int port)
{
    // server socket information
    struct sock_info s_sock = start_server(address, (uint16_t) port);

    int client_fd_arr[MAX_CLIENT];
    fd_set read_fds;

    // Set client_fd_arr and read_fds to be emtpy (all values are 0)
    for (int i = 0;i < MAX_CLIENT; i++) { client_fd_arr[i] = 0; }

    if (listen(s_sock.fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

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
                int bytes_read;

                if ((bytes_read = read_fd(client_fd_arr[i], s_sock.buffer)) == 0)
                {
                    printf("Client [a] %s [p] %d closed the connection\n",
                            inet_ntoa(s_sock.addr.sin_addr),
                            ntohs(s_sock.addr.sin_port));
                    FD_CLR(client_fd_arr[i], &read_fds);
                    client_fd_arr[i] = 0;
                }
                else
                {
                    send_fd(client_fd_arr[i], s_sock.buffer);
                }
            }
        }
    }
}

/**
 * Create a client.
 *
 * Runs a very simple client interface prompting for user messages to send
 * to the server.
 *
 * params
 *     address (char*): the IP of the target server.
 *     port (int): the port used by the target server.
 */
void run_client(char* address, int port)
{
    struct sock_info c_sock = start_client(address, (uint16_t) port);
    int command,
        connected = 0;

    // resad user input until they enter '.exit'
    while (1)
    {
        printf("%s", get_prompt());
        fgets(c_sock.buffer, BUFFER_SIZE, stdin);

        set_command(c_sock.buffer, &command);

        // execute commands
        if (command == SEND)
        {
            if (! connected)
            {
                printf("You are not connected to anyone.\n");
                continue;
            }

            send_fd(c_sock.fd, c_sock.buffer);
        }
        else if (command == EXIT) // close connection, leave shell loop
        {
            shutdown_fd(c_sock.fd);
            break;
        }
        else if (command == CONNECT)
        {
            if (connected)
            {
                printf("You are already connected\n");
                continue;
            }
            connect_client(c_sock);
            connected = 1;
            continue;
        }
        else if (command == DISCONNECT) // close connection, stay in shell loop
        {
            shutdown_fd(c_sock.fd);
            connected = 0;
            continue;
        }
        else
        {
            printf("unrecognized command %s\n", c_sock.buffer);
            continue;
        }

        read_fd(c_sock.fd, c_sock.buffer);
    }
}

int main(int argc, char** argv)
{
    enum Mode mode;
    char* address = "127.0.0.1";
    int port = 8080,
        opt_index = 1;
    char arg;

    struct option long_options[] = {
            {"address", required_argument, 0, 'a'},
            {"port",    required_argument, 0, 'p'},
            {0,         0,                 0, 0}
    };

    while ((arg = getopt_long(argc, argv, "a:p:",
                              long_options, &opt_index)) > 0)
    {
        switch (arg)
        {
            case 'a':
                address = optarg;
                break;
            case 'p':
                port = atoi(optarg);
        }
    }

    for (opt_index; opt_index < argc; opt_index++)
    {
        if (strcmp(argv[opt_index], "client") == 0) { mode = CLIENT; }
        else if (strcmp(argv[opt_index], "server") == 0) { mode = SERVER; }
        else { error(0, 0, "unknown option '%s'\n", argv[opt_index]); }
    }

    if (mode == SERVER)
    {
        run_server(address, port);
    }
    else if (mode == CLIENT)
    {
        run_client(address, port);
    }
}