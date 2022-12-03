#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"

enum Play {
    ROCK = 1,
    PAPER = 2,
    SCISSORS = 3,
};

enum Strat {
    LOSE = 0,
    DRAW = 3,
    WIN = 6,
};

static const int opponent_move[] = {
        ['A'] = ROCK,
        ['B'] = PAPER,
        ['C'] = SCISSORS,
};

static const int game_fix[] = {
        ['X'] = LOSE,
        ['Y'] = DRAW,
        ['Z'] = WIN,
};

int pickWinner(enum Play opponent, enum Strat strat) {
    switch (strat) {
        case LOSE: {
            if (opponent == ROCK) {
                return SCISSORS + strat;
            } else if (opponent == PAPER) {
                return ROCK + strat;
            } else {
                return PAPER + strat;
            }
        }
        case DRAW: {
            return strat + opponent;
        }
        case WIN: {
            if (opponent == ROCK) {
                return PAPER + strat;
            } else if (opponent == PAPER) {
                return SCISSORS + strat;
            } else {
                return ROCK + strat;
            }
        }
    }
}

int solve(rFile *rf) {
    int wins = 0;
    char *ptr = rf->buf;
    enum Play opponent;
    enum Strat me;

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
                me = game_fix[(int)*ptr];
                break;

            case '\n':
                if (*(ptr + 1) == '\n') {
                    return wins;
                }
                wins += pickWinner(opponent, me);
                break;

            default:
                break;
        }
        ptr++;
    }

    return wins;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    printf("wins: %d\n", solve(rf));
    rFileRelease(rf);
}
