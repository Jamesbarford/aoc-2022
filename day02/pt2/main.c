#include <stdio.h>
#include <stdlib.h>

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

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[10] = {'\0'};
    int wins = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        wins += pickWinner(opponent_move[(int)buf[0]], game_fix[(int)buf[2]]);
    }

    fclose(fp);
    printf("wins: %d\n", wins);
}
