/**
 * Main file for the c_chat program.
 *
 * Parses command line arguments, creates and connects chat servers and
 * clients.
 */

#include "chat_socket.h"

/**
 * enum used to specify the program should be run as a server of a client.
 */
enum Mode { SERVER, CLIENT };

/**
 * Read data from a file descriptor.
 *
 * params
 *     fd (int): the file descriptor to read from.
 *     buffer (char*): the character array used to store the data read.
 *
 * returns
 *     (ssize_t) if successful the amount of bytes read, -1 otherwise.
 */
ssize_t read_fd(int fd, char buffer[BUFFER_SIZE])
{
    ssize_t result = read(fd, buffer, BUFFER_SIZE);
    if (result < 0)
    {
        perror("read error");
        exit(EXIT_FAILURE);
    }

    return result;
}

/**
 * Write data to a files descriptor.
 *
 * params
 *     fd (int): the file descriptor to write to
 *     message (char*): the data to write to fd
 *
 * returns
 *     (ssize_t) if successful the amoutn of bytes written, -1 otherwise.
 */
ssize_t send_fd(int fd, char* message)
{
    ssize_t result = send(fd, message, strlen(message), 0);

    if (result < 0)
    {
        perror("send error");
        exit(EXIT_FAILURE);
    }

    return result;
}

/**
 * Create a server.
 *
 * params
 *     address (char*): the IP address for the server.
 *     port (int): the port used by the server.
 */
void run_server(char* address, int port)
{
    // server socket
    struct sock_info s_sock = start_server(address, (uint16_t) port);

    if (listen(s_sock.fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int client_fd = accept_connection(s_sock);

    read_fd(client_fd, s_sock.buffer);
    printf("%s\n", s_sock.buffer);

    char* rtn_message = "HELLO FROM SERVER";
    send_fd(client_fd, rtn_message);
    printf("Server sent\n");
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

    char* snd_message = "HELLO FROM CLIENT";

    send_fd(c_sock.fd, snd_message);
    printf("Client sent\n");

    read_fd(c_sock.fd, c_sock.buffer);
    printf("%s\n", c_sock.buffer);
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