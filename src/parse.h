#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include <stdio.h>

#define VERSION "0.1"

char *ltrim(char *s);
char *rtrim(char *s);

char *parse_str(FILE *stream);
bool parse_llint(FILE *stream, long int *r);
void skip_white(FILE *stream);

#endif
