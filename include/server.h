#include "chat_socket.h"

/**
 * Listen for and accept client connections.
 *
 * @param sock Socket to use whe creating the client file descriptor.
 * @return The file descriptor for the client.
 */
int accept_connection(sock_t sock);

/**
 * Create an IPv4 TCP socket for the server.
 *
 * @param address The address for the server.
 * @param port The port for the server.
 * @param sock Reference to the soc to modify.
 * @param log The log file for the server.
 * @param enc Whether or not the socket should operate over TLS.
 * @param cert The cert file path.
 * @param key The key file path/
 * @return The sock_t struct which was created.
 */
sock_t start_server(char* address, int port, sock_t *sock, FILE *log, int enc, char *cert, char *key);

/**
 * Create and run the chat server.
 *
 * @param address The address to run the server at.
 * @param port The port used by the server.
 * @param max_client The maximum amount of clients the server should serve.
 * @param enc Whether the server should use encryption or not (0 or 1).
 * @param cert Path to the cert file to use for encryption.
 * @param key Path to the key file to use for encryption.
 */
void run_server(char* address, int port, int max_client, int enc, char *cert, char *key);

/**
 * Establish a working fd set for user by a server.
 *
 * @param fd_arr An array of connected file descriptors.
 * @param set The FD_SET to be established.
 * @param sock_fd The file descriptor of the server socket.
 * @return The largest integer value of any file descriptor in the set.
 */
int prepare_fd_set(int *fd_arr, fd_set *set, int sock_fd, int max_client);

/**
 * Send a packet to a specified destination.
 *
 * @param fd_arr Array of all file descriptors.
 * @param user_arr Array of all usernames.
 * @param ssl_arr Array of ssl connections.
 * @param packet The packet to be sent.
 * @param dest The destination username to receive the packet.
 * @param enc Specifies if the server is using encryption or not.
 */
ssize_t handle_user_to_user(int *fd_arr, char **user_arr, SSL **ssl_arr, char *packet, char *dest, int max_client);

/**
 * Reply to client with list of connected users.
 *
 * @param fd The file descriptor to send the reply.
 * @param ssl The ssl connection to send the reply if encryption is enabled.
 * @param user_arr The array of connected users.
 * @param enc Specify if the server is using encryption or not.
 */
void handle_list(int fd, SSL *ssl, char **user_arr, int max_client);

/**
 * Test newly provided username against currently existing usernames.
 *
 * @param user_arr The array of currently existing usernames.
 * @param username The username to check for validity.
 * @return 1 if the username can be used, 0 otherwise.
 */
int valid_username(char **user_arr, char *username, int max_client);
