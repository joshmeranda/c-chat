#include "chat_socket.h"

#define MAX_CLIENT 10

/**
 * Broadcast a message to all connected clients.
 *
 * @param message The message to send to be broadcasted.
 * @param client_fd_arr An array of all file descriptors for the connected
 * clients.
 */
void broadcast(char* message, int* client_fd_arr);

/**
 * Create and run the chat server.
 *
 * @param address The address to run the server at.
 * @param port The port used by the server.
 */
void run_server(char* address, int port);