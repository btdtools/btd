#include <stdarg.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "misc.h"

void die(char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
   	fprintf(stderr, msg, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void *safe_malloc(unsigned long int s)
{
	void *r = malloc(s);
	if(r == NULL){
		perror("malloc");
		die("malloc() failed\n");
	}
	return r;
}

FILE *safe_fopen(char *p, char *mode)
{
	FILE *f = fopen(p, mode);
	if(f == NULL){
		perror("fopen");
		die("fopen() failed\n");
	}
	return f;
}

void safe_fclose(FILE *f)
{
	if(fclose(f) != 0){
		perror("fclose");
		die("fclose() failed\n");
	}
}

char *safe_strdup(const char *s)
{
	char *r = strdup(s);
	if(r == NULL){
		die("strup() failed");
	}
	return r;
}

char *safe_strcat(char **ab, int n)
{
	unsigned long int len = 0;
	for(int i = 0; i<n; i++){
		len += strlen(ab[i]);
	}
	char *r = safe_malloc(len+1);
	r[0] = '\0';
	for(int i = 0; i<n; i++){
		strcat(r, ab[i]);
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

char *pprint_address(struct addrinfo *ai)
{
	/* decoration + maxlen unix socket and also ipv4 and ipv6 + null*/
	char *r = safe_malloc(20+108+1);
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
