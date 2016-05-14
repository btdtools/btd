#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "log.h"
#include "misc.h"
#include "bibtex.h"

#define VERSION "0.1"

#define SQLITE_Q(q) SQLITE_E(q, SQLITE_OK)
#define SQLITE_E(q, c)\
	rc = q;\
	if(rc != c){\
		printf("SQLite error: %s\n", sqlite3_errmsg(db)); \
		die("SQLite error: %d-%s\n", rc, sqlite_currerr);\
	}\

char *sqlite_create_cfg_table =
	"CREATE TABLE IF NOT EXISTS config"
	"(version REAL, datecreated TEXT);"
	"INSERT OR IGNORE INTO config (rowid, version, datecreated)" 
	"VALUES(1," VERSION ", date('now'));";
char *sqlite_create_data_table =
	"CREATE TABLE IF NOT EXISTS data"
	"(name TEXT, author TEXT, path TEXT, datecreated TEXT, bibtex TEXT);";
char *sqlite_add_datarow =
	"INSERT INTO data "
	"(name, author, path, datecreated, bibtex)"
	"VALUES (?, ?, ?, date('now'), ?);";

struct btd_config *config;
sqlite3 *db;
sqlite3_stmt *stmt;
char *sqlite_currerr;
int rc;

static int db_get_version_cb(void *nu, int argc, char **argv, char **cname)
{
	char **data = (char **)nu;
	data[0] = safe_strdup(argv[0]);
	data[1] = safe_strdup(argv[1]);
	return 0;
	(void)argc;
	(void)cname;
}

static char **db_get_version()
{
	char **data = malloc((sizeof (char*))*2);
	SQLITE_Q(sqlite3_exec(db, "SELECT version, datecreated FROM config",
			db_get_version_cb, data, &sqlite_currerr));
	return data;
}

void db_convert(char *version)
{
	btd_log(2, "Db version: %s, current version: " VERSION "\n", version);
	if(strcmp(version, VERSION) == 0){
		btd_log(2, "Db up to date\n");
	} else {
		die("Database version: %s\n"
			"Program version: " VERSION "\n"
			"There is no upgrade possible\n", version);
	}
}

int db_add_bibtex(struct bibtex_object *obj, char *path)
{
	sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
	char *print = bibtex_print(obj);

	btd_log(2, "Adding bibtex entry: '%s'\n", print);
	SQLITE_Q(sqlite3_prepare_v2(db, sqlite_add_datarow, -1,
			&stmt, 0));

	btd_log(2, "Binding name\n");
	SQLITE_Q(sqlite3_bind_text(stmt, 1, obj->identifier,
			strlen(obj->identifier), SQLITE_STATIC));
	btd_log(2, "Binding author\n");
	char *author = bibtex_get_author(obj);
	SQLITE_Q(sqlite3_bind_text(stmt, 2, author,
			strlen(author), SQLITE_STATIC));
	btd_log(2, "Binding path\n");
	SQLITE_Q(sqlite3_bind_text(stmt, 3, path,
			strlen(path), SQLITE_STATIC))
	btd_log(2, "Binding bibtex\n");
	SQLITE_Q(sqlite3_bind_text(stmt, 4, print,
			strlen(print), SQLITE_STATIC));

	btd_log(2, "Step\n");
	SQLITE_E(sqlite3_step(stmt), SQLITE_DONE);

	btd_log(2, "Finalize\n");
	SQLITE_Q(sqlite3_finalize(stmt));

	free(print);
	sqlite3_int64 rowid2 = sqlite3_last_insert_rowid(db);
	printf("Old rowid: %llu\n", rowid);;
	printf("New rowid: %llu\n", rowid2);;
	if(rowid2 != 0 && rowid != rowid2){
		return rowid2;
	} else {
		return -1;
	}
}

void db_init(struct btd_config *cfg)
{
	char **datever;

	config = cfg;
	fprintf(stdout, 
		"BTD Config digest\n"
		"-----------------\n"
		"configpath: '%s'\n"
		"\n"
		"socket: '%s'\n"
		"db: '%s'\n"
		"files: '%s'\n"
		"filefmt: '%s'\n",
			config->configpath,
			config->socket,
			config->db,
			config->files,
			config->filefmt
			);

	btd_log(2, "Opening db at: '%s'\n", config->db);
	SQLITE_Q(sqlite3_open(config->db, &db));

	btd_log(2, "Grabbing and/or creating version table\n");
	SQLITE_Q(sqlite3_exec(db, sqlite_create_cfg_table, NULL, 0, &sqlite_currerr));

	btd_log(2, "Creating or verifying data table\n");
	SQLITE_Q(sqlite3_exec(db, sqlite_create_data_table, NULL, 0, &sqlite_currerr));

	datever = db_get_version();
	btd_log(1, "Opened db v%s created on %s\n", datever[0], datever[1]);
	db_convert(datever[0]);

	free(datever[0]);
	free(datever[1]);
	free(datever);

	if(!path_exists(config->files)){
		btd_log(2, "Creating filesystem at: '%s'\n", config->files);
		if(mkdir(config->files, 0777) == -1){
			perror("mkdir");
			die("If the directory didn't exist you can create it by running:\n"
				"$ mkdir -p '%s'\n", config->files);
		}
	}
	btd_log(2, "Filesystem initialized\n");
}

static int db_num_cb(void *nu, int argc, char **argv, char **cname)
{
	int *a = (int *)nu;
	*a = atoi(argv[0]);
	return 0;
	(void)argc;
	(void)cname;
}

int db_num()
{
	int num;
	SQLITE_Q(sqlite3_exec(db, "SELECT Count(*) FROM data",
			db_num_cb, &num, &sqlite_currerr));
	return num;
}

void db_close()
{
	sqlite3_close(db);
}
