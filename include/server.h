#include "chat_socket.h"

#define MAX_CLIENT 10

/**
 * Listen for and accept client connections.
 *
 * @param sock Socket to use whe creating the client file descriptor.
 * @param env Accept a connect over ssl.
 * @return The file descriptor for the client.
 */
int accept_connection(SOCK sock, int enc);

/**
 * Create an IPv4 TCP socket for the server.
 *
 * @param address The address for the server.
 * @param port The port for the server.
 * @return The sock_info describing the server socket.
 */
SOCK start_server(char* address, uint16_t port, int enc,  char *cert, char *key);

/**
 * Create and run the chat server.
 *
 * @param address The address to run the server at.
 * @param port The port used by the server.
 * @param enc Whether the server should use encryption or not (0 or 1).
 * @param cert Path to the cert file to use for encryption.
 * @param key Path to the key file to use for encryption.
 */
void run_server(char* address, int port, int enc, char *cert, char *key);

/**
 * Establish a working fd set for user by a server.
 *
 * @param fd_arr An array of connected file descriptors.
 * @param set The FD_SET to be established.
 * @param sock_fd The file descriptor of the server socket.
 * @return The largest integer value of any file descriptor in the set.
 */
int prepare_fd_set(int *fd_arr, fd_set *set, int sock_fd);

/**
 * Send a packet to a specified destination.
 *
 * @param fd_arr Array of all file descriptors.
 * @param user_arr Array of all usernames.
 * @param packet The packet to be sent.
 * @param dest The destination username to receive the packet.
 */
void handle_user_to_user(int *fd_arr, char **user_arr, char *packet, char *dest);

/**
 * Store a new username, and reply to new user with welcome message.
 *
 * @param username Pointer to the destination for the username.
 * @param fd The file descriptor associated with the username.
 * @param packet The packet containing the username.
 */
void handle_new_user(char **username, int fd, char *packet);

/**
 * Reply to client with list of connected users.
 *
 * @param fd The file descriptor to send the reply.
 * @param user_arr The array of connected users.
 */
void handle_list(int fd, char **user_arr);