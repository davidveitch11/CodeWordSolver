
/**
 * Test script for the puzzle file parser.
 * Call with single argument: filename of puzzle file to parse.
 * Will then display the contents of the file as parsed
 */

#include "puzzel.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("First argument must be puzzle file name\n");
        return -1;
    }

    struct puzzle* puzzle = parse(argv[1]);

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
