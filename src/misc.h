#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>

#define VERSION "0.1"

void die(char *msg, ...);
void *safe_malloc(unsigned long int s);
FILE *safe_fopen(char *p, char *mode);
void safe_fclose(FILE *f);
char *safe_strcat(char **ab, int n);
char *safe_strdup(const char *s);
bool path_exists(const char *path);
char *resolve_tilde(const char *path);

char *pprint_address(struct addrinfo *ai);
#endif
