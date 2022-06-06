/**
 * The data store module is responsible for parsing the processed data - the list
 * of words and its index. Call the method read to read the store.
 */

#include "dataStore.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

struct data_store* read() {
    struct data_store* data_store = malloc(sizeof(struct data_store));
    if (!data_store) {
        perror("Failed to allocate data store memory");
        exit(EXIT_FAILURE);
    }

    data_store->words = readWords();
    readPatterns(data_store);

    return data_store;
}

char* readWords() {
    FILE *fp = fopen("data/words", "rb");
    if (!fp) {
        perror("Failed to open words file");
        exit(EXIT_FAILURE);
    }

    // Find end of file, hence find file length
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("Failed to seek to end");
        exit(EXIT_FAILURE);
    }
    long end = ftell(fp);
    if (end < 0) {
        perror("Failed to get end length");
        exit(EXIT_FAILURE);
    }

    // Back to start
    rewind(fp);

    char *words = malloc(end);
    if (!words) {
        perror("Failed to allocate words");
        exit(EXIT_FAILURE);
    }

    if (fread(words, 1, end, fp) != end) {
        perror("Failed to read words");
        exit(EXIT_FAILURE);
    }

    fclose(fp);

    return words;
}

void readPatterns(struct data_store *data_store) {
    FILE *fp = fopen("data/patternIndex", "rb");
    if (!fp) {
        perror("Failed to open pattern index");
        exit(EXIT_FAILURE);
    }

    if (fread(&(data_store->num_patterns), 4, 1, fp) != 1) {
        perror("Failed to read pattern number");
        exit(EXIT_FAILURE);
    }
    data_store->num_patterns = ntohl(data_store->num_patterns);

    data_store->patterns = calloc(sizeof(struct pattern), data_store->num_patterns);
    if (!data_store->patterns) {
        perror("Failed to allocate pattern structure memory");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < data_store->num_patterns; i++) {
        readPattern(fp, data_store->patterns + i, data_store->words);
    }

    fclose(fp);
}

void readPattern(FILE *fp, struct pattern* p, char *words) {
    if (fread(&(p->len), 1, 1, fp) != 1) {
        perror("Failed to read pattern length");
        exit(EXIT_FAILURE);
    }

    p->pattern = calloc(1, p->len);
    if (!p->pattern) {
        perror("Failed to allocate pattern memory");
        exit(EXIT_FAILURE);
    }

    if (fread(p->pattern, 1, p->len, fp) != p->len) {
        perror("Failed to read pattern");
        exit(EXIT_FAILURE);
    }

    // Get start index
    uint32_t start;
    if (fread(&start, 4, 1, fp) != 1) {
        perror("Failed to read start");
        exit(EXIT_FAILURE);
    }
    p->start = words + ntohl(start);

    // Get end
    uint32_t num;
    if (fread(&num, 4, 1, fp) != 1) {
        perror("Failed to read num");
        exit(EXIT_FAILURE);
    }
    p->end = p->start + (ntohl(num) * p->len);
}

void freeDataStore(struct data_store* ds) {
    free(ds->words);

    for (uint32_t i = 0; i < ds->num_patterns; i++) {
        struct pattern* p = ds->patterns + i;
        free(p->pattern);
    }

    free(ds->patterns);

    free(ds);
}
