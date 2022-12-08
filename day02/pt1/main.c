#include <stdio.h>
#include <stdlib.h>

enum Play {
    ROCK = 1,
    PAPER = 2,
    SCISSORS = 3,
};

static const int opponent_move[] = {
        ['A'] = ROCK,
        ['B'] = PAPER,
        ['C'] = SCISSORS,
};

static const int my_pick[] = {
        ['X'] = ROCK,
        ['Y'] = PAPER,
        ['Z'] = SCISSORS,
};

int pickWinner(enum Play opponent, enum Play me) {
    if (opponent == me) {
        return me + 3;
    } else if (opponent == ROCK && me == SCISSORS) {
        return me;
    } else if (opponent == SCISSORS && me == PAPER) {
        return me;
    } else if (opponent == PAPER && me == ROCK) {
        return me;
    }
    return me + 6;
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[10] = {'\0'};
    int wins = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        wins += pickWinner(opponent_move[(int)buf[0]], my_pick[(int)buf[2]]);
    }

    fclose(fp);
    printf("wins: %d\n", wins);
}
