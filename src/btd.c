#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sqlite3.h>

#include "config.h"
#include "misc.h"
#include "log.h"
#include "db.h"
#include "bibtex.h"

#define MAXCMDLEN 8
#define FDWRITE(fd, str, as...) {\
	char *msgpointer; \
	asprintf(&msgpointer, str, ## as); \
	write(fd, msgpointer, strlen(msgpointer)); \
	free(msgpointer);}

char *PROCOTOLUSAGE =\
	"Protocol specification:\n"\
	"\n"\
	"Commands are case insensitive and space has to be escaped with\\\n"\
	""\
	"Command Args         Info\n"\
	"BYE                  Close the connection gracefully.\n"\
	"NUM                  Print the number of entries available.\n"\
	"SHOW    ID           Show the snippet matching ID.\n"\
	"LIST                 Print a summary.\n"\
	"HELP                 Display this help.\n"\
	"BIBTEX  PATH SNIPPET Add a bibtex snippet to the database in\n"\
	"                     directory DIR and use SNIPPET as the data.\n"\
	"";

struct btd_config *config;
int socket_fd;

void skip_white(FILE *stream)
{
	char c;
	while(isspace(c = fgetc(stream)) && c != EOF);
	if(c != EOF){
		ungetc(c, stream);
	}
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

void cleanup()
{
	btd_log(2, "Closing socket\n");
	close(socket_fd);
	btd_log(2, "Unlinking socket\n");
	unlink(config->socket);
	btd_log(2, "Closing database\n");
	db_close();
}

void sig_handler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM){
		cleanup();
		fflush(stdout);
		depart("Signal %s caught\nQuitting...\n", strsignal(signo));
	}
}

int connection_handler(int fd)
{
	char *cmd = NULL;
	FILE *stream = fdopen(fd, "r");

	while(true) {
		free(cmd);
		cmd = parse_str(stream);
		if(cmd == NULL){
			btd_log(1, "Early EOF?\n");
			break;
		}
		printf("Parsed command: '%s'\n", cmd);
		if(strcasecmp("bibtex", cmd) == 0){
			char *errmsg = NULL;
			char *path = parse_str(stream);
			struct bibtex_object *obj =\
				bibtex_parse(stream, &errmsg, config->check_fields);
			if(obj == NULL){
				FDWRITE(fd, "1\nParsing failed: %s\n", errmsg);
				free(errmsg);
			} else {
				int id = db_add_bibtex(obj, path);
				bibtex_free(obj);
				FDWRITE(fd, "0\nAdded with id: %d\n", id);
			}
			free(path);
		} else if(strcasecmp("num", cmd) == 0){
			FDWRITE(fd, "0\n%d\n", db_num());
		} else if(strcasecmp("show", cmd) == 0){
			char *num_str = parse_str(stream);
			long long int num = strtoll(num_str, NULL, 10);
			if(num <= 0){
				FDWRITE(fd, "1\nNumber should be positive\n");
			} else {
				char *bibtex_str = db_get(num);
				if(bibtex_str == NULL){
					FDWRITE(fd, "1\nNumber not a valid ID\n");
				} else {
					FDWRITE(fd, "0\n%s\n", bibtex_str);
					free(bibtex_str);
				}
			}
			free(num_str);
		} else if(strcasecmp("list", cmd) == 0){
			FDWRITE(fd, "0\n");
			db_list(fd);
		} else if(strcasecmp("bye", cmd) == 0){
			FDWRITE(fd, "0\nbye\n");
			break;
		} else if(strcasecmp("help", cmd) == 0){
			FDWRITE(fd, "0\n%s\n", PROCOTOLUSAGE);
		} else {
			FDWRITE(fd, "1\nUnknown command: '%s'\n", cmd);
		}
	}
	btd_log(1, "Closing client...\n");
	if(close(fd) != 0) {
		perror("close");
		return 1;
	}
	return 0;
}

int main (int argc, char **argv)
{
	struct sockaddr_un address;
	int connection_fd;
	socklen_t address_length;
	pid_t child;

	btd_init_log();

	/* Register signal handlers */
	if(signal(SIGINT, sig_handler) == SIG_ERR){
		die("Can't catch SIGINT\n");
	}
	if(signal(SIGTERM, sig_handler) == SIG_ERR){
		die("Can't catch SIGTERM\n");
	}

	/* Parse args and config */
	config = malloc(sizeof (struct btd_config));
	btd_config_populate(config, argc, argv);
	printf("Config parsing done\n");
	btd_config_print(config, stdout);

	/* Init db */
	db_init(config);

	/* Setup socket */
	btd_log(2, "Registering socket\n");
	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0) {
		perror("socket");
		die("Bye\n");
	} 
	btd_log(2, "Registered socket\n");
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, config->socket);

	if(bind(socket_fd, (struct sockaddr *) &address, 
			sizeof(struct sockaddr_un)) != 0) {
		perror("bind");
		die("Bye\n");
	}
	btd_log(2, "Bound socket\n");

	if(listen(socket_fd, 5) != 0) {
		perror("listen");
		die("Bye\n");
	}
	btd_log(2, "Listening to socket\n");

	btd_log(1, "Waiting for a client to connect\n");
	fflush(stdout);
	while((connection_fd = accept(socket_fd, (struct sockaddr *) &address,
			&address_length)) > -1) {
		btd_log(1, "Client connected...\n");
		child = fork();
		if(child == 0) {
			return connection_handler(connection_fd);
		}
		close(connection_fd);
	}
	depart("Bye...\n");
	cleanup();
}
