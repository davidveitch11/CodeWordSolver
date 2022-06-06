/**
 * This script will read in a dictionray of words ('\n\r' separated) 
 * from the directory "data" and produce a compatible data store in
 * the same directory.
 * 
 * Compiles with link: '-l ws2_32'
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "makeData.h"

struct pattern_list_node* list = NULL;
uint32_t num_patterns = 0;

char buffer[100];
char pattern_buffer[100];

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        printf("Usage: $ %s <file_name> [test]\n", argv[0]);
        printf("Where file_name is the name of the dictionary file\n");
        printf("Include test flag to output to stdout a verbose representation\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 3 && strcmp(argv[2], "test") != 0) {
        printf("Unrecognised second argument '%s'\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    input(argv[1]);

    if (argc == 3) {
        outputStd();
    } else {
        outputFile();
    }

    return 0;
}

void input(char *file_name) {
    FILE* fp = fopen(file_name, "r");
    if (!fp) {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, 100, fp)) {
        get_pattern();
        add_pattern();
    }

    fclose(fp);
}

void get_pattern() {
    // Cut off line terminator
    char *b = buffer;
    while (*b) {
        if (*b == '\n' || *b == '\r') {
            *b = 0;
            break;
        }

        b++;
    }

    // Count frequency of letters in word
    char alpha[26] = {0};
    for (int i = 0; buffer[i]; i++) {
        alpha[buffer[i] - 'a']++;
    }

    #if DEBUG
        printf("%s\n", buffer);
        for (int i = 0; i < 26; i++) {
            printf("%d", alpha[i]);
        }
        printf("\n");
        for (int i = 0; i < 26; i++) {
            printf("%c", i + 'a');
        }
        printf("\n");
    #endif

    // Group counters
    char group = 1;
    char groups[26] = {0};

    // Assign letters to groups
    for (int i = 0; buffer[i]; i++) {
        int n = buffer[i] - 'a';

        #if DEBUG
            printf("%c %d ", buffer[i], n);
        #endif

        if (alpha[n] == 1) {
            // Only one - default group
            pattern_buffer[i] = 0;

            #if DEBUG
                printf("default\n");
            #endif
        } else {
            // More than one - assign group

            // New group
            if (groups[n] == 0) {
                groups[n] = group++;

                #if DEBUG
                    printf("new group ");
                #endif
            }

            // Assign
            pattern_buffer[i] = groups[n];

            #if DEBUG
                printf("group %d\n", groups[n]);
            #endif
        }
    }
}

void add_pattern() {
    int n = strlen(buffer);

    #if DEBUG
        for (int i = 0; i < n; i++) {
            printf("%d ", pattern_buffer[i]);
        }
        printf("\n");
    #endif

    struct pattern_list_node* head = list;
    while (head) {
        #if DEBUG
            for (int i = 0; i < n; i++) {
                printf("%d ", head->pattern[i]);
            }
            printf("\n");
        #endif

        if (head->len == n && memcmp(pattern_buffer, head->pattern, n) == 0) {
            // Found location
            add_word_to_pattern(head, n);

            return;
        }

        head = head->next;
    }

    new_pattern_node(n);
    add_word_to_pattern(list, n);
}

void new_pattern_node(int n) {
    num_patterns++;

    struct pattern_list_node* node = malloc(sizeof(struct pattern_list_node));
    if (!node) {
        perror("Failed to allocate pattern list node");
        exit(EXIT_FAILURE);
    }

    node->len = n;
    node->next = list;
    node->list = NULL;
    node->pattern = malloc(n);
    node->num = 0;

    if (!node->pattern) {
        perror("Failed to allocate string for pattern");
        exit(EXIT_FAILURE);
    }

    memcpy(node->pattern, pattern_buffer, n);

    list = node;

    #if DEBUG
        printf("new pattern node %d ", list->num);
        for (int i = 0; i < list->len; i++) {
            printf("%d ", list->pattern[i]);
        }
        printf("\n");
    #endif
}

void add_word_to_pattern(struct pattern_list_node* node, int n) {
    struct word_list_node* wln = malloc(sizeof(struct word_list_node));
    if (!wln) {
        perror("Failed to allocate word list node");
        exit(EXIT_FAILURE);
    }

    wln->next = node->list;
    node->list = wln;
    
    node->num++;

    wln->word = malloc(n + 1);
    if (!wln->word) {
        perror("Failed to allocate string for word");
        exit(EXIT_FAILURE);
    }
    
    memcpy(wln->word, buffer, n);
    wln->word[n] = 0;

    #if DEBUG
        printf("new word node %s\n", node->list->word);
    #endif
}

void outputStd() {
    while (list) {
        printf("'");
        for (int i = 0; i < list->len; i++) {
            printf("%d ", list->pattern[i]);
        }
        printf("' (%d words)\n", list->num);

        while (list->list) {
            printf("\t%s\n", list->list->word);

            struct word_list_node* next = list->list->next;

            free(list->list->word);
            free(list->list);

            list->list = next;
        }

        struct pattern_list_node* next = list->next;

        free(list->pattern);
        free(list);

        list = next;
    }
}

void outputFile() {
    FILE *wf = fopen("data/words", "wb");
    if (!wf) {
        perror("Failed to open words file");
        exit(EXIT_FAILURE);
    }

    FILE *pf = fopen("data/patternIndex", "wb");
    if (!pf) {
        perror("Failed to open patternIndex file");
        exit(EXIT_FAILURE);
    }

    // Write number of patterns to pattern index file
    num_patterns = htonl(num_patterns);
    if (fwrite(&num_patterns, 4, 1, pf) != 1) {
        perror("Failed to write num_patterns");
        exit(EXIT_FAILURE);
    }
    num_patterns = ntohl(num_patterns);

    uint32_t wi = 0;

    while (list) {
        // Write the pattern record

        // length of pattern
        // fwrite(&(list->len), 1, 1, stdout);
        // printf("write len\n");
        if (fwrite(&(list->len), 1, 1, pf) != 1) {
            perror("Failed to write len");
            exit(EXIT_FAILURE);
        }

        // pattern
        // fwrite(list->pattern, 1, list->len, stdout);
        // printf("write pattern\n");
        if (fwrite(list->pattern, 1, list->len, pf) != list->len) {
            perror("Failed to write pattern");
            exit(EXIT_FAILURE);
        }

        #if DEBUG
            for (int i = 0; i < list->len; i++) {
                printf("%d ", list->pattern[i]);
            }
            printf("written\n");
        #endif

        // index
        wi = htonl(wi);
        // fwrite(&wi, 4, 1, stdout);
        // printf("write index\n");
        if (fwrite(&wi, 4, 1, pf) != 1) {
            perror("Failed to write index");
            exit(EXIT_FAILURE);
        }
        wi = ntohl(wi);

        #if DEBUG
            printf("wi %d %x\n", wi, *(&wi));
        #endif

        // number of words
        list->num = htonl(list->num);
        // fwrite(&(list->num), 4, 1, stdout);
        // printf("write num\n");
        if (fwrite(&(list->num), 4, 1, pf) != 1) {
            perror("Failed to write num");
            exit(EXIT_FAILURE);
        }
        list->num = ntohl(list->num);

        // Write list of words

        while (list->list) {
            // write word
            if (fwrite(list->list->word, 1, list->len, wf) != list->len) {
                perror("Failed to write word");
                exit(EXIT_FAILURE);
            }

            // free memory, next
            struct word_list_node* next = list->list->next;
            free(list->list->word);
            free(list->list);
            list->list = next;
        }

        // advance word list pointer
        wi += list->num * list->len;

        // Free memory and move to next

        struct pattern_list_node* next = list->next;
        free(list->pattern);
        free(list);
        list = next;
    }

    fclose(pf);
    fclose(wf);
}
