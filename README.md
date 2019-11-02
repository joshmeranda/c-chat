# c_chat
A simple chat server and client application written in c

## Build
Build program with `make` command

## Running
Can be run in two modes, server and client, which are specified with like-named subcommands. When running over TLS be sure to provide a cert and key for the server.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                

### Client
```
Usage: chat client -a <address> -p <port> -u <username> [-e]
  -h --help         Show this help text.
  -a --address [ADDR]  The address for the client.
  -p --port [PORT] The port for the client.
  -u --username [USERNAME]  The username for the client.
  -e --encrypt      Specify to communicate with tls.
```

### Server
```
Usage: chat server -a <address> -p <port> [-e -c FILE -k FILE] 
  -h --help         Show this help text.
  -a --address [ADDR]  The address for the server.
  -p --port [PORT]  The port for the server.
  -e --encrypt      Specify to communicate with tls, requires '-c' && '-k'
                    options.
  -c --cert [FILE]  The cert file for tls encryption.
  -k --key [FILE]   The key file for th tls encryption.
  -m --max-client [COUNT]  The maximum amount of client connections the server
                    should handle.
```
