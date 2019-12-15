#include "chat_socket.h"

/**
 * Accept a new client connection.
 *
 * @param sock The socket s truct to modify.
 * @param fd The filde descriptor associated with incoming client connections (server fd).
 * @param log The log file to send logs to.
 * @param enc Whether to create an encrypted connection or not.
 * @return The file descriptor for the new connection, or -1 on error;.
 */
int accept_connection(sock_t **sock, int fd, FILE *log, int enc);


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
 * @param sock_t Array of sockets from which to pull fds from.
 * @param server_fd The file descriptor the server is listing for new connections on.
 * @param set The FD_SET to be established.
 * @param sock_fd The file descriptor of the server socket.
 * @return The largest integer value of any file descriptor in the set.
 */
int prepare_fd_set(sock_t **sock_arr, int server_fd, fd_set *set, int max_client);

/**
 * Send a packet to a specified destination.
 *
 * @param sock_arr Array of all available socket connections.
 * @param packet The packet to be sent.
 * @param dest The destination username to receive the packet.
 * @param max_client The size of the sock_arr.
 */
ssize_t handle_user_to_user(sock_t **sock_arr, char *packet, char *dest, int max_client);

/**
 * Reply to client with list of connected users.
 *
 * @param sock The socket to which the message is to be sent.
 * @param enc Specify if the server is using encryption or not.
 */
void handle_list(sock_t *dest_sock, sock_t **sock, int max_client);

/**
 * Test newly provided username against currently existing usernames.
 *
 * @param sock_arr Array of sockets from which to test usernames.
 * @param username The username to check for validity.
 * @return 1 if the username can be used, 0 otherwise.
 */
int valid_username(sock_t **sock_arr, char *username, int max_client);
