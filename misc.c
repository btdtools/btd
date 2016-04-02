#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "misc.h"

char *ltrim(char *s){
	while(isspace(*s)){
		s++;
	}
	return s;
}

char *rtrim(char *s){
	char *end = s + strlen(s);
	while((end != s) && isspace(*(end-1))){
		end--;
	}
	*end = '\0';
	return s;
}

char *safe_strdup(const char *s){
	char *r = strdup(s);
	if(r == NULL){
		die("strup() failed");
	}
	return r;
}
