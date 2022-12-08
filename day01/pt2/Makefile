TARGET  := solve.out
CC      := cc
CFLAGS  := -Wall -Wpedantic -Wextra -O2

all: $(TARGET)

$(TARGET): ./main.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(TARGET)
