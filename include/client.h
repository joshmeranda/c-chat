#ifndef CLIENT_SHELL_H
#define CLIENT_SHELL_H

#include "chat_socket.h"

enum command_types
{
    SEND,
    EXIT,
    HELP,
    CONNECT,
    DISCONNECT,
    BROADCAST,
    DESTINATION,
    LIST
};

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
 */
void run_client(char* address, int port, char* username);

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
 * @param fd The file descriptor associated with the socket.
 * @param dest The destination username to send the packet to.
 * @param src The source username of the packet.
 * @param msg The message to send.
 * @return The amount of bytes sent over the server.
 */
ssize_t client_send(int fd, char* dest, char* src, char* msg);

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
void *client_read(void *c_sock);

#endif // CLIENT_SHELL_H