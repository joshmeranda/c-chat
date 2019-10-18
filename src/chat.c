#include "chat_socket.h"
#include "client.h"
#include "server.h"
#include <error.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
"Usage: chat client -a <address> -p <port> -u <username>\n"
"  -h --help       Show this help text.\n"
"  -a --address    The address for the client.\n"
"  -p --port       The port for the client.\n"
"  -u --username   The username for the client.\n"
);
}

/**
 * Print the usage text for the server sub-command.
 */
void server_usage()
{
printf(
"Usage: chat server -a <address> -p <port>\n"
"  -h --help       Show this help text.\n"
"  -a --address    The address for the server.\n"
"  -p --port       The port for the server.\n"
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
    int port = -1, arg;

    struct option long_options[] = {
            {"help",     no_argument, 0, 'h'},
            {"address",  required_argument, 0, 'a'},
            {"port",     required_argument, 0, 'p'},
            {"username", required_argument, 0, 'u'},
            {0,         0,                  0,  0}
    };

    while ((arg = getopt_long(argc, argv, "a:p:u:", long_options, 0)) > 0)
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
            default:
                return 1; // Stop execution upon unknown option
        }
    }

    run_client(address, port, username);

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
    char *address = NULL;
    int port = -1, arg;

    struct option long_options[] = {
            {"help",     no_argument, 0, 'h'},
            {"address",  required_argument, 0, 'a'},
            {"port",     required_argument, 0, 'p'},
            {0,         0,                  0,  0}
    };

    while ((arg = getopt_long(argc, argv, "a:p:", long_options, 0)) > 0)
    {
        switch (arg)
        {
            case 'h':
                server_usage();
                return 0;
            case 'a':
                address = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                return 1; // Stop execution upon unknown option
        }
    }

    run_server(address, port);

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