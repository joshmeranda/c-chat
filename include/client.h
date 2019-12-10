#ifndef CLIENT_H
#define CLIENT_H

#include "chat_socket.h"
#include <openssl/ssl.h>

/**
 * Create an IPv4 TCP socket for the client.
 *
 * @param address Address of the server to connect to.
 * @param port The port of the server to connect to.
 * @param env Establish am ssl enabled socket.
 * @return The sock_info describing the client socket.
 */
sock_t start_client(char* address, uint16_t port, int enc);

/**
 * Connect a client to a server.
 *
 * @param sock The sock_info of the client.
 * @param enc Connect client to server with ssl.
 */
void connect_client(sock_t *sock, int enc);

/**
 * Generate the prompt for user input.
 *
 * @return The prompt for the user.
 */
void prompt(char *username);

/**
 * Create and run a chat client.
 *
 * @param address The address of the server to connect to.
 * @param port The port of the server to connect to.
 * @param username The username to be used during the session.
 * @param enc Whether the client should operate with encryption or not (0 or 1).
 */
void run_client(char* address, int port, char* username, int enc);

/**
 * Disconnect the client and end the thread reading input from the socket.
 *
 * @param sock The client socket to be closed.
 * @param th The thread ID to close.
 */
void disconnect_client(sock_t *sock, pthread_t th);

/**
 * Form and send a packet to the server.
 *
 * @param sock The socket from which data should be sent.
 * @param dest The destination username to send the packet to.
 * @param src The source username of the packet.
 * @param msg The message to send.
 * @param enc Whether the message should be encrypted or not.
 * @return The amount of bytes sent over the server.
 */
ssize_t client_send(sock_t *sock, char* dest, char* src, char* msg, int enc);

/**
 * Continually read incoming data from the connected socket.
 *
 * @param sock The socket from which to read data.
 * @return Returns null.
 */
void *client_read(void *sock);

#endif // CLIENT_H