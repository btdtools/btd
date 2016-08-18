#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <arpa/inet.h>
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

char *safe_strdup(const char *s)
{
	char *r = strdup(s);
	if(r == NULL){
		die("strup() failed");
	}
	return r;
}

bool path_exists(const char *path)
{
	struct stat buf;
	return (stat(path, &buf) == 0);
}

char *resolve_tilde(const char *path) {
	static glob_t globbuf;
	char *head, *tail, *result = NULL;

	tail = strchr(path, '/');
	head = strndup(path, tail ? (size_t)(tail - path) : strlen(path));

	int res = glob(head, GLOB_TILDE, NULL, &globbuf);
	free(head);
	if (res == GLOB_NOMATCH || globbuf.gl_pathc != 1) {
		result = safe_strdup(path);
	} else if (res != 0) {
		die("glob() failed\n");
	} else {
		head = globbuf.gl_pathv[0];
		result = calloc(strlen(head) + (tail ? strlen(tail) : 0) + 1, 1);
		strncpy(result, head, strlen(head));
		if (tail){
			strncat(result, tail, strlen(tail));
		}
	}
	globfree(&globbuf);

	return result;
}

char *parse_str(FILE *stream)
{
	skip_white(stream);
	int size = 32;
	char *buf = malloc(size);
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

void skip_white(FILE *stream)
{
	char c;
	while(isspace(c = fgetc(stream)) && c != EOF);
	if(c != EOF){
		ungetc(c, stream);
	}
}

char *pprint_address(struct addrinfo *ai)
{
	/* decoration + maxlen unix socket and also ipv4 and ipv6 + null*/
	char *r = malloc(20+108+1);
	struct sockaddr_in *inadr;
	struct sockaddr_in6 *in6adr;
	switch(ai->ai_family){
		case AF_UNIX:
			sprintf(r, "Unix domain socket: %s", ai->ai_addr->sa_data);
			break;
		case AF_INET:
			inadr = (struct sockaddr_in*)ai->ai_addr;
			sprintf(r, "TCP(ipv4): %s:%d",
				inet_ntoa(inadr->sin_addr), ntohs(inadr->sin_port));
			break;
		case AF_INET6:
			in6adr = (struct sockaddr_in6*)ai->ai_addr;
			sprintf(r, "TCP(ipv6): [%s]:%d",
				in6adr->sin6_addr.s6_addr, ntohs(in6adr->sin6_port));
			break;
		default:
			sprintf(r, "Unknown socket type: %d", ai->ai_family);
			break;
	}
	return r;
}
