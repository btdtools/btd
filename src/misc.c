#include <stdarg.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "misc.h"
#include "btd.h"

void perrordie(char *prg)
{
	perror(prg);
	die("Aborting\n");
}

void die(char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	cleanup();
	exit(EXIT_FAILURE);
}

void *safe_malloc(unsigned long int s)
{
	void *r = malloc(s);
	if (r == NULL)
		perrordie("malloc");
	return r;
}

void safe_free(int count, ...)
{
	va_list ap;
	va_start(ap, count);
	for (int i = 0; i<count; i++)
		free(va_arg(ap, void *));
	va_end(ap);
}

void *safe_realloc(void *p, unsigned long int s)
{
	if ((p = realloc(p, s)) == NULL)
		perrordie("calloc");
	return p;
}

void *safe_calloc(unsigned long int nm, unsigned long int s)
{
	void *r = calloc(nm, s);
	if (r == NULL)
		perrordie("calloc");
	return r;
}

FILE *safe_fopen(char *p, char *mode)
{
	FILE *f = fopen(p, mode);
	if (f == NULL)
		perrordie("fopen");
	return f;
}

void safe_fclose(FILE *f)
{
	if (fclose(f) != 0)
		perrordie("fclose");
}

char *safe_strdup(const char *s)
{
	char *r = strdup(s);
	if (r == NULL)
		perror("strdup");
	return r;
}

char *safe_strcat(int count, ...)
{
	va_list ap;
	va_start(ap, count);
	unsigned long int len = 0;
	for (int i = 0; i<count; i++)
		len += strlen(va_arg(ap, char *));
	va_end(ap);

	va_start(ap, count);
	char *r = safe_malloc(len+1);
	r[0] = '\0';
	for (int i = 0; i<count; i++){
		strcat(r, va_arg(ap, char *));
	}
	va_end(ap);
	return r;
}

void safe_fputs(FILE *f, char *m)
{
	if (fputs(m, f) < 0)
		perrordie("fputs");
}

void safe_fprintf(FILE *f, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (vfprintf(f, fmt, ap) < 0)
		perrordie("vfprintf");
	va_end(ap);
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
		die("glob failed\n");
	} else {
		head = globbuf.gl_pathv[0];
		result = calloc(strlen(head) +
			(tail ? strlen(tail) : 0) + 1, 1);
		strncpy(result, head, strlen(head));
		if (tail)
			strncat(result, tail, strlen(tail));
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
	switch (ai->ai_family){
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
