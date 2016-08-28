#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>

#define die(fmt) puts(fmt); exit(EXIT_FAILURE);
#define dief(fmt, ...) printf(fmt, __VA_ARGS__); exit(EXIT_FAILURE);
#define depart(fmt) puts(fmt); exit(EXIT_SUCCESS);
#define departf(fmt, ...) printf(fmt, __VA_ARGS__); exit(EXIT_SUCCESS);

#define VERSION "0.1"

void *safe_malloc(unsigned long int s);
FILE *safe_fopen(char *p, char *mode);
void safe_fclose(FILE *f);
char *ltrim(char *s);
char *rtrim(char *s);
char *safe_strcat(char **ab, int n);
char *safe_strdup(const char *s);
bool path_exists(const char *path);
char *resolve_tilde(const char *path);

char *parse_str(FILE *stream);
void skip_white(FILE *stream);

char *pprint_address(struct addrinfo *ai);
#endif
