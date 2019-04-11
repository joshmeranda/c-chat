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
 * Get the prompt for the user.
 *
 * returns
 *     (char*): the prompt for the user.
 */
char* get_prompt();

/**
 * Set the value of command form user input.
 *
 * All commands begin with a single '.' character. If no command is
 * entered by the user, the command SEND is assumed and returned.
 *
 * params
 *     input (char*): the user input to test.
 */
void set_command(char* input, int* i);

/**
 * Form the message packet to send to the server.
 *
 * Packet is in the format '<to>\0<from>\0<data>\0'
 *
 * params
 *     to (char*): the intended recipient.
 *     from (char*): the message sender.
 *     message (char*): the message to be sent.
 *     packet (char*): variable to store the string.
 *
 * returns:
 *     (char*): the final packet to be sent by the client.
 */
char* form_packet(char* to, char* from, char* message, char* packet);

/**
 * Determine if there is data to be read from the server.
 *
 * params
 *     fd (int): the file descriptor for the sever.
 *
 * returns
 *     (int): 0 if nothing to read, non-negative number if something to
 *     rea, -1 if error.
 */
int server_data(int fd);

#endif // CLIENT_SHELL_Hs