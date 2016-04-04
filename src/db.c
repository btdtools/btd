#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

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

void db_init(char *path)
{
	char **dateversion;

	int rc = sqlite3_open(path, &db);
	btd_log(2, "Opening db at: '%s'\n", path);
	if(rc != SQLITE_OK){
		die("SQLite error: %s\n", sqlite3_errmsg(db));
	}

	btd_log(2, "Checking for version table\n");
	rc = sqlite3_exec(db, sqlite_create_cfg_table, NULL, 0, &sqlite_currerr);
	if(rc != SQLITE_OK ){
		die("SQLite error: %s\n", sqlite_currerr);
	}


	btd_log(2, "Grabbing version\n");
	dateversion = db_get_version();
	btd_log(1, "Opened db v%s created on %s\n", dateversion[0], dateversion[1]);

	free(dateversion[0]);
	free(dateversion[1]);
	free(dateversion);
}

void db_close()
{
	sqlite3_close(db);
}
