#ifndef DB_H
#define DB_H

#include "bibtex.h"

/* Initialize database and filesystem */
void db_init();

/* Add a bibtex entry, returns id or -1 if failed*/
int db_add_bibtex(struct bibtex_object *obj, char *path);

/* Ask how many datapoints there are */
int db_num();

/* Get a bibtex snippet */
char *db_get(long int id);

/* Print a list of bibtex snippets */
void db_list(FILE *fd);

/* File operations */
void db_file_list(FILE *fd);
void db_file_upload(char *fn, size_t length, FILE *fd);
void db_file_remove(long int id, FILE *fd);

/* Close and commit database and filesystem */
void db_close();

#endif
