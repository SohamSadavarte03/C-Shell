CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = project/main.c project/input.c project/log.c project/hop.c project/reveal.c project/proclore.c project/seek.c

OBJS = $(SRCS:.c=.o)

EXEC = a.out

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC)

.PHONY: all clean run
