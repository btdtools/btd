#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <netdb.h>

struct btd_config
{
	/* Deamon options */
	char *configpath;
	/* Configuration options */
	struct addrinfo *socket;
	char *db;
	char *files;
	char *filefmt;
	char *pathsep;
	char *pidfile;
	bool check_fields;
};

/* Populate a structure, read from args and config file */
void btd_config_populate(struct btd_config *config, int argc, char **argv);

/* Print a configuration */
void btd_config_print(struct btd_config *config, FILE *fp);

#endif
