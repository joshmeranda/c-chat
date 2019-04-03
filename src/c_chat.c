/**
 * Main file for the c_chat program.
 *
 * Parses command line arguments, creates and connects chat servers and
 * clients.
 */

#include "chat_socket.h"
#include "client_shell.h"

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
    FD_ZERO(&read_fds);

    FD_SET(s_sock.fd, &read_fds);
    int max_fd = s_sock.fd;

    if (listen(s_sock.fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
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
                    printf("Client [a] %s [p] %d closed the connection",
                            inet_ntoa(s_sock.addr.sin_addr),
                            ntohs(s_sock.addr.sin_port));
                }
                else
                {
                    printf("%d bytes read\n%s\n", bytes_read, s_sock.buffer);
                    send_fd(client_fd_arr[i], s_sock.buffer);
                    printf("Server sent\n");
                }
            }
        }
    }
}

/**
 * Create a client.
 *
 * params
 *     address (char*): the IP of the target server.
 *     port (int)L the port used by the target server.
 */
void run_client(char* address, int port)
{
    struct sock_info c_sock = start_client(address, (uint16_t) port);

    connect_client(c_sock);

    //char* snd_message = "josh";
    client_shell(&c_sock);

    //send_fd(c_sock.fd, snd_message);
    send_fd(c_sock.fd, c_sock.buffer);
    printf("Client sent\n");

    read_fd(c_sock.fd, c_sock.buffer);
    printf("%s\n", c_sock.buffer);

    shutdown_fd(c_sock.fd);
}

int main(int argc, char** argv)
{
    enum Mode mode;
    char* address = "127.0.0.1";
    int port = 8080;

    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--address") == 0)
        {
            address = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0)
        {
            port = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "server") == 0)
        {
            mode = SERVER;
        }
        else if (strcmp(argv[i], "client") == 0)
        {
            mode = CLIENT;
        }
        else
        {
            mode = -1;
        }
    }

    if (mode == SERVER)
    {
        run_server(address, port);
    }
    else if (mode == CLIENT)
    {
        run_client(address, port);
    }
    else if (mode == -1)
    {
        printf("Could not read argument for mode");
        exit(1);
    }
}