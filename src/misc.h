#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include <netdb.h>

#define die(fmt, as...) printf(fmt, ## as); exit(EXIT_FAILURE);
#define depart(fmt, as...) printf(fmt, ## as); exit(EXIT_SUCCESS);

#define VERSION "0.1"

char *ltrim(char *s);
char *rtrim(char *s);
char *safe_strdup(const char *s);
bool path_exists(const char *path);
char *resolve_tilde(const char *path);

char *parse_str(FILE *stream);
void skip_white(FILE *stream);

char *pprint_address(struct addrinfo *ai);
#endif
