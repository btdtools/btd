#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bibtex.h"
#include "misc.h"

#define BUFSIZE 4048

//typedef enum {
//	BIBTEX_FIELD_ADDRESS,
//	BIBTEX_FIELD_ANNOTE,
//	BIBTEX_FIELD_AUTHOR,
//	BIBTEX_FIELD_BOOKTITLE,
//	BIBTEX_FIELD_CHAPTER,
//	BIBTEX_FIELD_CROSSREF,
//	BIBTEX_FIELD_EDITION,
//	BIBTEX_FIELD_EDITOR,
//	BIBTEX_FIELD_HOWPUBLISHED,
//	BIBTEX_FIELD_INSTITUTION,
//	BIBTEX_FIELD_JOURNAL,
//	BIBTEX_FIELD_KEY,
//	BIBTEX_FIELD_MONTH,
//	BIBTEX_FIELD_NOTE,
//	BIBTEX_FIELD_NUMBER,
//	BIBTEX_FIELD_ORGANIZATION,
//	BIBTEX_FIELD_PAGES,
//	BIBTEX_FIELD_PUBLISHER,
//	BIBTEX_FIELD_SCHOOL,
//	BIBTEX_FIELD_SERIES,
//	BIBTEX_FIELD_TITLE,
//	BIBTEX_FIELD_TYPE,
//	BIBTEX_FIELD_VOLUME,
//	BIBTEX_FIELD_YEAR,
//	BIBTEX_FIELD_OTHER
//} bibtex_field;
//
//struct bibtex_entry {
//	bibtex_field field;
//	char *key;
//	char *value;
//	struct bibtex_entry *next;
//};
//
//struct bibtex_object {
//	bibtex_entrytype type;
//	char *identifier;
//	struct bibtex_entry *head;
//};

void skip_whitespace(FILE *istream)
{
	char c;
	while(isspace((c = fgetc(istream))));
	ungetc(c, istream);
}

struct bibtex_object *bibtex_parse(FILE *istream, char *errmsg)
{
	struct bibtex_object *obj = malloc(sizeof (struct bibtex_object));
	char buf[BUFSIZE+1];
	char c;
	int bufloc = 0;

	//Skip whitespace
	while(isspace((c = fgetc(istream))));

	//Read @
	if(c != '@'){
		asprintf(&errmsg, 
			"Bibtex entry should start with @ but starts with '%c'\n", c);
		free(obj);
		return NULL;
	}

	//Read entrytype
	while(isalpha(c = fgetc(istream)) && bufloc < BUFSIZE) {
		buf[bufloc++] = c;
	}
	buf[bufloc] = '\0';
	if(strcmp(buf, "article") == 0)
		obj->type = BIBTEX_ENTRY_ARTICLE;
	else if(strcmp(buf, "book") == 0)
		obj->type = BIBTEX_ENTRY_BOOK;
	else if(strcmp(buf, "booklet") == 0)
		obj->type = BIBTEX_ENTRY_BOOKLET;
	else if(strcmp(buf, "conference") == 0)
		obj->type = BIBTEX_ENTRY_CONFERENCE;
	else if(strcmp(buf, "inbook") == 0)
		obj->type = BIBTEX_ENTRY_INBOOK;
	else if(strcmp(buf, "incollection") == 0)
		obj->type = BIBTEX_ENTRY_INCOLLECTION;
	else if(strcmp(buf, "inproceedings") == 0)
		obj->type = BIBTEX_ENTRY_INPROCEEDINGS;
	else if(strcmp(buf, "manual") == 0)
		obj->type = BIBTEX_ENTRY_MANUAL;
	else if(strcmp(buf, "mastersthesis") == 0)
		obj->type = BIBTEX_ENTRY_MASTERSTHESIS;
	else if(strcmp(buf, "misc") == 0)
		obj->type = BIBTEX_ENTRY_MISC;
	else if(strcmp(buf, "phdthesis") == 0)
		obj->type = BIBTEX_ENTRY_PHDTHESIS;
	else if(strcmp(buf, "proceedings") == 0)
		obj->type = BIBTEX_ENTRY_PROCEEDINGS;
	else if(strcmp(buf, "techreport") == 0)
		obj->type = BIBTEX_ENTRY_TECHREPORT;
	else if(strcmp(buf, "unpublished") == 0)
		obj->type = BIBTEX_ENTRY_UNPUBLISHED;
	else {
		asprintf(&errmsg, "Invalid bibtex entry type: '%s'\n", buf);
		free(obj);
		return NULL;
	}

	//Read openbrace and identifier
	if(c != '{'){
		asprintf(&errmsg, "After a entrytype I except '{' but got '%c'\n", c);
		free(obj);
		return NULL;
	}
	while((isalnum(c = fgetc(istream)) || 
			c == '|' || c == '+' || c == '_' || c == '-') 
			&& bufloc < BUFSIZE) {
		buf[bufloc++] = c;
	}
	buf[bufloc] = '\0';
	obj->identifier = safe_strdup(buf);

