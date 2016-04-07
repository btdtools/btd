#ifndef BIBTEX_H
#define BIBTEX_H

#include <stdio.h>

typedef enum { 
	BIBTEX_ENTRY_ARTICLE,
	BIBTEX_ENTRY_BOOK,
	BIBTEX_ENTRY_BOOKLET,
	BIBTEX_ENTRY_CONFERENCE,
	BIBTEX_ENTRY_INBOOK,
	BIBTEX_ENTRY_INCOLLECTION,
	BIBTEX_ENTRY_INPROCEEDINGS,
	BIBTEX_ENTRY_MANUAL,
	BIBTEX_ENTRY_MASTERSTHESIS,
	BIBTEX_ENTRY_MISC,
	BIBTEX_ENTRY_PHDTHESIS,
	BIBTEX_ENTRY_PROCEEDINGS,
	BIBTEX_ENTRY_TECHREPORT,
	BIBTEX_ENTRY_UNPUBLISHED
} bibtex_entrytype;

typedef enum {
	BIBTEX_FIELD_ADDRESS,
	BIBTEX_FIELD_ANNOTE,
	BIBTEX_FIELD_AUTHOR,
	BIBTEX_FIELD_BOOKTITLE,
	BIBTEX_FIELD_CHAPTER,
	BIBTEX_FIELD_CROSSREF,
	BIBTEX_FIELD_EDITION,
	BIBTEX_FIELD_EDITOR,
	BIBTEX_FIELD_HOWPUBLISHED,
	BIBTEX_FIELD_INSTITUTION,
	BIBTEX_FIELD_JOURNAL,
	BIBTEX_FIELD_KEY,
	BIBTEX_FIELD_MONTH,
	BIBTEX_FIELD_NOTE,
	BIBTEX_FIELD_NUMBER,
	BIBTEX_FIELD_ORGANIZATION,
	BIBTEX_FIELD_PAGES,
	BIBTEX_FIELD_PUBLISHER,
	BIBTEX_FIELD_SCHOOL,
	BIBTEX_FIELD_SERIES,
	BIBTEX_FIELD_TITLE,
	BIBTEX_FIELD_TYPE,
	BIBTEX_FIELD_VOLUME,
	BIBTEX_FIELD_YEAR,
	BIBTEX_FIELD_OTHER
} bibtex_field;

struct bibtex_entry {
	bibtex_field field;
	char *key;
	char *value;
	struct bibtex_entry *next;
};

struct bibtex_object {
	bibtex_entrytype type;
	char *identifier;
	struct bibtex_entry *head;
};

/* Parse exactly one bibtex object, returns 0 when success */
struct bibtex_object *bibtex_parse(FILE *istream, char *errmsg);

/* Print a bibtex object */
void bibtex_print(FILE *ostream, struct bibtex_object *obj);

/* Free a bibtex object */
void bibtex_free(struct bibtex_object *obj);

#endif
