#include <stdio.h>
#include <sqlite3.h>

#include "log.h"

sqlite3 *db;

int db_init(char *path){
	btd_log(2, "Opening db at: '%s'\n", path);
	return sqlite3_open(path, &db);
}

void db_close(){
	sqlite3_close(db);
}
