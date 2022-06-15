/**
 * The data store module is responsible for parsing the processed data - the list
 * of words and its index. Call the method read to read the store.
 * 
 * Compiles with link: '-l ws2_32'
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

/**
 * @brief Load the data into a new data structure and return it
 */
struct data_store* read() {
    // Allocate memory
    struct data_store* data_store = malloc(sizeof(struct data_store));
    if (!data_store) {
        perror("Failed to allocate data store memory");
        exit(EXIT_FAILURE);
    }

    // Fill values
    data_store->words = readWords();
    readPatterns(data_store);

    return data_store;
}

/**
 * @brief Read the entire word list into memory and return the start location
 */
char* readWords() {
    // Open the file for reading
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

    // Go back to start
    rewind(fp);

    // Allocate area in memory to hold words
    char *words = malloc(end);
    if (!words) {
        perror("Failed to allocate words");
        exit(EXIT_FAILURE);
    }

    // Load words list into location
    if (fread(words, 1, end, fp) != end) {
        perror("Failed to read words");
        exit(EXIT_FAILURE);
    }

    fclose(fp);

    return words;
}

/**
 * @brief Read and parse the patternIndex file. Store it in the data_store object
 */
void readPatterns(struct data_store *data_store) {
    // Open file for reading
    FILE *fp = fopen("data/patternIndex", "rb");
    if (!fp) {
        perror("Failed to open pattern index");
        exit(EXIT_FAILURE);
    }

    // Read the number of patterns, convert byte ordering
    if (fread(&(data_store->num_patterns), 4, 1, fp) != 1) {
        perror("Failed to read pattern number");
        exit(EXIT_FAILURE);
    }
    data_store->num_patterns = ntohl(data_store->num_patterns);

    // Allocate array to hold the patterns
    data_store->patterns = calloc(sizeof(struct pattern), data_store->num_patterns);
    if (!data_store->patterns) {
        perror("Failed to allocate pattern structure memory");
        exit(EXIT_FAILURE);
    }

    // Read each pattern into the allocated array
    for (uint32_t i = 0; i < data_store->num_patterns; i++) {
        readPattern(fp, data_store->patterns + i, data_store->words);
    }

    fclose(fp);
}

/**
 * @brief Read a single pattern record from the current place in the file to the location given
 * @param fp file to read record from
 * @param p pattern to load record into
 * @param words start location of the full list of words
 */
void readPattern(FILE *fp, struct pattern* p, char *words) {
    // Read the length of the pattern - the number of characters in the pattern
    if (fread(&(p->len), 1, 1, fp) != 1) {
        perror("Failed to read pattern length");
        exit(EXIT_FAILURE);
    }

    // Allocate memory to hold the pattern
    p->pattern = calloc(1, p->len);
    if (!p->pattern) {
        perror("Failed to allocate pattern memory");
        exit(EXIT_FAILURE);
    }

    // Read the pattern into the memory allocated
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

/**
 * @brief Function for freeing a datastore object and all internal structures
 */
void freeDataStore(struct data_store* ds) {
    free(ds->words);

    for (uint32_t i = 0; i < ds->num_patterns; i++) {
        struct pattern* p = ds->patterns + i;
        free(p->pattern);
    }

    free(ds->patterns);

    free(ds);
}
