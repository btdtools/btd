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
#include "parse.h"
#include "libbtd.h"
#include "db.h"
#include "bibtex.h"
#include "doc.h"

struct btd_config *config;
int socket_fd;

void cleanup()
{
	btd_log(2, "Closing socket\n");
	close(socket_fd);
	btd_log(2, "Unlinking socket\n");
	if (config->socket->ai_family == AF_UNIX)
		unlink(config->socket->ai_addr->sa_data);
	btd_log(2, "Freeing config\n");
	btd_config_free(config);
	btd_log(2, "Closing database\n");
	db_close();
}

void sig_handler(int signo)
{
	if (signo == SIGINT || signo == SIGTERM || signo == SIGPIPE){
		cleanup();
		btd_log(0, "Signal %s caught\nQuitting...\n", strsignal(signo));
		exit(EXIT_SUCCESS);
	}
}

int connection_handler(int fd)
{
	char *cmd = NULL;
	FILE *stream = fdopen(fd, "w+");
	if (stream == NULL)
		perrordie("fdopen");
	safe_fprintf(stream, "btd %s\n", VERSION);
	setbuf(stream, NULL);

	while (true) {
		free(cmd);
		btd_log(1, "Going to parse command\n");
		cmd = parse_str(stream);
		btd_log(1, "Parsed command: '%s'\n", cmd);
		if (strcasecmp("bibtex", cmd) == 0){
			char *errmsg = NULL;
			char *path = parse_str(stream);
			btd_log(2, "path parsed: %s\n", path);
			struct bibtex_object *obj = bibtex_parse(
				stream, &errmsg, config->check_fields);
			btd_log(2, "bibtex parsed: %p\n", (void *)obj);
			if (obj == NULL){
				safe_fprintf(stream,
					"1\nParsing failed: %s\n", errmsg);
			} else {
				int id = db_add_bibtex(obj, path);
				bibtex_free(obj);
				safe_fprintf(stream,
					"0\nAdded with id: %d\n", id);
			}
			free(path);
			free(errmsg);
		} else if (strcasecmp("num", cmd) == 0){
			safe_fprintf(stream, "0\n%d\n", db_num());
		} else if (strcasecmp("show", cmd) == 0){
			long int num;
			if (parse_llint(stream, &num)){
				char *bibtex_str = db_get(num);
				if (bibtex_str == NULL){
					safe_fputs(stream,
						"1\nNumber not a known ID\n");
				} else {
					safe_fprintf(stream,
						"0\n%s\n", bibtex_str);
					free(bibtex_str);
				}
			} else {
				safe_fputs(stream,
					"1\nArgument is not a number");
			}
		} else if (strcasecmp("detach", cmd) == 0){
			long int num;
			if (parse_llint(stream, &num))
				db_file_remove(num, stream);
			else
				safe_fputs(stream,
					"1\nArgument is not a number");
		} else if (strcasecmp("upload", cmd) == 0){
			char *fn = parse_str(stream);
			long int length;
			if (parse_llint(stream, &length))
				db_file_upload(fn, length, stream);
			else
				safe_fputs(stream,
					"1\nArgument not a number");
			free(fn);
		} else if (strcasecmp("list", cmd) == 0){
			safe_fputs(stream, "0\n");
			db_list(stream);
		} else if (strcasecmp("files", cmd) == 0){
			btd_log(2, "Getting files\n");
			safe_fputs(stream, "0\n");
			db_file_list(stream);
		} else if (strcasecmp("bye", cmd) == 0 || strlen(cmd) == 0){
			safe_fputs(stream, "0\nbye\n");
			break;
		} else if (strcasecmp("help", cmd) == 0){
			safe_fprintf(stream, "0\n%s\n", protocol_doc);
		} else {
			safe_fprintf(stream, "1\nUnknown command: '%s'\n", cmd);
		}
	}
	free(cmd);
	fflush(stream);
	btd_log(1, "Closing client...\n");
	fclose(stream);
	return 0;
}

int main(int argc, char **argv)
{
	int connection_fd;
	pid_t me = getpid();

	btd_init_log();

	/* Register signal handlers */
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		perrordie("signal(SIGINT)");
	if (signal(SIGTERM, sig_handler) == SIG_ERR)
		perrordie("signal(SIGTERM)");
	if (signal(SIGPIPE, sig_handler) == SIG_ERR)
		perrordie("signal(SIGPIPE)");

	/* Parse args and config */
	config = safe_malloc(sizeof (struct btd_config));
	btd_config_populate(config, argc, argv);
	btd_log(2, "Config parsing done\n");
	btd_config_print(config, stdout);

	if (strlen(config->pidfile) > 0){
		btd_log(2, "Writing pidfile at %s\n", config->pidfile);
		FILE *pidfile = safe_fopen(config->pidfile, "w");
		safe_fprintf(pidfile, "%d", me);
		safe_fclose(pidfile);
	}

	/* Init db */
	db_init(config);

	/* Setup socket */
	btd_log(2, "Registering socket\n");
	for (struct addrinfo *r = config->socket; r != NULL; r=r->ai_next){
		char *t = pprint_address(r);
		btd_log(0, "Trying to connect to: %s\n", t);
		free(t);
		socket_fd = socket(
			r->ai_family, r->ai_socktype, r->ai_protocol);

		if (socket_fd < 0){
			perror("socket");
			continue;
		}
		btd_log(2, "Registered socket\n");

		if (bind(socket_fd, r->ai_addr, r->ai_addrlen) != 0){
			perror("bind");
		} else {
			btd_log(2, "Bound socket\n");

			if (listen(socket_fd, 5) != 0)
				perrordie("listen");
			btd_log(2, "Listening to socket\n");
			btd_log(1, "Waiting for a client to connect\n");
			while ((connection_fd = accept(socket_fd,
					r->ai_addr, &r->ai_addrlen)) > -1) {
				btd_log(1, "Client connected...\n");
				if (config->multithread){
					if (fork() == 0)
						return connection_handler(
							connection_fd);
				} else {
					connection_handler(connection_fd);
				}
				close(connection_fd);
			}
			break;
		}
		close(socket_fd);
	}
	btd_log(0, "Couldn't bind any socket...\n");
	cleanup();
}
