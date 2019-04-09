#ifndef CLIENT_SHELL_H
#define CLIENT_SHELL_H

#include "chat_socket.h"
#include <stdio.h>

char* get_prompt();

void client_shell(struct sock_info *c_sock);

#endif // CLIENT_SHELL_H