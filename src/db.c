#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <uuid/uuid.h>

#include "config.h"
#include "log.h"
#include "misc.h"
#include "bibtex.h"

char *sqlite_create_cfg_table =
	"CREATE TABLE IF NOT EXISTS config"
	"(version REAL, datecreated TEXT);"
	"INSERT OR IGNORE INTO config (rowid, version, datecreated)"
	"VALUES(1, '" VERSION "', date('now'));";
char *sqlite_create_data_table =
	"CREATE TABLE IF NOT EXISTS data"
	"(name TEXT, author TEXT, path TEXT, datecreated TEXT, bibtex TEXT);";
char *sqlite_create_file_table =
	"CREATE TABLE IF NOT EXISTS files"
	"(data INTEGER, path TEXT, uuid TEXT);";
char *sqlite_add_datarow =
	"INSERT INTO data "
	"(name, author, path, datecreated, bibtex)"
	"VALUES (?, ?, ?, date('now'), ?);";
char *sqlite_add_filerow =
	"INSERT INTO files "
	"(data, path, uuid)"
	"VALUES (?, ?, ?);";

struct btd_config *config;
sqlite3 *db;
sqlite3_stmt *stmt;
char *sqlite_currerr;

static void sqlite_errcheck(int got, int want)
{
	if(got != want){
		safe_fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
		die("SQLite error: %d-%s\n", got, sqlite_currerr);\
	}
}

static void sqlite_query(int got)
{
	sqlite_errcheck(got, SQLITE_OK);
}

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
	char **data = safe_malloc((sizeof(char*))*2);
	sqlite_query(sqlite3_exec(db, "SELECT version, datecreated FROM config",
		db_get_version_cb, data, &sqlite_currerr));
	return data;
}

void db_convert(char *version)
{
	btd_log(2, "Db version: %s, current version: %s\n", version, VERSION);
	if (strcmp(version, VERSION) == 0){
		btd_log(2, "Db up to date\n");
	} else {
		die("Database version: %s\n"
			"Program version: %s\n"
			"There is no upgrade possible\n", version, VERSION);
	}
}

int db_add_bibtex(struct bibtex_object *obj, char *path)
{
	sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
	char *print = bibtex_print(obj);

	btd_log(2, "Adding bibtex entry: '%s'\n", print);
	sqlite_query(sqlite3_prepare_v2(db, sqlite_add_datarow, -1,
			&stmt, 0));

	btd_log(2, "Binding name\n");
	sqlite_query(sqlite3_bind_text(stmt, 1, obj->identifier,
			strlen(obj->identifier), SQLITE_STATIC));

	btd_log(2, "Binding author\n");
	char *author = bibtex_get_author(obj);
	sqlite_query(sqlite3_bind_text(stmt, 2, author,
			strlen(author), SQLITE_STATIC));

	btd_log(2, "Binding path\n");
	sqlite_query(sqlite3_bind_text(stmt, 3, path,
			strlen(path), SQLITE_STATIC));

	btd_log(2, "Binding bibtex\n");
	sqlite_query(sqlite3_bind_text(stmt, 4, print,
			strlen(print), SQLITE_STATIC));

	btd_log(2, "Step\n");
	sqlite_errcheck(sqlite3_step(stmt), SQLITE_DONE);

	btd_log(2, "Finalize\n");
	sqlite_query(sqlite3_finalize(stmt));

	free(print);
	sqlite3_int64 rowid2 = sqlite3_last_insert_rowid(db);
	return rowid2 != 0 && rowid != rowid2 ? rowid2 : -1;
}

static void create_folder(char *p)
{
	if (!path_exists(p)){
		btd_log(1, "%s doesn't exist, creating\n", p);
		if (mkdir(p, 0777) != 0){
			perror("mkdir");
			die("mkdir()\n");
		}
	}
}

