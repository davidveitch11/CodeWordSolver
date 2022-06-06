
#include <inttypes.h>
#include <stdio.h>

// Code word structure
struct cwrd {
    int len; // number of code letters
    char* clets; // ref to code letter array
    uint32_t* possible;
    char dirty; // Set to 1 if any number has been solved (pattern changed)

    char* pattern; // reprentation of the pattern for this code word
    char* known; // pattern but without any filled in values
};

struct puzzle {
    struct cwrd** cwrds;
    char known[26];
    char needed[26];
};

struct puzzle* parse(char *filename);
char parseKnown(FILE *fp, char *known);
struct cwrd_node* parseWord(FILE *fp, struct cwrd_node* head);
void freePuzzle(struct puzzle* p);
