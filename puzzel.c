
/**
 * This module is responsible for parsing a puzzle file into a data structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "puzzel.h"
#include "data.h"

struct cwrd_node {
    struct cwrd* cwrd;
    struct cwrd_node* next;
};

/**
 * Open the file name 'filename' and parse the file as a puzzle.
 */
struct puzzle* parse(char *filename) {
    // Open puzzle file
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Allocate memory
    struct puzzle* p = malloc(sizeof(struct puzzle));
    if (!p) {
        perror("Failed to allocate puzzle");
        exit(EXIT_FAILURE);
    }

    // Clear the 'known' field
    if (memset(p->known, 0, 26) != p->known) {
        perror("Failed to clear known");
        exit(EXIT_FAILURE);
    }

    // Parse the known fields
    char r = 1;
    while (r) {
        r = parseKnown(fp, p->known);
    }

    // Parse the word fields
    int i = 0;
    struct cwrd_node* head = NULL;
    struct cwrd_node* next;
    while (next = parseWord(fp, head)) {
        i++;
        head = next;
    }

    // Allocate list of pointers
    p->cwrds = calloc(sizeof(struct cwrd*), i + 1);
    if (!p->cwrds) {
        perror("Failed to allocate cwrds");
        exit(EXIT_FAILURE);
    }

    // Convert linked list to array of pointers
    for (int j = 0; j < i; j++) {
        p->cwrds[j] = head->cwrd;

        next = head->next;
        free(head);
        head = next;
    }

    // Clear the 'needed' field
    if (memset(p->needed, 0, 26) != p->needed) {
        perror("Failed to clear needed");
        exit(EXIT_FAILURE);
    }

    // Fill needed field
    for (int j = 0; j < i; j++) {
        struct cwrd* cw = p->cwrds[j];

        for (int k = 0; k < cw->len; k++) {
            int l = cw->clets[k] - 1;
            p->needed[l] = 1;
        }
    }

    return p;
}

/**
 * Parse a single line of the file as a known letter.
 * Returns 1 on success (line parsed) or 0 if it has
 * reached the end of the 'known' section
 */
char parseKnown(FILE *fp, char *known) {
    // Read a single line into the buffer
    char buffer[10];
    if (fgets(buffer, 10, fp) != buffer) {
        if (feof(fp)) {
            return 0;
        }

        perror("Failed to read line.");
        exit(EXIT_FAILURE);
    }

    // Check for end of section
    if (buffer[0] == '\n' || buffer[0] == '\r') {
        return 0;
    }

    // Parse line
    int letter;
    char character;
    if (sscanf(buffer, "%d %c", &letter, &character) != 2) {
        printf("Failed to parse line '%s' in known\n", buffer);
        exit(EXIT_FAILURE);
    }

    // Bounds check
    letter--;
    if (letter < 0 || letter > 25) {
        printf("Code letter out of range: letter %d in line '%s'\n", letter, buffer);
        exit(EXIT_FAILURE);
    }
    if (character < 'a' || character > 'z') {
        printf("Letter out of range: letter %c in line '%s'\n", character, buffer);
        exit(EXIT_FAILURE);
    }

    // Save value
    known[letter] = character;

    return 1;
}

/**
 * @brief Parse a line of input from the file as a codeword
 */
struct cwrd_node* parseWord(FILE *fp, struct cwrd_node* head) {
    // Read a line
    char line_buffer[100];
    if (fgets(line_buffer, 100, fp) != line_buffer) {
        if (feof(fp)) {
            return NULL;
        }

        perror("Failed to read line");
        exit(EXIT_FAILURE);
    }
    
    // Check for last line of file
    if (strlen(line_buffer) == 0) {
        return NULL;
    }

    // Create new node
    struct cwrd_node* node = malloc(sizeof(struct cwrd_node));
    if (!node) {
        perror("Failed to allocate cwrd node");
        exit(EXIT_FAILURE);
    }

    // Create node contents
    node->cwrd = malloc(sizeof(struct cwrd));
    if (!node->cwrd) {
        perror("Failed to allocate cwrd");
        exit(EXIT_FAILURE);
    }

    // Link to rest of list
    node->next = head;
    
    char word_buffer[50];
    int n = 0;

    // Parse a series of integers
    char *token = strtok(line_buffer, " ");
    do {
        int i;
        if (sscanf(token, "%d", &i) != 1) {
            printf("Error parsing token '%s' in word %d\n", token, n + 1);
            exit(EXIT_FAILURE);
        }
        word_buffer[n] = i;

        n++;
    } while (token = strtok(NULL, " "));

    // Store total length
    node->cwrd->len = n;
    node->cwrd->dirty = 1;

    // Allocate string to hold word
    node->cwrd->clets = malloc(n);
    if (!node->cwrd->clets) {
        perror("Failed to allocate word");
        exit(EXIT_FAILURE);
    }

    // Store
    if (memcpy(node->cwrd->clets, word_buffer, n) != node->cwrd->clets) {
        perror("Failed to move memory");
        exit(EXIT_FAILURE);
    }

    // Allocate other fields

    node->cwrd->possible = calloc(sizeof(uint32_t), n);
    if (!node->cwrd->possible) {
        perror("Failed to allocate possible");
        exit(EXIT_FAILURE);
    }

    node->cwrd->known = calloc(1, n);
    if (!node->cwrd->known) {
        perror("Failed to allocate known");
        exit(EXIT_FAILURE);
    }

    node->cwrd->pattern = generatePattern(node->cwrd->clets, node->cwrd->len);

    return node;
}

/**
 * @brief Method for freeing the memory belonging to a puzzle
 */
void freePuzzle(struct puzzle* p) {
    struct cwrd **cws = p->cwrds;
    while (*cws) {
        struct cwrd* cw = *cws;

        free(cw->clets);
        free(cw->possible);
        free(cw->pattern);
        free(cw->known);

        free(cw);

        cws++;
    }

    free(p->cwrds);
    free(p);
}
