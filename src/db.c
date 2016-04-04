#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h>

#include "log.h"
#include "misc.h"

#define VERSION "0.1"

char *sqlite_create_cfg_table =
	"CREATE TABLE IF NOT EXISTS config"
	"(version REAL, datecreated TEXT);"
	"INSERT OR IGNORE INTO config (rowid, version, datecreated)" 
	"VALUES(1," VERSION ", date('now'));";

sqlite3 *db;
char *sqlite_currerr;

static int db_get_version_cb(void *nu, int argc, char **argv, char **cname)
{
	char **data = (char **)nu;
	data[0] = safe_strdup(argv[0]);
	data[1] = safe_strdup(argv[1]);
	return 0;
	(void)argc;
	(void)cname;
}

char **db_get_version()
{
	char **data = malloc((sizeof (char*))*2);
	int rc = sqlite3_exec(db, "SELECT version, datecreated FROM config",
		db_get_version_cb, data, &sqlite_currerr);
	if(rc != SQLITE_OK ){
		die("SQLite error: %s\n", sqlite_currerr);
	}
	return data;
}

void db_init(char *dbpath, char *fspath)
{
	char **datever;

	int rc = sqlite3_open(dbpath, &db);
	btd_log(2, "Opening db at: '%s'\n", dbpath);
	if(rc != SQLITE_OK){
		die("SQLite error: %s\n", sqlite3_errmsg(db));
	}

	btd_log(2, "Grabbing and/or creating version table\n");
	rc = sqlite3_exec(db, sqlite_create_cfg_table, NULL, 0, &sqlite_currerr);
	if(rc != SQLITE_OK ){
		die("SQLite error: %s\n", sqlite_currerr);
	}

	datever = db_get_version();
	btd_log(1, "Opened db v%s created on %s\n", datever[0], datever[1]);

	free(datever[0]);
	free(datever[1]);
	free(datever);

	if(!path_exists(fspath)){
		btd_log(2, "Creating filesystem at: '%s'\n", fspath);
		rc = mkdir(fspath, 0777);
		if(rc == -1){
			perror("mkdir");
			die("If the directory didn't exist you can create it by running:\n"
				"$ mkdir -p '%s'\n", fspath);
		}
	}
	btd_log(2, "Filesystem initialized\n");
}

void db_close()
{
	sqlite3_close(db);
}
