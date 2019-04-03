/**
 * Header file for chat_sockets.
 *
 * Defines functions used to create and connect chat servers and
 * clients.s
 */

#ifndef CHAT_SOCKET_H
#define CHAT_SOCKET_H
#define PORT 8080
#define BUFFER_SIZE 1024

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/**
 * Wrapper struct for sockaddr_in and a file descriptor.
 *
 * members
 *     addr (sockaddr_in): socket addresss information.
 *     fd (int): file descriptor for the socket.
 *     buffer (char*): buffer for reading and writing to sockets
 */
struct sock_info
{
    struct sockaddr_in addr;
    int fd; // file descriptor
    char buffer[BUFFER_SIZE];
};

/* . . . . . . . . . */
/* Server Functions  */
/* . . . . . . . . . */

/**
 * Create the socket used by the server.
 *
 * Currently set to SOCK_STREAM and IPv4.
 *
 * params
 *     address (char*): the address used during socket creation.
 *     port (int): the port used during socket creation.
 *
 * returns
 *     (sock_info): contains information needed by other server functions.
 */
struct sock_info start_server(char* address, uint16_t port);

/**
 * Start listening for and accepting client connections.
 *
 * params
 *     sock {sock_info)): socket used to generate client file descriptor.
 *
 * returns
 *     (int): the file descriptor for the client.
 */
int accept_connection(struct sock_info sock);


/* . . . . . . . . . */
/* Client Functions  */
/* . . . . . . . . . */

/**
 * Create the socket used by the client.
 *
 * Currently set to SOCK_STREAM an IPv4
 *
 * params
 *     address (char*): address of the server to connect to
 *     port (int): the port used by the server to connect to
 *
 *
 */
struct sock_info start_client(char* address, uint16_t port);

void connect_client(struct sock_info sock);

/* . . . . . . */
/* Unspecific  */
/* . . . . . . */

ssize_t read_fd(int fd, char buffer[BUFFER_SIZE]);

ssize_t send_fd(int fd, char* message);

/**
 * Shutdown the file descriptor.
 *
 * Disables all send and receive operations for the specified file descriptor.
 *
 * params
 *     fd (int): the file descriptor to shut down.
 *
 * returns
 *     (int): 0 if successful, otherwise exits with errno.
 */
int shutdown_fd(int fd);

#endif // CHAT_SOCKET_H