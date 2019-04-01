# c_chat
A simple chat server and client application written in c

## Build
To compile on a machine with comand `make` type `make all` or simply `make`
while the cwd contains the Makefile.

To run in sever move type `./chat server --address <address> --port <port>`
where `<address>` and `<port>` are the address and port used by the server.

To run in client mode use the same instructions seeen above for server but
replace 'server' with 'client'
(eg) `./chat client --addres <address> --port <port>`