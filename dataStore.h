
#ifndef DATA_STORE_H_
#define DATA_STORE_H_

#include <inttypes.h>
#include <stdio.h>

struct data_store {
    uint32_t num_patterns;
    struct pattern* patterns;
    char *words;
};

struct pattern {
    char len;
    char *pattern;
    char *start;
    char *end; // location of byte immediately following the last entry
};

struct data_store* read();
char* readWords();
void readPatterns(struct data_store *data_store);
void readPattern(FILE *fp, struct pattern* p, char *words);
void freeDataStore(struct data_store* ds);

#endif
