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
#include "libbtd.h"

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
		config->configpath = safe_strdup(arg);
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

static void free_config_socket(struct btd_config *config)
{
	if (config->socket->ai_family == AF_UNIX)
		free(config->socket->ai_addr);
	freeaddrinfo(config->socket);
}

bool parse_boolean(char *value, char *name)
{
	if(strcmp(value, "true") != 0 && strcmp(value, "false") != 0)
		die("%s can either be 'true' or 'false'\n", name);
	return strcmp(value, "true") == 0;
}

void update_config(struct btd_config *config, char *key, char *value){
	key = rtrim(ltrim(key));

	/* If the key is empty or the line starts with a comment we skip */
	if (key[0] == '#' || strlen(key) == 0)
		return;

	/* If the key contains a comment we skip */
	if (strchr("key", (char)'#') != NULL)
		return;
	value[strcspn(value, "#")] = '\0';
	value = rtrim(ltrim(value));

	/* If the value stripped of comments is empty we skip */
	if (strlen(value) == 0)
		return;

	/* Configuration options */
	if (strcmp(key, "socket") == 0){
		free_config_socket(config);
		config->socket = btd_get_addrinfo(value);
	} else if (strcmp(key, "datadir") == 0){
		free(config->datadir);
		config->datadir = resolve_tilde(value);
	} else if (strcmp(key, "filefmt") == 0){
		free(config->filefmt);
		config->filefmt = resolve_tilde(value);
	} else if (strcmp(key, "pidfile") == 0){
		free(config->pidfile);
		config->pidfile = safe_strdup(value);
	} else if (strcmp(key, "check_fields") == 0){
		config->check_fields = parse_boolean(value, "check_fields");
	} else if (strcmp(key, "multithread") == 0){
		config->multithread = parse_boolean(value, "multithread");
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
	config->check_fields = true;
	config->multithread = true;
	config->pidfile = safe_strdup("");

	argp_parse(&argp, argc, argv, 0, 0, config);
	btd_log(2, "Arguments parsed. Loglevel set to %d\n",
		get_btd_log_level());

	if (config->configpath == NULL)
		config->configpath = get_config_path();

	config->datadir = get_data_path();
	key = safe_strcat(2, config->datadir, "/btd.socket");
	config->socket = btd_get_addrinfo(key);
	free(key);

	btd_log(2, "Opening config at '%s'\n", config->configpath);
	fp = safe_fopen(config->configpath, "r");

	while (getline(&line, &len, fp) != -1) {
		sep = strcspn(line, "=");
		key = strndup(line, sep);
		if (key == NULL){
			die("strndup() failed\n");
		}
		update_config(config, key, line+sep+1);
		free(line);
		free(key);
		line = NULL;
	}
	free(line);
	btd_log(2, "Done parsing\n");
	config->db = safe_strcat(2, config->datadir, "/db.sqlite");
	config->filesdir = safe_strcat(2, config->datadir, "/files/");

	safe_fclose(fp);
}

void btd_config_print(struct btd_config *config, FILE *fp)
{
	safe_fprintf(fp,
		"BTD Config digest\n"
		"-----------------\n"
		"configpath: '%s'\n"
		"\n"
		"datadir: '%s'\n"
		"database:'%s'\n"
		"filefmt: '%s'\n"
		"pidfile: '%s'\n"
		"check_fields: '%s'\n"
		"multithread: '%s'\n",
			config->configpath,
			config->datadir,
			config->db,
			config->filefmt,
			config->pidfile,
			config->check_fields ? "true": "false",
			config->multithread ? "true": "false"
			);
	safe_fputs(fp, "sockets:\n");
	char *s;
	for (struct addrinfo *r = config->socket; r != NULL; r=r->ai_next){
		s = pprint_address(r);
		safe_fprintf(fp, "%s\n", s);
		free(s);
	}
}

void btd_config_free(struct btd_config *config)
{
	free_config_socket(config);
	free(config->configpath);
	free(config->datadir);
	free(config->db);
	free(config->filesdir);
	free(config->filefmt);
	free(config->pidfile);
	free(config);
}
