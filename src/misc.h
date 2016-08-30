#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>

#define VERSION "0.1"

void perrordie(char *prg);
void die(char *msg, ...);
void *safe_calloc(unsigned long int nm, unsigned long int s);
void *safe_malloc(unsigned long int s);
FILE *safe_fopen(char *p, char *mode);
void safe_fclose(FILE *f);
char *safe_strcat(int count, ...);
char *safe_strdup(const char *s);
void safe_fputs(FILE *f, char *m);
void safe_fprintf(FILE *f, char *m, ...);
bool path_exists(const char *path);
char *resolve_tilde(const char *path);

char *pprint_address(struct addrinfo *ai);
#endif
