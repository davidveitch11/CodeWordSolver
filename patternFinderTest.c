/**
 * This is an interactive test script which will allow the user to enter a word
 * and will output any words which have the same pattern.
 * 
 * The user will be prompted to enter two words. The first a normal english word
 * such as "honest" and the other a kind of mask to indicate the 'known' letters.
 * For example, "honest" has the pattern '0 0 0 0 0 0' and so would match any
 * six letter word with no repeats if the mask given is "######", such as "letter".
 * If the mask given includes letters such as "&&####" then all six letter words
 * without repeats with the extra condition that the first letters are "ho" will
 * be returned, including "hoping".
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "data.h"

#define MAX 100

void loop();

int main(int argc, char** argv) {
    printf("When prompted, enter a word to define a pattern followed by\n");
    printf("a space and then a second word containing known letters. This\n");
    printf("must be the same length and have a '#' where the letter in\n");
    printf("that position is irrelevant and '&' when the letter must match\n");
    printf("the one given. There is a maximum length of 100 characters\n");

    init();

    loop();

    return 1;
}

void loop() {
    int buf_len = (MAX * 2) + 3; // space for two words, a space, '\n', and '\0'
    char buffer[buf_len];

    printf("\nEnter words: ");
    if (fgets(buffer, buf_len, stdin) != buffer) {
        perror("err");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(buffer);

    // Check for over-length
    if (buffer[len - 1] != '\n') {
        printf("The strings you have given are too long.\n");
        return;
    }

    // Cut off '\n'
    buffer[len - 1] = '\0';

    char *pattern_string = strtok(buffer, " ");
    if (pattern_string == NULL) {
        printf("Input format invalid (p)\n");
        return;
    }

    char *mask_string = strtok(NULL, " ");
    if (pattern_string == NULL) {
        printf("Input format invalid (m)\n");
        return;
    }

    if (strlen(pattern_string) != strlen(mask_string)) {
        printf("Strings must be the same length\n");
        return;
    }

    size_t n = strlen(pattern_string);

    char* pattern = generatePattern(pattern_string, n);
    if (!pattern) {
        return;
    }

    char* known = malloc(n);
    if (!known) {
        perror("Failed to allocate known memory");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        if (mask_string[i] == '#') {
            known[i] = 0;
        } else if (mask_string[i] == '&') {
            known[i] = pattern_string[i];
        } else {
            printf("Mask must only contain '#' or '&'\n");
            return;
        }
    }

    for (int i = 0; i < n; i++) {
        printf("%d ", pattern[i]);
    }
    printf("\n");

    newPattern(n, pattern, known);

    char* next;
    while (next = nextWord()) {
        printf("\t");
        for (int i = 0; i < n; i++) {
            printf("%c", *(next + i));
        }
        printf("\n");
    }
}
