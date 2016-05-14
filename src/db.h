#ifndef DB_H
#define DB_H

#include "bibtex.h"

/* Initialize database and filesystem */
void db_init();

/* Add a bibtex entry, returns id or -1 if failed*/
int db_add_bibtex(struct bibtex_object *obj, char *path);

/* Ask how many datapoints there are */
int db_num();

/* Close and commit database and filesystem */
void db_close();

#endif
