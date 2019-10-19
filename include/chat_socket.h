#ifndef CHAT_SOCKET_H
#define CHAT_SOCKET_H

#define USERNAME_MAX 25
#define BUFFER_SIZE USERNAME_MAX * 2 + 3 // enough to span <src>|<dest>|<256 chars>|
#define DELIMITER "|"

#include <netinet/in.h>

/**
 * Describes a connected socket.
 */
typedef struct sock_info sock_info;

struct sock_info
{
    struct sockaddr_in addr;
    int fd; // file descriptor
    char buffer[BUFFER_SIZE];
};

/**
 * Create an IPv4 TCP socket for the server.
 *
 * @param address The address for the server.
 * @param port The port for the server.
 * @return The sock_info describing the server socket.
 */
struct sock_info start_server(char* address, uint16_t port);

/**
 * Listen for and accept client connections.
 *
 * @param sock Socket to use whe creating the client file descriptor.
 * @return The file descriptor for the client.
 */
int accept_connection(struct sock_info sock);

/**
 * Create an IPv4 TCP socket for the client.
 *
 * @param address Address of the server to connect to.
 * @param port The port of the server to connect to.
 * @return The sock_info describing the client socket.
 */
struct sock_info start_client(char* address, uint16_t port);

/**
 * Connect a client to a server.
 *
 * @param sock The sock_info of the client.
 */
void connect_client(struct sock_info sock);

/**
 * Read data from a file descriptor.
 *
 * @param fd The file descriptor to read dat from.
 * @param buffer The buffer to store the data in.
 * @return The amount of characters read into the buffer, or -1 on error.
 */
ssize_t read_fd(int fd, char *buffer);

/**
 * Write data to a file descriptor.
 *
 * @param fd The file descriptor to write to.
 * @param message The data to write to the file descriptor.
 * @return The amount of data written to the file descriptor, or -1 on error.
 */
ssize_t send_fd(int fd, char* message);

/**
 * Shutdown a file descriptor.
 *
 * @param fd The file descriptor to shut down.
 * @return 0 if successful, -1 if failure.
 */
int shutdown_fd(int fd);

#endif // CHAT_SOCKET_H