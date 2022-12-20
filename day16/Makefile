CC := cc
all: a.out part2.out

a.out: ./main.c ./map.c ./list.c ./hmap.c
	$(CC) -Wall -Wpedantic -Wextra -O2 -o $@ $^

part2.out: ./part2.c ./map.c ./list.c ./hmap.c
	$(CC) -Wall -Wpedantic -Wextra -O2 -o $@ $^

clean:
	rm -rf a.out