static void create_folder_rec(char *p)
{
	char t;
	for (unsigned long int i = 0; i<strlen(p); i++){
		if (p[i] == '/'){
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

	btd_log(2, "Creating filesystem at: '%s'\n", config->datadir);
	create_folder_rec(config->filesdir);

	btd_log(2, "Filesystem initialized\n");

	btd_log(2, "Opening db at: '%s'\n", config->db);
	sqlite_query(sqlite3_open(config->db, &db));

	btd_log(2, "Grabbing and/or creating version table\n");
	sqlite_query(sqlite3_exec(
		db, sqlite_create_cfg_table, NULL, 0, &sqlite_currerr));

	btd_log(2, "Creating or verifying data table\n");
	sqlite_query(sqlite3_exec(
		db, sqlite_create_data_table, NULL, 0, &sqlite_currerr));

	btd_log(2, "Creating or verifying files table\n");
	sqlite_query(sqlite3_exec(
		db, sqlite_create_file_table, NULL, 0, &sqlite_currerr));

	datever = db_get_version();
	btd_log(1, "Opened db v%s created on %s\n", datever[0], datever[1]);
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
	sqlite_query(sqlite3_exec(db, "SELECT Count(*) FROM data",
			db_num_cb, &num, &sqlite_currerr));
	return num;
}

char *db_get(long int id)
{
	char *l = NULL;
	sqlite_query(sqlite3_prepare_v2(
		db, "SELECT bibtex FROM data WHERE rowid=?", -1, &stmt, 0));

	sqlite_query(sqlite3_bind_int(stmt, 1, id));

	if(sqlite3_step(stmt) == SQLITE_ROW)
		l = safe_strdup((char *)sqlite3_column_text(stmt, 0));

	sqlite_query(sqlite3_finalize(stmt));
	return l;
}

static int db_list_cb(void *nu, int argc, char **argv, char **cname)
{
	FILE *fd = (FILE *)nu;
	/* Cut off entries longer than 10 chars */
	for (int i = 0; i<4; i++)
		if (strlen(argv[i]) > 10)
			strcpy(argv[i]+strlen(argv[i])-4, "...");

	/* Cut off entries longer than 20 chars */
	for (int i = 4; i<6; i++)
		if (strlen(argv[i]) > 20)
			strcpy(argv[i]+strlen(argv[i])-4, "...");

	safe_fprintf(fd, "%s\t%s\t%s\t%s\t%s\t%s\n",
		argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
	return 0;
	(void)argc;
	(void)cname;
}

void db_list(FILE *fd)
{
	safe_fputs(fd, "id\tname\tauthor\tpath\tdate\tbibtex\n");
	sqlite_query(sqlite3_exec(db, "SELECT rowid, name, author, path, date"
		"created, bibtex FROM data", db_list_cb, fd, &sqlite_currerr));
}

void db_detach(long int id)
{
	btd_log(2, "Removing file with id: %d\n", id);
}


void db_attach(char *fn, long int id, long int length, FILE *fd)
{
	char c, *bt = db_get(id), ustr[37], ids[40];
	uuid_t uuid;
	if (bt == NULL){
		safe_fprintf(fd, "1\nNot a valid id\n");
	} else {
		free(bt);
		uuid_generate_random(uuid);
		uuid_unparse_lower(uuid, ustr);

		btd_log(2, "Adding file entry with uuid: '%s'\n", ustr);
		sqlite_query(sqlite3_prepare_v2(
			db, sqlite_add_filerow, -1, &stmt, 0));

		sprintf(ids, "%ld", id);
		btd_log(2, "Binding dataid\n");
		sqlite_query(sqlite3_bind_text(
			stmt, 1, ids, strlen(ids), SQLITE_STATIC));

		btd_log(2, "Binding uuid\n");
		sqlite_query(sqlite3_bind_text(
			stmt, 2, ustr, strlen(ustr), SQLITE_STATIC));

		btd_log(2, "Binding path\n");
		sqlite_query(sqlite3_bind_text(
			stmt, 3, fn, strlen(fn), SQLITE_STATIC));

		btd_log(2, "Step\n");
		sqlite_errcheck(sqlite3_step(stmt), SQLITE_DONE);

		btd_log(2, "Finalize\n");
		sqlite_query(sqlite3_finalize(stmt));

		bt = safe_strcat(2, config->filesdir, ustr);
		FILE *f = safe_fopen(bt, "w");
		while((c = fgetc(fd)) != EOF && length-- > 0)
			fputc(c, f);
		safe_fclose(f);
		if(length > 0){
			btd_log(1, "Early EOF in file data?\n");
			db_detach(sqlite3_last_insert_rowid(db));
		}
		free(bt);
	}
}

void db_close()
{
	sqlite3_close(db);
}
