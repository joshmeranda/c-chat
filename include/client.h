#ifndef CLIENT_SHELL_H
#define CLIENT_SHELL_H

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
SOCK start_client(char* address, uint16_t port, int enc);

/**
 * Connect a client to a server.
 *
 * @param sock The sock_info of the client.
 * @param enc Connect client to server with ssl.
 */
void connect_client(SOCK *sock, int enc);

/**
 * Generate the prompt for user input.
 *
 * @return The prompt for the user.
 */
void prompt(char *username);

/**
 * Form the packet to send to the server. The argument list must end in NULL.
 *
 * @param packet Pointer to the packet string.
 * @param ... The values for the sections of tha packet.
 * @return
 */
char* form_packet(char **packet, ...);

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
 * @param fd The file descriptor to close.
 * @param th The thread ID to close.
 */
void disconnect_client(int fd, pthread_t th);

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
ssize_t client_send(SOCK *sock, char* dest, char* src, char* msg, int enc);

/**
 * Get the next word in a string, meaning there is a ' ' or '\n' ending the
 * string.
 *
 * @param input The string to parse.
 * @return Pointer to the null terminated word, needs to be freed after use.
 */
char *get_next_word(char *input);

/**
 * Continually read incoming data from the connected socket.
 *
 * @param sock The socket from which to read data.
 * @return Returns null.
 */
void *client_read(void *sock);

#endif // CLIENT_SHELL_H