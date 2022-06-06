
struct word_list_node {
    char *word;
    struct word_list_node* next;
};

struct pattern_list_node {
    char len;
    char *pattern;
    struct word_list_node* list;
    struct pattern_list_node* next;
    uint32_t num;
};

void input(char *file_name);
void get_pattern();
void add_pattern();
void new_pattern_node(int n);
void add_word_to_pattern(struct pattern_list_node* node, int n);
void outputStd();
void outputFile();

#define DEBUG 0