	//Read key-valuepairs
	obj->next = NULL;
	bibtex_read_keyvals(istream, obj, obj->next);
//	while(isspace((c = fgetc(istream))));
//	if(c != ','){
//		asprintf(&errmsg, "After a entrytype I except '{' but got '%c'\n", c);
//		free(obj->identifier);
//		free(obj);
//		return NULL;
//	}
	//struct bibtex_entry {
	//	bibtex_field field;
	//	char *key;
	//	char *value;
	//	struct bibtex_entry *next;
	//};


	
	return obj;
}

void bibtex_print(FILE *ostream, struct bibtex_object *obj)
{
	fprintf(ostream, "@%s{%s,\n", 
		obj->type == BIBTEX_ENTRY_ARTICLE ? "article" :
		obj->type == BIBTEX_ENTRY_BOOK ? "book" :
		obj->type == BIBTEX_ENTRY_BOOKLET ? "booklet" :
		obj->type == BIBTEX_ENTRY_CONFERENCE ? "conference" :
		obj->type == BIBTEX_ENTRY_INBOOK ? "inbook" :
		obj->type == BIBTEX_ENTRY_INCOLLECTION ? "incollection" :
		obj->type == BIBTEX_ENTRY_INPROCEEDINGS ? "inproceedings" :
		obj->type == BIBTEX_ENTRY_MANUAL ? "manual" :
		obj->type == BIBTEX_ENTRY_MASTERSTHESIS ? "mastersthesis" :
		obj->type == BIBTEX_ENTRY_MISC ? "misc" :
		obj->type == BIBTEX_ENTRY_PHDTHESIS ? "phdthesis" :
		obj->type == BIBTEX_ENTRY_PROCEEDINGS ? "proceedings" :
		obj->type == BIBTEX_ENTRY_TECHREPORT ? "techreport" :
		obj->type == BIBTEX_ENTRY_UNPUBLISHED ? "unpublished" : "HUH!",
		obj->identifier);
	struct bibtex_entry *head = obj->head;
	while(head != NULL) {
		if(head->field == BIBTEX_FIELD_OTHER){
			fprintf(ostream, "\t%s = %s\n",
				head->key,
				head->value);
		} else {
			fprintf(ostream, "\t%s = %s\n",
				head->field == BIBTEX_FIELD_ADDRESS ? "address" :
				head->field == BIBTEX_FIELD_ANNOTE ? "annote" :
				head->field == BIBTEX_FIELD_AUTHOR ? "author" :
				head->field == BIBTEX_FIELD_BOOKTITLE ? "booktitle" :
				head->field == BIBTEX_FIELD_CHAPTER ? "chapter" :
				head->field == BIBTEX_FIELD_CROSSREF ? "crossref" :
				head->field == BIBTEX_FIELD_EDITION ? "edition" :
				head->field == BIBTEX_FIELD_EDITOR ? "editor" :
				head->field == BIBTEX_FIELD_HOWPUBLISHED ? "howpublished" :
				head->field == BIBTEX_FIELD_INSTITUTION ? "institution" :
				head->field == BIBTEX_FIELD_JOURNAL ? "journal" :
				head->field == BIBTEX_FIELD_KEY ? "key" :
				head->field == BIBTEX_FIELD_MONTH ? "month" :
				head->field == BIBTEX_FIELD_NOTE ? "note" :
				head->field == BIBTEX_FIELD_NUMBER ? "number" :
				head->field == BIBTEX_FIELD_ORGANIZATION ? "organization" :
				head->field == BIBTEX_FIELD_PAGES ? "pages" :
				head->field == BIBTEX_FIELD_PUBLISHER ? "publisher" :
				head->field == BIBTEX_FIELD_SCHOOL ? "school" :
				head->field == BIBTEX_FIELD_SERIES ? "series" :
				head->field == BIBTEX_FIELD_TITLE ? "title" :
				head->field == BIBTEX_FIELD_TYPE ? "type" :
				head->field == BIBTEX_FIELD_VOLUME ? "volume" :
				head->field == BIBTEX_FIELD_YEAR ? "year" : "HUH!",
				head->value);
		}
		head = head->next;
	}
	fprintf(ostream, "}");
}

void bibtex_free(struct bibtex_object *obj)
{
	free(obj->identifier);
	struct bibtex_entry *head = obj->head;
	struct bibtex_entry *next;
	while(head != NULL) {
		if(head->field == BIBTEX_FIELD_OTHER){
			free(head->key);
		}
		free(head->value);
		next = head->next;
		free(head);
		head = next;
	}
	free(obj);
}
