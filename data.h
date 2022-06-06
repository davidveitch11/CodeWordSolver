
#include "dataStore.h"

void init();
char* generatePattern(char *string, char len);
void newPattern(int n, char *base, char *filled);
char matchPattern(char *pattern, int len, struct pattern *p);
char* nextWord();
