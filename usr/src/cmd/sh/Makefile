CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lreadline

SRC = main.c \
      parser.c \
      alias.c \
      jobs.c \
      readline.c \
      utils.c \
	  pipeline.c

OBJ = $(SRC:.c=.o)

TARGET = shell

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean