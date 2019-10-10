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

static char* command_arr[] =
{
    ".send",
    ".exit",
    ".help",
    ".connect",
    ".disconnect",
    ".broadcast",
    ".dest",
    ".list",
    "\0"
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
 * @param to The username of the desired recipient.
 * @param from The username of the sender.
 * @param message The message to be sent in the packet.
 * @param packet The pointer to the final packet.
 * @return The pointer to the resultant packet.
 */
char* form_packet(char* to, char* from, char* message, char* packet);

/**
 * Determine if there is data to be read from the server.
 *
 * @param fd The file descriptor for the server.
 * @return The amount of bytes available to be read, or -1 if error
 */
int server_data(int fd);

/**
 * Crease and run a chat client.
 *
 * @param address The address of the server to connect to.
 * @param port The port of the server to connect to.
 * @param username The username to be used during the session.
 */
void run_client(char* address, int port, char* username);

#endif // CLIENT_SHELL_H