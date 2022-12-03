#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"

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
    if (opponent == ROCK && me == SCISSORS) {
        return SCISSORS;
    } else if (opponent == SCISSORS && me == PAPER) {
        return PAPER;
    } else if (opponent == PAPER && me == ROCK) {
        return ROCK;
    }
    return me + 6;
}

int solve(rFile *rf) {
    int wins = 0;
    char *ptr = rf->buf;
    enum Play opponent, me;

    while (*ptr != '\0') {
        switch (*ptr) {
            case 'A':
            case 'B':
            case 'C':
                opponent = opponent_move[(int)*ptr];
                break;

            case 'X':
            case 'Y':
            case 'Z':
                me = my_pick[(int)*ptr];
                break;

            case '\n':
                if (*(ptr + 1) == '\n') {
                    return wins;
                }
                wins += pickWinner(opponent, me);
                ptr++;
                break;
            default:
                break;
        }
        ptr++;
    }

    return wins;
}

int main(void) {
    rFile *rf = rFileRead("./warmup.txt");
    printf("wins: %d\n", solve(rf));
    rFileRelease(rf);
}
