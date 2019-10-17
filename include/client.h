#ifndef CLIENT_SHELL_H
#define CLIENT_SHELL_H

#include "chat_socket.h"
#include <stdio.h>

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
char* get_prompt();

/**
 * Set the value of the command to the corresponding value in the
 * command_types enum. The input is compared to the string values in
 * command_arr, and command is set to be the corresponding values found in
 * command_types.
 *
 * @param input The user input to test.
 */
void set_command(char* input, int* command);

/**
 * Form the message to be sent to the server. Packet joins arguments into the
 * packet arg delimited by the null byte.
 *
 * @param dest The username of the desired recipient.
 * @param src The username of the sender.
 * @param message The message to be sent in the packet.
 * @param packet The pointer to the final packet.
 * @return The pointer to the resultant packet.
 */
char* form_packet(char *to, char *from, char *message, char *packet);

/**
 * Create and run a chat client.
 *
 * @param address The address of the server to connect to.
 * @param port The port of the server to connect to.
 * @param username The username to be used during the session.
 */
void run_client(char* address, int port, char* username);

/**
 * Continually read incoming data from the connected socket.
 *
 * @param sock The socket from which to read data.
 * @return Returns null.
 */
void *client_read(void *c_sock);

void disconnect_client(int fd, pthread_t t_th);

ssize_t client_send(int fd, char* dest, char* src, char* msg);

#endif // CLIENT_SHELL_H