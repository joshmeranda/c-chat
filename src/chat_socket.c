#include "chat_socket.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <openssl/err.h>

int exit_received = 0;

void handle_signal(int sig)
{
    exit_received = sig;
}

ssize_t read_fd(int fd, char buffer[BUFFER_SIZE])
{
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);

    if (bytes_read < 0)
    {
        perror("read error");
        return bytes_read;
    } else if (bytes_read == 0) {
        return 0;
    }

    if (buffer[bytes_read - 1] != '\0')
    {
        buffer[bytes_read] = '\0';
    }

    return bytes_read;
}

ssize_t send_fd(int fd, char* message)
{
    ssize_t bytes_sent = send(fd, message, strlen(message), 0);

    if (bytes_sent < 0)
    {
        perror("send error");
    }

    return bytes_sent;
}

int shutdown_socket(sock_t *sock)
{
    if (shutdown(sock->fd, SHUT_RDWR) < 0)
    {
        perror("Shutdown failure");
        return -1;
    }

    if (sock->ssl) SSL_free(sock->ssl);
    if (sock->ctx) SSL_CTX_free(sock->ctx);
    if (sock->buffer) free(sock->buffer);

    return 0;
}

void load_certs(SSL_CTX *ctx, char *cert, char *key)
{
    // set local certificate from cert file
    if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    // set the private key from the key file
    if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    // verify the private key
    if (!SSL_CTX_check_private_key(ctx))
    {
        fprintf(stderr, "Private key does not match the public cert.\n");
        abort();
    }
}

SSL_CTX* init_server_ctx()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = TLS_server_method(); // move to non-deprecated
    ctx = SSL_CTX_new(method);

    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    return ctx;
}

SSL_CTX* init_client_ctx()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = TLS_client_method();
    ctx = SSL_CTX_new(method);

    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    return ctx;
}

ssize_t read_ssl(SSL *ssl, char *buffer)
{
    size_t bytes_read = SSL_read(ssl, buffer, BUFFER_SIZE);

    if (bytes_read < 0)
    {
        ERR_print_errors_fp(stderr);
        // perror("read error");
        return bytes_read;
    }
    else if (bytes_read == 0)
    {
        return 0;
    }

    if (buffer[bytes_read - 1] != '\0')
    {
        buffer[bytes_read] = '\0';
    }

    return bytes_read;
}

ssize_t send_ssl(SSL *ssl, char *message)
{
    ssize_t bytes_sent = SSL_write(ssl, message, strlen(message));

    if (bytes_sent < 0)
    {
        perror("send error");
    }

    return bytes_sent;
}

ssize_t chat_send(sock_t *sock, char *packet)
{
    if (sock->ssl != NULL)
        return send_ssl(sock->ssl, packet);
    else
        return send_fd(sock->fd, packet);
}

ssize_t chat_read(sock_t *sock, char *buffer)
{
    if (sock->ssl != NULL)
        return read_ssl(sock->ssl, buffer);
    else
        return read_fd(sock->fd, buffer);
}

char* form_packet(char **packet, ...)
{
    va_list args;
    char *arg;

    *packet = NULL;
    va_start(args, packet);
    arg = va_arg(args, char*);

    while (arg != NULL)
    {
        append_section(packet, arg);
        arg = va_arg(args, char*);
    }
    va_end(args);

    return *packet;
}

char *get_next_section(char **packet)
{
    return strndup(*packet, strstr(*packet, DELIMITER) - *packet);
}

char *append_section(char **packet, char *section)
{
    if (*packet == NULL)
    {
        *packet = malloc(strlen(section) + strlen(DELIMITER) + 1);
        strcpy(*packet, section);
    }
    else
    {
        *packet = realloc(*packet, strlen(*packet) + strlen(section) + strlen(DELIMITER) + 1);
        strcat(*packet, section);
    }

    return strcat(*packet, DELIMITER);
}