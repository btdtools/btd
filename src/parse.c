#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "parse.h"
#include "misc.h"

char *ltrim(char *s)
{
	while(isspace(*s))
		s++;
	return s;
}

char *rtrim(char *s)
{
	char *end = s + strlen(s);
	while((end != s) && isspace(*(end-1)))
		end--;
	*end = '\0';
	return s;
}

char *parse_str(FILE *stream)
{
	char c = skip_white(stream);
	int size = 32;
	char *buf = safe_malloc(size);
	int position = 0;
	if(c != EOF)
		buf[position++] = c;

	while(!isspace(c = fgetc(stream)) && c != EOF){
		if(c == '\\'){
			switch(c = fgetc(stream)){
			case 'n':
				c = '\n';
				break;
			}
		}
		buf[position++] = c;
		if(position >= size-1)
			if((buf = realloc(buf, size *= 2)) == NULL)
				perrordie("realloc");
	}
	buf[position] = '\0';
	return buf;
}

bool parse_llint(FILE *stream, long int *r)
{
	char *ref, *str = parse_str(stream);
	*r = strtoll(str, &ref, 10);
	if(*ref != '\0'){
		safe_fprintf(stream, "1\n'%s' is not a number\n", str);
		free(str);
		return false;
	} else {
		free(str);
		return true;
	}
}

char skip_white(FILE *stream)
{
	char c;
	while (isspace(c = fgetc(stream)) && c != EOF);
	return c;
}
