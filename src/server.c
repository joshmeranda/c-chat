#include "server.h"

void broadcast(char* message, int* client_fd_arr)
{
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (client_fd_arr[i] != 0) { send_fd(client_fd_arr[i], message); }
    }
}