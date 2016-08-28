#include <argp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "config.h"
#include "log.h"
#include "misc.h"
#include "parse.h"
#include "xdg.h"

const char *argp_program_version = "btd 0.1";
const char *argp_program_bug_address = "<mart@martlubbers.net>";

static char doc[] = "btd -- A BibTex daemon";
static char args_doc[] = "[~/.config/btd/config]";

static struct argp_option options[] = {
	{"verbose",'v', 0, 0, "Increase verbosity", 0},
	{"quiet",'q', 0, 0, "Decrease verbosity", 0},
	{0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct btd_config *config = state->input;

	switch (key)
	{
		case 'q':
			btd_decr_log();
			break;
		case 'v':
			btd_incr_log();
			break;
		case ARGP_KEY_ARG:
			config->configpath = arg;
			break;
		case ARGP_KEY_END:
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = {
	.options=options, 
	.parser=parse_opt,
	.args_doc=args_doc,
	.doc=doc,
	.children=NULL,
	.help_filter=NULL,
	.argp_domain=NULL};

static void create_unixsocket(struct btd_config *config, char *path)
{
	if(strlen(path) > 108){
		btd_log(0, "Path is too long(%lu), UNIX socket can be 108 max\n",
			strlen(path));
		die("Socket creation failed\n");
	}
	char *sa = resolve_tilde(path);

	config->socket = (struct addrinfo *)safe_malloc(sizeof(struct addrinfo));
    memset(config->socket, 0, sizeof(struct addrinfo));
	config->socket->ai_family = AF_UNIX;
	config->socket->ai_socktype = SOCK_STREAM;
	config->socket->ai_protocol = 0;

	/* Build address object */
	config->socket->ai_addr = safe_malloc(sizeof(struct sockaddr_un));
	memset(config->socket->ai_addr, 0, sizeof(struct sockaddr_un));
	config->socket->ai_addr->sa_family = AF_UNIX;
	strcpy(config->socket->ai_addr->sa_data, sa);

	/* Register length */
	config->socket->ai_addrlen = sizeof(struct sockaddr_un);

	free(sa);
}

void update_config(struct btd_config *config, char *key, char *value){
	key = rtrim(ltrim(key));

	/* If the key is empty or the line starts with a comment we skip */
	if (key[0] == '#' || strlen(key) == 0){
		return;
	}

	/* If the key contains a comment we skip */
	if (strchr("key", (char)'#') != NULL){
		return;
	}
	value[strcspn(value, "#")] = '\0';
	value = rtrim(ltrim(value));

	/* If the value stripped of comments is empty we skip */
	if (strlen(value) == 0){
		return;
	}

	/* Configuration options */
	if (strcmp(key, "socket") == 0){
		printf("Trying to solve: '%s'\n", value);
		int portindex = -1;
		for(int i = strlen(value)-1; i>=0; i--){
			if(value[i] == ':'){
				char *end;
				strtol(value+i+1, &end, 10);
				if(value+i+1 != end){
					portindex = i+1;
					value[i] = '\0';
					btd_log(2, "Found port spec: %s\n", value+portindex);
					if(value[0] == '[' && value[strlen(value)-1] == ']'){
						btd_log(2, "Stripping ipv6 square braces\n");
						value = value+1;
						value[strlen(value)-1] = '\0';
					}
					break;
				} 
			}
		}
		struct addrinfo hints;
        struct addrinfo *result;
		int s = 0;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

		btd_log(2, "Address without port: %s\n", value);
		if(portindex == -1){
			btd_log(2, "no port found so treating it as a unix socket\n");
			create_unixsocket(config, value);
		} else {
			if((s = getaddrinfo(value, value+portindex, &hints, &result)) != 0) {
				btd_log(2, "getaddrinfo returned %s, "
					"treating it as an unix socket\n", gai_strerror(s));
				value[portindex-1] = ':';
				create_unixsocket(config, value);
			} else {
				btd_log(2, "Succesfully parsed ipv4 or ipv6 addresses\n");
				config->socket = result;
			}
		}
	} else if (strcmp(key, "datadir") == 0){
		config->datadir = resolve_tilde(value);
	} else if (strcmp(key, "filefmt") == 0){
		config->filefmt = resolve_tilde(value);
	} else if (strcmp(key, "pidfile") == 0){
		config->pidfile = safe_strdup(value);
	} else if (strcmp(key, "check_fields") == 0){
		if(strcmp(value, "true") != 0 && strcmp(value, "false") != 0){
			die("check_fields can either be 'true' or 'false'\n");
		}
		config->check_fields = strcmp(value, "true") == 0;
	}
}

void btd_config_populate(struct btd_config *config, int argc, char **argv)
{
	FILE *fp;
	char *key, *line = NULL;
	size_t len, sep;

	config->configpath = NULL;

	config->socket = NULL;

	config->filefmt = safe_strdup(".pdf");
	config->check_fields = safe_strdup("false");
	config->pidfile = safe_strdup("");

	argp_parse(&argp, argc, argv, 0, 0, config);
	btd_log(2, "Arguments parsed. Loglevel set to %d\n", btd_log_level);

	if(config->configpath == NULL){
		config->configpath = get_config_path();
	}
	config->datadir = get_data_path();
	char *l[2] = {config->datadir, "/btd.socket"};
	key = safe_strcat(l, 2);
	create_unixsocket(config, key);
	free(key);
	config->pidfile = resolve_tilde(config->pidfile);

	btd_log(2, "Opening config at '%s'\n", config->configpath);
	fp = safe_fopen(config->configpath, "r");

	while (getline(&line, &len, fp) != -1) {
		sep = strcspn(line, "=");
		key = strndup(line, sep);
		if (key == NULL){
			die("strndup() failed\n");
		}
		update_config(config, key, line+sep+1);
		free(key);
		free(line);
		line = NULL;
	}
	btd_log(2, "Done parsing\n");
	l[1] = "/db.sqlite";
	config->db = safe_strcat(l, 2);

	safe_fclose(fp);
}

void btd_config_print(struct btd_config *config, FILE *fp){
	fprintf(fp, 
		"BTD Config digest\n"
		"-----------------\n"
		"configpath: '%s'\n"
		"\n"
		"datadir: '%s'\n"
		"database:'%s'\n"
		"filefmt: '%s'\n"
		"pidfile: '%s'\n"
		"check_fields: '%s'\n",
			config->configpath,
			config->datadir,
			config->db,
			config->filefmt,
			config->pidfile,
			config->check_fields ? "true": "false"
			);
	fputs("sockets:\n", fp);
	char *s;
	for(struct addrinfo *r = config->socket; r != NULL; r=r->ai_next){
		s = pprint_address(r);
		fprintf(fp, "%s\n", s);
		free(s);
	}
}
