#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "parse.h"
#include "misc.h"
#include "log.h"

char *ltrim(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

char *rtrim(char *s)
{
	char *end = s + strlen(s);
	while ((end != s) && isspace(*(end-1)))
		end--;
	*end = '\0';
	return s;
}

static int parse_num_escape(FILE *fp, int maxlen, int base, int (*comp)(int))
{
	char *b = safe_calloc(maxlen+1, 1);
	int c, i;
	for (i = 0; i<maxlen && comp(c = fgetc(fp)); i++)
		b[i] = c;
	if (i < maxlen)
		ungetc(c, fp);
	c = strtol(b, NULL, base);
	free(b);
	return c;
}

char *parse_str(FILE *stream)
{
	int position = 0, size = 32;
	char c, *buf = safe_malloc(size);

	skip_white(stream);
	while ((c = fgetc(stream)) != EOF && !isspace(c)){
		if (c == EOF)
			break;	

		if (c == '\\'){
			if((c = fgetc(stream)) == 'a')
				c = '\a';
			else if(c == 'b')
				c = '\b';
			else if(c == 'f')
				c = '\f';
			else if(c == 'n')
				c = '\n';
			else if(c == 'r')
				c = '\r';
			else if(c == 't')
				c = '\t';
			else if(c == 'v')
				c = '\v';
			else if(c == '0')
				c = parse_num_escape(stream, 3, 8, isdigit);
			else if(c == 'x')
				c = parse_num_escape(stream, 2, 16, isxdigit);
		}

		buf[position++] = c;
		if (position >= size-1)
			if ((buf = realloc(buf, size *= 2)) == NULL)
				perrordie("realloc");
	}
	buf[position] = '\0';
	return buf;
}

bool parse_llint(FILE *stream, long int *r)
{
	char *ref, *str = parse_str(stream);
	*r = strtoll(str, &ref, 10);
	if (*ref != '\0'){
		safe_fprintf(stream, "1\n'%s' is not a number\n", str);
		free(str);
		return false;
	} else {
		free(str);
		return true;
	}
}

void skip_white(FILE *stream)
{
	char c;
	while (isspace(c = fgetc(stream)) && c != EOF);
	if (c != EOF)
		ungetc(c, stream);
}
