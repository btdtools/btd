#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

struct btd_config
{
	/* Deamon options */
	char *configpath;
	/* Configuration options */
	char *socket;
	char *db;
	char *files;
	char *filefmt;
	bool check_fields;
};

/* Populate a structure, read from args and config file */
void btd_config_populate(struct btd_config *config, int argc, char **argv);

/* Print a configuration */
void btd_config_print(struct btd_config *config, FILE *fp);

#endif
