#define _GNU_SOURCE

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "log.h"
#include "misc.h"
#include "bibtex.h"

#define SQLITE_Q(q) SQLITE_E(q, SQLITE_OK)
#define SQLITE_E(q, c)\
	rc = q;\
	if(rc != c){\
		printf("SQLite error: %s\n", sqlite3_errmsg(db)); \
		dief("SQLite error: %d-%s\n", rc, sqlite_currerr);\
	}\

char *sqlite_create_cfg_table =
	"CREATE TABLE IF NOT EXISTS config"
	"(version REAL, datecreated TEXT);"
	"INSERT OR IGNORE INTO config (rowid, version, datecreated)" 
	"VALUES(1, '" VERSION "', date('now'));";
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
	char **data = safe_malloc((sizeof (char*))*2);
	SQLITE_Q(sqlite3_exec(db, "SELECT version, datecreated FROM config",
			db_get_version_cb, data, &sqlite_currerr));
	return data;
}

void db_convert(char *version)
{
	btd_logf(2, "Db version: %s, current version: %s\n", version, VERSION);
	if(strcmp(version, VERSION) == 0){
		btd_log(2, "Db up to date\n");
	} else {
		dief("Database version: %s\n"
			"Program version: %s\n"
			"There is no upgrade possible\n", version, VERSION);
	}
}

int db_add_bibtex(struct bibtex_object *obj, char *path)
{
	sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
	char *print = bibtex_print(obj);

	btd_logf(2, "Adding bibtex entry: '%s'\n", print);
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
	if(rowid2 != 0 && rowid != rowid2){
		return rowid2;
	} else {
		return -1;
	}
}

static void create_folder(char *p)
{
	if(!path_exists(p)){
		btd_logf(1, "%s doesn't exist, creating\n", p);
		if(mkdir(p, 0777) != 0){
			perror("mkdir");
			die("mkdir()\n");
		}
	}
}

static void create_folder_rec(char *p)
{
	char t;
	for(unsigned long int i = 0; i<strlen(p); i++){
		if(p[i] == '/'){
			t = p[i+1];
			p[i+1] = '\0';
			create_folder(p);
			p[i+1] = t;
		}
	}
	create_folder(p);
}

void db_init(struct btd_config *cfg)
{
	char **datever;

	config = cfg;

	btd_logf(2, "Creating filesystem at: '%s'\n", config->datadir);
	create_folder_rec(config->datadir);

	btd_log(2, "Filesystem initialized\n");

	btd_logf(2, "Opening db at: '%s'\n", config->db);
	SQLITE_Q(sqlite3_open(config->db, &db));

	btd_log(2, "Grabbing and/or creating version table\n");
	SQLITE_Q(sqlite3_exec(db, sqlite_create_cfg_table, NULL, 0, &sqlite_currerr));

	btd_log(2, "Creating or verifying data table\n");
	SQLITE_Q(sqlite3_exec(db, sqlite_create_data_table, NULL, 0, &sqlite_currerr));

	datever = db_get_version();
	btd_logf(1, "Opened db v%s created on %s\n", datever[0], datever[1]);
	db_convert(datever[0]);

	free(datever[0]);
	free(datever[1]);
	free(datever);
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

char *db_get(long long int id)
{
	char *l = NULL;
	SQLITE_Q(sqlite3_prepare_v2(db, "SELECT bibtex FROM data WHERE rowid=?",
			-1, &stmt, 0));

	SQLITE_Q(sqlite3_bind_int(stmt, 1, id));

	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW){
		l = safe_strdup((char *)sqlite3_column_text(stmt, 0));
	}

	SQLITE_Q(sqlite3_finalize(stmt));
	return l;
}

static int db_list_cb(void *nu, int argc, char **argv, char **cname)
{
	FILE *fd = (FILE *)nu;
	/* Cut off entries longer than 10 chars */
	for(int i = 0; i<4; i++){
		if(strlen(argv[i]) > 10){
			strcpy(argv[i]+strlen(argv[i])-4, "...");
		}
	}
	/* Cut off entries longer than 20 chars */
	for(int i = 4; i<6; i++){
		if(strlen(argv[i]) > 20){
			strcpy(argv[i]+strlen(argv[i])-4, "...");
		}
	}

	fprintf(fd, "%s\t%s\t%s\t%s\t%s\t%s\n",
		argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
	return 0;
	(void)argc;
	(void)cname;
}

void db_list(FILE *fd)
{
	fputs("id\tname\tauthor\tpath\tdate\tbibtex\n", fd);
	SQLITE_Q(sqlite3_exec(db, 
		"SELECT rowid, name, author, path, datecreated, bibtex FROM data",
			db_list_cb, &fd, &sqlite_currerr));
}

void db_close()
{
	sqlite3_close(db);
}
