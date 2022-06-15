
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

// Location in memory of the start of the next word to be read
char* current;

// Location in memory of the first word after the last word to be read in this pattern.
// When current equals end, the pattern has no more words to be read
char* end;

// Length of pattern/word currently being used
int len;

// Array holding the known letters for this pattern in the places they occur.
char *known;

// The structure holding the list of words and patterns.
// Must be initialised first
struct data_store* data_store;

/**
 * @brief Initialise the data store. Must be called before any other function in this module
 */
void init() {
    data_store = read();
}

/**
 * @brief Generate a pattern string for this word.
 * Takes a word and allocates a new string to hold that word's pattern.
 * All characters in input string must be either 1-26 or a-z
 */
char* generatePattern(char *string, char len) {
    // Allocate new memory to hold pattern
    char *pattern = malloc(len);
    if (!pattern) {
        perror("Failed to allocate pattern string in generation");
        exit(EXIT_FAILURE);
    }

    // Find the range of the letters in string. All letters must be in either 1-26 or a-z
    char range = 'a'; // default alphanumeric
    if (string[0] < 'a') {
        range = 1; // use 1-26 if different range
    }

    // Ensure all letters in the range
    for (int i = 0; i < len; i++) {
        if (string[i] < range || string[i] > (range + 25)) {
            printf("Out of range character encountered: ");
            printf("%d position %d\n", string[i], i);
            free(pattern);
            return NULL;
        }
    }

    // Count frequency of letters in word
    char alpha[26] = {0};
    for (int i = 0; i < len; i++) {
        alpha[string[i] - range]++;
    }

    // Group counters
    char group = 1;
    char groups[26] = {0};

    // Assign letters to groups
    for (int i = 0; i < len; i++) {
        // Turn letters from their range into 0-25
        int n = string[i] - range;

        // Act based on number of ocurrences of same letter within the word

        if (alpha[n] == 1) {
            // Only one - default group
            pattern[i] = 0;
        } else {
            // More than one - assign group

            // New group
            if (groups[n] == 0) {
                groups[n] = group++;
            }

            // Assign
            pattern[i] = groups[n];
        }
    }

    return pattern;
}

/**
 * @brief Set a new pattern to search through.
 * Calling this method indicates the search for words will continue for this new pattern.
 * This will start reading from the start of this pattern.
 * @param n Number of characters in pattern
 * @param base Basic pattern holds information about letter duplication
 * @param filled Array of characters which have already been filled in, entries set to zero when not filled
 */
void newPattern(int n, char *base, char *filled) {
    // Save parameters
    len = n;
    known = filled;

    // Search for the correct pattern through the list
    for (uint32_t i = 0; i < data_store->num_patterns; i++) {
        if (matchPattern(base, len, data_store->patterns + i)) {
            // Pattern matches

            // Save start and end locations
            struct pattern* p = data_store->patterns + i;
            current = p->start;
            end = p->end;

            // No need to continue searching
            return;
        }
    }

    // If we got here, we didnt find a matching pattern

    printf("Error: unable to find pattern '");
    for (int i = 0; i < n; i++) {
        printf("%d ", *(base + i));
    }
    printf("' - aborting\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief Check if two patterns match.
 * @return char 1 if pattern is the same, 0 otherwise
 */
char matchPattern(char *pattern, int len, struct pattern *p) {
    // Easiest difference between patterns is length
    if (len != p->len) {
        return 0;
    }

    // If lengths match, check contents are identical
    char *test = p->pattern;
    for (int i = 0; i < len; i++, pattern++, test++) {
        if (*pattern != *test) {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Get the next word for the current search.
 * @return char* Pointer to start of the next word or null if no more words remain
 */
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
