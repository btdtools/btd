#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "parse.h"
#include "misc.h"

char *ltrim(char *s)
{
	while(isspace(*s)){
		s++;
	}
	return s;
}

char *rtrim(char *s)
{
	char *end = s + strlen(s);
	while((end != s) && isspace(*(end-1))){
		end--;
	}
	*end = '\0';
	return s;
}

char *parse_str(FILE *stream)
{
	skip_white(stream);
	int size = 32;
	char *buf = safe_malloc(size);
	int position = 0;
	char c;

	while(!isspace(c = fgetc(stream)) && c != EOF){
		if(c == '\\'){
			c = fgetc(stream);
		}
		buf[position++] = c;
		if(position >= size-1){
			if((buf = realloc(buf, size *= 2)) == NULL){
				perror("realloc");
				die("Realloc failed...\n");
			}
		}
	}
	if(c == EOF){
		free(buf);
		return NULL;
	}
	buf[position] = '\0';
	return buf;
}

bool parse_llint(FILE *stream, long long int *r)
{
	char *ref, *str = parse_str(stream);
	printf("Trying to parse '%s' as a num\n", str);
	*r = strtoll(str, &ref, 10);
	if(*ref != '\0'){
		fprintf(stream, "1\n'%s' is not a number\n", str);
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
	while(isspace(c = fgetc(stream)) && c != EOF);
	if(c != EOF){
		ungetc(c, stream);
	}
}
