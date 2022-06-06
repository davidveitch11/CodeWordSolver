
/**
 * The data module is responsible for reading from the word list.
 * The method init must be called before reading.
 * The method newPattern must be called to start a new search through the list.
 * After newPattern, nextWord will return the location of the next word in the list.
 * 
 * This module also provides a method generatePattern for generating a pattern from
 * a letter string
 */

#include <stdlib.h>
#include "data.h"

// Location currently looked at
char* current;
char* end;
int len;
char *known;

struct data_store* data_store;

void init() {
    data_store = read();
}

char* generatePattern(char *string, char len) {
    char *pattern = malloc(len);
    if (!pattern) {
        perror("Failed to allocate pattern string in generation");
        exit(EXIT_FAILURE);
    }

    // Find range of letters
    char range = 'a'; // default alphanumeric
    if (string[0] < 'a') {
        range = 1; // use 1-26 if different range
    }

    // Ensure all letters alphanum
    for (int i = 0; i < len; i++) {
        if (string[i] < range || string[i] > (range + 25)) {
            printf("Out of range character encountered: ");
            printf("%d position %d\n", string[i], i);
            return NULL;
        }
    }
    
    // printf("string=");
    // for (int i = 0; i < len; i++) {
    //     printf("%d ", string[i]);
    // }
    // printf("\n");

    // Count frequency of letters in word
    char alpha[26] = {0};
    for (int i = 0; i < len; i++) {
        alpha[string[i] - range]++;
    }

    // printf("alpha=");
    // for (int i = 0; i < 26; i++) {
    //     printf("%d ", alpha[i]);
    // }
    // printf("\n");

    // Group counters
    char group = 1;
    char groups[26] = {0};

    // Assign letters to groups
    for (int i = 0; i < len; i++) {
        int n = string[i] - range;

        // printf("%d ", n);

        if (alpha[n] == 1) {
            // Only one - default group
            pattern[i] = 0;

            // printf("0 (default)\n");
        } else {
            // More than one - assign group

            // New group
            if (groups[n] == 0) {
                groups[n] = group++;

                // printf("(new group) ");
            }

            // Assign
            pattern[i] = groups[n];
            
            // printf("%d\n", pattern[i]);
        }
    }

    // printf("pattern=");
    // for (int i = 0; i < len; i++) {
    //     printf("%d ", pattern[i]);
    // }
    // printf("\n");

    return pattern;
}

void newPattern(int n, char *base, char *filled) {
    len = n;
    known = filled;

    // printf("new pattern=");
    // for (int i = 0; i < n; i++) {
    //     printf("%d ", base[i]);
    // }

    // printf("known=");
    // for (int i = 0; i < n; i++) {
    //     printf("%d ", filled[i]);
    // }

    for (uint32_t i = 0; i < data_store->num_patterns; i++) {
        if (matchPattern(base, len, data_store->patterns + i)) {
            struct pattern* p = data_store->patterns + i;
            current = p->start;
            end = p->end;

            // printf("current=%p end=%p\n", current, end);
            char* c1 = current;
            while (c1 < end) {
                // printf(" = ");
                // for (int j = 0; j < n; j++) {
                //     printf("%c", c1[j]);
                // }
                // printf("\n");

                c1 += n;
            }

            return;
        }
    }

    printf("Error: unable to find pattern '");
    for (int i = 0; i < n; i++) {
        printf("%d ", *(base + i));
    }
    printf("' - aborting\n");
    exit(EXIT_FAILURE);
}

char matchPattern(char *pattern, int len, struct pattern *p) {
    if (len != p->len) {
        return 0;
    }

    char *test = p->pattern;

    for (int i = 0; i < len; i++, pattern++, test++) {
        if (*pattern != *test) {
            return 0;
        }
    }

    return 1;
}

char* nextWord() {
    while (current != end) {
        // Check pattern matches known values
        char *p = known;
        char ok = 1;
        for (int i = 0; i < len; i++, p++) {
            if (*p > 0) {
                if (*(current + i) != *p) {
                    ok = 0;
                    break;
                }
            }
        }

        // Move to next
        char* ret = current;
        current += len;

        // If matched pattern, return word
        if (ok) {
            return ret;
        }
    }

    return NULL;
}
