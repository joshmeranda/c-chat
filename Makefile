CC=gcc
PTHREADL=-pthread
SSLL=-lssl -lcrypto
CFLAGS=-g -Wall $(PTHREADL) $(SSLL)
LFLAGS=-I./include -g -Wall
EXECUTABLE=chat
SRC=$(addprefix src/, chat.c chat_socket.c client.c server.c log.c)
OBJ=$(SRC:.c=.o)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(LFLAGS) -c $< -o $@

clear: clean

clean:
	rm -fv $(OBJ) $(EXECUTABLE)
