CC := cc

a.out: ./main.c ./map.c ./list.c ./hmap.c
	$(CC) -Wall -Wpedantic -Wextra -O2 -o $@ $^

clean:
	rm -rf a.out
