#include "chat_socket.h"

struct sock_info start_server(char* address, uint16_t port)
{
    struct sock_info s_sock;
    int opt = 1;

    // Creating socket file descriptor
    if ((s_sock.fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(s_sock.fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    s_sock.addr.sin_family = AF_INET; // TODO AF_UNIX ?
    s_sock.addr.sin_addr.s_addr = INADDR_ANY; /*
                                                 * TODO specify the address
                                                 * See client.c for inet_pton
                                                 */
    s_sock.addr.sin_port = htons( PORT ); // TODO allow for specifying port

    // Forcefully attaching socket to the port 8080
    if (bind(s_sock.fd, (struct sockaddr *) &s_sock.addr,
             sizeof(s_sock.addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return s_sock;
}

int accept_connection(struct sock_info s_sock)
{
    int client_fd,
        address_size = sizeof(s_sock.addr);

    if ((client_fd = accept(s_sock.fd,
                            (struct sockaddr *) &s_sock.addr,
                            (socklen_t*) &address_size)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return client_fd;
}

struct sock_info start_client(char* address, uint16_t port)
{
    struct sock_info c_sock;

    // create the socket
    if ((c_sock.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket failure");
        exit(EXIT_FAILURE);
    }

    // zero memory block
    bzero(&c_sock.addr, sizeof(c_sock.addr));

    c_sock.addr.sin_family = AF_INET;
    c_sock.addr.sin_port = htons(PORT); // TODO allow for specifying ports

    // TODO allow for specifying the address
    if (inet_pton(AF_INET, "127.0.0.1", &c_sock.addr.sin_addr) <= 0)
    {
        perror("Invalid / unsupported address");
        exit(EXIT_FAILURE);
    }

    return c_sock;
}

void connect_client(struct sock_info c_sock)
{
    if (connect(c_sock.fd, (struct sockaddr *) &c_sock.addr,
                sizeof(c_sock.addr)) < 0)
    {
        perror("Connection failure");
        exit(EXIT_FAILURE);
    }
}

ssize_t read_fd(int fd, char buffer[BUFFER_SIZE])
{
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read < 0)
    {
        perror("read error");
        return bytes_read;
    }

    if (buffer[bytes_read - 1] != '\0')
    {
        buffer[bytes_read] = '\0';
    }

    return bytes_read;
}

ssize_t send_fd(int fd, char* message)
{
    ssize_t result = send(fd, message, strlen(message), 0);

    if (result < 0)
    {
        perror("send error");
    }

    return result;
}

int shutdown_fd(int fd)
{
    if (shutdown(fd, 2) < 0)
    {
        perror("Shutdown failure");
        return -1;
    }

    return 0;
}