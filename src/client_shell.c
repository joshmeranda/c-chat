#include "client_shell.h"

char* get_prompt()
{
    return " $> ";
}

void set_command(char* input, int* command)
{
    if (input[0] != '.')
    {
        *command = SEND;
        return;
    }

    int i = 0;
    while (strcmp(command_arr[i], "\0") != 0)
    {
        if (strcmp(command_arr[i], input) == 0) { break; } // command found
        i++;
    }

    *command = i;
}