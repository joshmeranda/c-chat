#include "chat_socket.h"
#include <stdio.h>

/**
 * @property max_data The amount of messages contained.
 * @property data_count The amount of messages which have been received.
 * @property offset The starting index of the data.
 * @property data The array of data received by the client.
 */
struct
{
    int offset;
    ssize_t count,
            size;
    char **data;
    sock_t *sock;
} typedef wdata_t;

void update_data(wdata_t *data, char *message);

void update_screen(wdata_t *data);
