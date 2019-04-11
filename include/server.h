#include "chat_socket.h"

#define MAX_CLIENT 10

void broadcast(char* message, int* client_fd_arr);