#ifndef MISC_H
#define MISC_H

#include <stdbool.h>

#define die(msg) printf("%s\n", msg); exit(EXIT_FAILURE);
#define depart(msg) printf("%s\n", msg); exit(EXIT_SUCCESS);

char *ltrim(char *s);
char *rtrim(char *s);
char *safe_strdup(const char *s);
bool path_exists(const char *path);

#endif
