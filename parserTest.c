
#include "puzzel.h"

#include <stdio.h>

int main(int argc, char **argv) {
    struct puzzle* puzzle = parse("puzzles/a.pzl");

    // Print puzzle
    for (int i = 0; i < 26; i++) {
        if (puzzle->known[i]) {
            printf("%d -> %c\n", i + 1, puzzle->known[i]);
        }
    }

    struct cwrd** cws = puzzle->cwrds;
    struct cwrd* cw;
    while (cw = *cws) {
        for (int i = 0; i < cw->len; i++) {
            printf("%d ", cw->clets[i]);
        }

        printf("\n");

        cws++;
    }

    freePuzzle(puzzle);

    return 0;
}
