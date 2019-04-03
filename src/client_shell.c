#include "client_shell.h"

char* get_prompt()
{
    return " $> ";
}

void client_shell(struct sock_info *c_sock)
{
    printf("%s", get_prompt());
    fgets(c_sock->buffer, BUFFER_SIZE, stdin);
}