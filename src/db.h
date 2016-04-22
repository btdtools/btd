#ifndef DB_H
#define DB_H

#include "bibtex.h"

/* Initialize database and filesystem */
void db_init();

/* Add a bibtex entry */
void db_add_bibtex(struct bibtex_object *obj, char *filename);

/* Close and commit database and filesystem */
void db_close();

#endif
