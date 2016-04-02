#ifndef MISC_H
#define MISC_H

#define die(msg) printf("%s\n", msg); exit(EXIT_FAILURE);
#define depart(msg) printf("%s\n", msg); exit(EXIT_SUCCESS);

char *ltrim(char *s);
char *rtrim(char *s);
char *safe_strdup(const char *s);

#endif
