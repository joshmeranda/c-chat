#ifndef CLIENT_SHELL_H
#define CLIENT_SHELL_H

#include "chat_socket.h"
#include <stdio.h>

enum command_types
{
    SEND,
    EXIT,
    CONNECT,
    DISCONNECT,
    BROADCAST,
    LIST
};

static char* command_arr[] =
{
    ".send\n",
    ".exit\n",
    ".connect\n",
    ".disconnect\n",
    ".broadcast\n",
    ".list\n",
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

#endif // CLIENT_SHELL_H