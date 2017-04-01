#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <uuid/uuid.h>

#include "config.h"
#include "libbtd.h"
#include "bibtex.h"

#define VERSION "0.1"

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
	"(path TEXT, datecreated TEXT, uuid TEXT);";
char *sqlite_add_datarow =
	"INSERT INTO data "
	"(name, author, path, datecreated, bibtex)"
	"VALUES (?, ?, ?, date('now'), ?);";
char *sqlite_add_filerow =
	"INSERT INTO files "
	"(path, uuid, datecreated)"
	"VALUES (?, ?, date('now'));";

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

static char **db_get_version()
{
	char **version = malloc(2*sizeof(char *));
	sqlite_query(sqlite3_prepare_v2(db,
		"SELECT version, datecreated FROM config", -1, &stmt, 0));

	sqlite_errcheck(sqlite3_step(stmt), SQLITE_ROW);
	version[0] = safe_strdup((char *)sqlite3_column_text(stmt, 0));
	version[1] = safe_strdup((char *)sqlite3_column_text(stmt, 1));
	sqlite3_finalize(stmt);
	return version;
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
	sqlite_query(sqlite3_open_v2(config->db, &db, SQLITE_OPEN_READWRITE |
		SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX , NULL));

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

int db_num()
{
	int num;
	sqlite_query(sqlite3_prepare_v2(
		db, "SELECT Count(*) FROM data", -1, &stmt, 0));

	sqlite_errcheck(sqlite3_step(stmt), SQLITE_ROW);
	num = sqlite3_column_int(stmt, 0);
	sqlite_query(sqlite3_finalize(stmt));
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

char *dot_shorten(const unsigned char *c, size_t len)
{
	char *r = (char *)c;
	if(strlen(r) > len)
		strcpy(r+len-4, "...");
	return r;
}

void db_list(FILE *fd)
{
	int rc;
	safe_fputs(fd, "id\tname\tauthor\tpath\tdate\tbibtex\n");
	sqlite_query(sqlite3_prepare_v2(db,
		"SELECT rowid, name, author, path, date"
		"created, bibtex FROM data", -1, &stmt, 0));
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
		safe_fprintf(fd, "%lld\t%s\t%s\t%s\t%s\t%s\n",
			sqlite3_column_int64(stmt, 0),
			dot_shorten(sqlite3_column_text(stmt, 1), 10),
			dot_shorten(sqlite3_column_text(stmt, 2), 10),
			dot_shorten(sqlite3_column_text(stmt, 3), 10),
			dot_shorten(sqlite3_column_text(stmt, 4), 10),
			dot_shorten(sqlite3_column_text(stmt, 5), 20));
	}
	sqlite_errcheck(rc, SQLITE_DONE);

	btd_log(2, "Finalize\n");
	sqlite_query(sqlite3_finalize(stmt));
}

void db_file_list(FILE *fd)
{
	struct stat b;
	char *bt;
	safe_fputs(fd, "id\tpath\tdatecreated\tsize\n");
	btd_log(2, "Listing files\n");
	sqlite_query(sqlite3_prepare_v2(db, 
		"SELECT rowid, path, datecreated, uuid FROM files",
		-1, &stmt, 0));

	btd_log(2, "Step\n");
	while (sqlite3_step(stmt) == SQLITE_ROW){
		bt = safe_strcat(2,
			config->filesdir, sqlite3_column_text(stmt, 3));
		stat(bt, &b);
		btd_log(2, "Row retrieved\n");
		fprintf(fd, "%lld\t%s\t%s\t%ld\n",
			sqlite3_column_int64(stmt, 0),
			sqlite3_column_text(stmt, 1),
			sqlite3_column_text(stmt, 2),
			b.st_size);
		free(bt);
	}

	btd_log(2, "Finalize\n");
	sqlite_query(sqlite3_finalize(stmt));
}

void db_file_remove(long int id, FILE *fd)
{
	btd_log(2, "Trying to find file with %ld\n", id);
	sqlite_query(sqlite3_prepare_v2(db,
		"SELECT uuid FROM files WHERE rowid=?", -1, &stmt, 0));

	btd_log(2, "Binding dataid\n");
	sqlite_query(sqlite3_bind_int64(stmt, 1, id));

	btd_log(2, "Step\n");
	int rc = sqlite3_step(stmt);
	char uuid[37];
	if (rc == SQLITE_ROW) {
		memcpy(uuid, sqlite3_column_text(stmt, 0), 37);
	} else if (rc == SQLITE_DONE) {
		safe_fprintf(fd, "1\nNo file with id %d\n", id);
		return;
	} else {
		safe_fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
		die("SQLite error: %d-%s\n", rc, sqlite_currerr);
	}

	btd_log(2, "Finalize\n");
	sqlite_query(sqlite3_finalize(stmt));

	btd_log(2, "Removing file from db\n");
	sqlite_query(sqlite3_prepare_v2(db,
		"DELETE FROM files WHERE rowid=?", -1, &stmt, 0));

	btd_log(2, "Binding dataid\n");
	sqlite_query(sqlite3_bind_int64(stmt, 1, id));

	btd_log(2, "Step\n");
	sqlite_errcheck(sqlite3_step(stmt), SQLITE_DONE);

	btd_log(2, "Finalize\n");
	sqlite_query(sqlite3_finalize(stmt));


	char *bt = safe_strcat(2, config->filesdir, uuid);
	if (unlink(bt) == -1) {
		safe_fputs(fd,
			"1\nSomething went wrong removing the actual file\n");
		btd_log(2, "Something went wrong removing the file: %s\n",
			strerror(errno));
	} else {
		safe_fputs(fd, "0\n");
	}
	free(bt);
}

void db_file_upload(char *fn, long int length, FILE *fd)
{
	char c, ustr[37];
	uuid_t uuid;

	uuid_generate_random(uuid);
	uuid_unparse_lower(uuid, ustr);

	btd_log(2, "Adding file entry with uuid: '%s'\n", ustr);
	sqlite_query(sqlite3_prepare_v2(
		db, sqlite_add_filerow, -1, &stmt, 0));

	btd_log(2, "Binding path\n");
	sqlite_query(sqlite3_bind_text(
		stmt, 2, fn, strlen(fn), SQLITE_STATIC));

	btd_log(2, "Binding uuid\n");
	sqlite_query(sqlite3_bind_text(
		stmt, 3, ustr, strlen(ustr), SQLITE_STATIC));

	btd_log(2, "Step\n");
	sqlite_errcheck(sqlite3_step(stmt), SQLITE_DONE);

	btd_log(2, "Finalize\n");
	sqlite_query(sqlite3_finalize(stmt));

	char *bt = safe_strcat(2, config->filesdir, ustr);
	FILE *f = safe_fopen(bt, "w");
	while((c = fgetc(fd)) != EOF && length-- > 0)
		fputc(c, f);
	safe_fclose(f);
	if(length > 0){
		btd_log(1, "Early EOF in file data?\n");
		db_file_remove(sqlite3_last_insert_rowid(db), fd);
	}
	free(bt);
	
}

void db_close()
{
	sqlite3_close(db);
}
