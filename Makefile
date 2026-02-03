# Compiler
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

# $(wildcard *.c) finds all .c files in the current directory and returns a space-separated list
# Source files
SRCS = $(wildcard *.c)
DEPS = $(wildcard *.h)

# Target executable
TARGET = peekc

.PHONY: all clean
all: $(TARGET)

# Rebuild when any source or header changes
$(TARGET): $(SRCS) $(DEPS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)


