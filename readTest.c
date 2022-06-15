/**
 * Test file for word list, reads the data stored in directory "data" created using
 * makeData. It will output the data in the same form as output by adding flag
 * 'test' to makeData. Both output to stdout.
 */

#include "dataStore.h"

int main(int argc, char **argv) {
    struct data_store* ds = read();

    printf("Number of patterns: %d\n", ds->num_patterns);

    for (uint32_t i = 0; i < ds->num_patterns; i++) {
        struct pattern *p = ds->patterns + i;

        printf("'");
        for (char j = 0; j < p->len; j++) {
            printf("%d ", *(p->pattern + j));
        }
        printf("'\n");

        char *start = p->start;

        while (start < p->end) {
            printf("\t");

            for (char j = 0; j < p->len; j++) {
                printf("%c", *(start + j));
            }

            printf("\n");

            start += p->len;
        }
    }

    freeDataStore(ds);

    return 0;
}
