#include "enc_chat_socket.h"
#include <openssl/err.h>

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
        fprintf(stderr, "{rivate key does not match the public cert.\n");
        abort();
    }
}

SSL_CTX* init_server_ctx()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = TLS_server_method();
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

ssize_t read_ssl(SSL *ssl, char *buffer, size_t n)
{
    ssize_t bytes_read = SSL_read(ssl, buffer, n);

    if (bytes_read < 0)
    {
        perror("read error");
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

ssize_t send_ssl(SSL *ssl, char *message, size_t n)
{
    ssize_t bytes_sent = SSL_write(ssl, message, n);

    if (bytes_sent < 0)
    {
        perror("send error");
    }

    return bytes_sent;
}