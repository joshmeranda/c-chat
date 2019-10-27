#include "client.h"
#include "server.h"
#include <error.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#define MAX_CLIENT 1024

/**
 * Print the usage text for the chat program.
 */
void chat_usage()
{
printf(
"Usage: chat <command> [options]\n"
"  client    Run the client mode of the chat program.\n"
"  server    Run the server mode of the chat program.\n"
);
}

/**
 * Print the usage text for the client sub-command.
 */
void client_usage()
{
printf(
"Usage: chat client -a <address> -p <port> -u <username> [-e]\n"
"  -h --help         Show this help text.\n"
"  -a --address [ADDR]  The address for the client.\n"
"  -p --port [PORT] The port for the client.\n"
"  -u --username [USERNAME]  The username for the client.\n"
"  -e --encrypt      Specify to communicate with tls.\n"
);
}

/**
 * Print the usage text for the server sub-command.
 */
void server_usage()
{
printf(
"Usage: chat server -a <address> -p <port> [-e -c FILE -k FILE] \n"
"  -h --help         Show this help text.\n"
"  -a --address [ADDR]  The address for the server.\n"
"  -p --port [PORT]  The port for the server.\n"
"  -e --encrypt      Specify to communicate with tls, requires '-c' && '-k'\n"
"                    options.\n"
"  -c --cert [FILE]  The cert file for tls encryption.\n"
"  -k --key [FILE]   The key file for th tls encryption.\n"
"  -m --max-client [COUNT]  The maximum amount of client connections the server\n"
"                    should handle.\n"
);
}

/**
 * Run the client sub-command and parse the program arguments.
 *
 * @param argc The amount of arguments.
 * @param argv The array of arguments.
 * @return The exit code for the program.
 */
int client_cli(int argc, char **argv)
{
    char *address = NULL, *username = NULL;
    int port = -1, arg, enc = 0;

    struct option long_options[] = {
            {"help",     no_argument, 0, 'h'},
            {"address",  required_argument, 0, 'a'},
            {"port",     required_argument, 0, 'p'},
            {"username", required_argument, 0, 'u'},
            {"encrypt", no_argument,       0, 'e'},
            {0,         0,                  0,  0}
    };

    while ((arg = getopt_long(argc, argv, "ha:p:u:e", long_options, 0)) > 0)
    {
        switch (arg)
        {
            case 'h':
                client_usage();
                return 1;
            case 'a':
                address = strdup(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'u':
                username = strdup(optarg);
                break;
            case 'e':
                enc = 1;
                break;
            default:
                return 1; // Stop execution upon unknown option
        }
    }

    if (address == NULL || port == -1|| username == NULL)
    {
        printf("chat: Missing required options.\nSee 'chat client --help' for more information.");
    }
    else
    {
        run_client(address, port, username, enc);
    }

    free(address);
    free(username);

    return 0;
}

/**
 * Run the server sub-command and parse the program arguments.
 *
 * @param argc The amount of arguments.
 * @param argv The array of arguments.
 * @return The exit code for the program.
 */
int server_cli(int argc, char **argv)
{
    char *address = NULL, *cert = NULL, *key = NULL;
    int port = -1, arg, enc = 0, max_client = MAX_CLIENT;

    struct option long_options[] = {
            {"help",       no_argument,       0, 'h'},
            {"address",    required_argument, 0, 'a'},
            {"port",       required_argument, 0, 'p'},
            {"encrypt",    no_argument,       0, 'e'},
            {"cert",       required_argument, 0, 'c'},
            {"key",        required_argument, 0, 'k'},
            {"max-client", required_argument, 0, 'm'},
            {0,            0,                 0,  0}
    };

    while ((arg = getopt_long(argc, argv, "ha:p:ec:k:m:", long_options, 0)) > 0)
    {
        switch (arg)
        {
            case 'h':
                server_usage();
                return 0;
            case 'a':
                address = strdup(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'e':
                enc = 1;
                break;
            case 'c':
                cert = strdup(optarg);
                break;
            case 'k':
                key = strdup(optarg);
                break;
            case 'm':
                max_client = atoi(optarg);
                break;
            default:
                return 1; // Stop execution upon unknown option
        }
    }

    if (address == NULL || port == -1 || (enc == 1 && (cert == NULL || key == NULL)))
    {
        printf("chat: Missing required options.\nSee 'chat client --help' for more information.");
    }
    else
    {
        run_server(address, port, max_client, enc, cert, key);
    }

    if (cert != NULL) free(address);
    if (cert != NULL) free(cert);
    if (key != NULL) free(key);

    return 0;
}

int main(int argc, char** argv)
{
    if (strcmp(argv[1], "server") == 0)
    {
        server_cli(argc, argv);
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        client_cli(argc, argv);
    }
    else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
        chat_usage();
    }
    else
    {
        printf("chat: Unsupported command '%s'\n", argv[1]);
    }

    return 0;
}