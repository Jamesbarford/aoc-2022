TARGET  := solve.out
CC      := cc
CFLAGS  := -Wall -Wpedantic -Wextra -O0 -g
OBJECTS := $(wildcard ../../includes/*.o)
HEADERS := $(wildcard ../../includes/*.h)

all: $(TARGET)

$(TARGET): ./main.c
	$(CC) $(OBJECTS) $(CFLAGS) -o $@ $^

clean:
	rm $(TARGET)
