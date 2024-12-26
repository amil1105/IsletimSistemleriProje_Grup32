# AMIL SHIKHIYEV G221210561
# Erkin Erdoğan B241210385
# Kianoush Seddighpour G221210571
# Manar AL SAYED ALI G221210558

# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGETS = mysh increment


all: $(TARGETS)

mysh: mysh.c
	$(CC) $(CFLAGS) -o mysh mysh.c

increment: increment.c
	$(CC) $(CFLAGS) -o increment increment.c

clean:
	rm -f $(TARGETS)
