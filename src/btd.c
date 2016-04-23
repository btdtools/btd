#define _GNU_SOURCE

#include <ctype.h>
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

struct btd_config *config;
int socket_fd;

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
	char cmdbuf[MAXCMDLEN+1];
	int cmdbuf_pos;
	bool stop = false;

	while(!stop) {
		cmdbuf_pos = 0;
		do {
			if(read(fd, ((char *)cmdbuf)+cmdbuf_pos, 1) != 1){
				printf("early eof?");
				stop = true;
				break;
			}
		} while(isalpha(cmdbuf[cmdbuf_pos++]) && cmdbuf_pos<MAXCMDLEN);
		if(cmdbuf_pos >= MAXCMDLEN){
			FDWRITE(fd, "err Command exceeded maximum length\n");
			char c;
			do {
				read(fd, &c, 1);
			} while(c != EOF && c != '\n');
		} else if(!isspace(cmdbuf[cmdbuf_pos-1])){
			FDWRITE(fd, "err Command should be terminated by a space\n");
		} else {
			cmdbuf[cmdbuf_pos-1] = '\0';
			printf("Parsed command: '%s'\n", cmdbuf);
			if(strcmp("bibtex", cmdbuf) == 0){
				FILE *stream = fdopen(fd, "r");
				char *errmsg = NULL;
				struct bibtex_object *obj = bibtex_parse(stream, &errmsg);
				if(obj == NULL){
					FDWRITE(fd, "Parsing failed\n");
					FDWRITE(fd, errmsg);
					free(errmsg);
				} else {
					int id = db_add_bibtex(obj, "");
					bibtex_free(obj);
					FDWRITE(fd, "%d\n", id);
				}
			} else if(strcmp("bye", cmdbuf) == 0){
				FDWRITE(fd, "bye\n");
				stop = true;
			} else {
				FDWRITE(fd, "err Unknown command: '%s'\n", cmdbuf);
			}
		}
	};
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
