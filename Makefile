CC=gcc
CFLAGS=--all-warnings
LFLAGS=-I./include
EXECUTABLE=chat

# source and object files
SRC=src/c_chat.c src/chat_socket.c
OBJ=$(SRC:.c=.o)

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(LFLAGS) $(CFLAGS) $(OBJ) -o $@ -lm

%.o: %.c
	$(CC) $(LFLAGS) -c $< -o $@

clear: clean

clean:
	@echo "=== Cleaning ==="
	rm -fv $(OBJ) $(EXECUTABLE)
	@echo "=== DONE ==="