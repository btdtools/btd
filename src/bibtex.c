#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bibtex.h"
#include "misc.h"

#define BUFSIZE 4048

#define EXPECT(ex, c) \
	if(c == EOF) asprintf(errmsg, "Early EOF\n"); \
	else asprintf(errmsg, "Expected " ex " but got '%c'\n", c);

char *safe_strdup(const char *s)
{
	char *r = strdup(s);
	if(r == NULL){
		die("strup() failed");
	}
	return r;
}

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

struct bibtex_object *bibtex_parse(FILE *istream, char **errmsg)
{
	struct bibtex_object *obj = malloc(sizeof (struct bibtex_object));
	char buf[BUFSIZE+1];
	char c;
	int bufloc = 0;

	//Skip whitespace
	do {
		c = fgetc(istream);
	} while(isspace(c));

	//Read @
	if(c != '@'){
		EXPECT("'@'", c);
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
		asprintf(errmsg, "Invalid bibtex entry type: '%s'\n", buf);
		free(obj);
		return NULL;
	}

	//Read openbrace and identifier
	if(c != '{'){
		EXPECT("'{'", c);
		free(obj);
		return NULL;
	}
	bufloc = 0;
	while((isalnum(c = fgetc(istream)) || 
			c == '|' || c == '+' || c == '_' || c == '-') 
			&& bufloc < BUFSIZE) {
		buf[bufloc++] = c;
	}
	buf[bufloc] = '\0';
	if(bufloc == 0){
		asprintf(errmsg, "Identifier can't be empty\n");
		free(obj);
		return NULL;
	}
	obj->identifier = safe_strdup(buf);

	//Read key-valuepairs
	obj->head = NULL;
	while(c == ','){
		struct bibtex_entry *current = malloc(sizeof (struct bibtex_entry));
		current->key = NULL;
		//skip whitespace
		do{
			c = fgetc(istream);
		} while(isspace(c));

		//parse ident
		bufloc = 0;
		buf[bufloc++] = c;
		while(isalpha(c = fgetc(istream)) && bufloc < BUFSIZE) {
			buf[bufloc++] = c;
		}
		buf[bufloc] = '\0';
		if(strcmp(buf, "address") == 0)
			current->field = BIBTEX_FIELD_ADDRESS;
		else if(strcmp(buf, "annote") == 0)
			current->field = BIBTEX_FIELD_ANNOTE;
		else if(strcmp(buf, "author") == 0)
			current->field = BIBTEX_FIELD_AUTHOR;
		else if(strcmp(buf, "booktitle") == 0)
			current->field = BIBTEX_FIELD_BOOKTITLE;
		else if(strcmp(buf, "chapter") == 0)
			current->field = BIBTEX_FIELD_CHAPTER;
		else if(strcmp(buf, "crossref") == 0)
			current->field = BIBTEX_FIELD_CROSSREF;
		else if(strcmp(buf, "edition") == 0)
			current->field = BIBTEX_FIELD_EDITION;
		else if(strcmp(buf, "editor") == 0)
			current->field = BIBTEX_FIELD_EDITOR;
		else if(strcmp(buf, "howpublished") == 0)
			current->field = BIBTEX_FIELD_HOWPUBLISHED;
		else if(strcmp(buf, "institution") == 0)
			current->field = BIBTEX_FIELD_INSTITUTION;
		else if(strcmp(buf, "journal") == 0)
			current->field = BIBTEX_FIELD_JOURNAL;
		else if(strcmp(buf, "key") == 0)
			current->field = BIBTEX_FIELD_KEY;
		else if(strcmp(buf, "month") == 0)
			current->field = BIBTEX_FIELD_MONTH;
		else if(strcmp(buf, "note") == 0)
			current->field = BIBTEX_FIELD_NOTE;
		else if(strcmp(buf, "number") == 0)
			current->field = BIBTEX_FIELD_NUMBER;
		else if(strcmp(buf, "organization") == 0)
			current->field = BIBTEX_FIELD_ORGANIZATION;
		else if(strcmp(buf, "pages") == 0)
			current->field = BIBTEX_FIELD_PAGES;
		else if(strcmp(buf, "publisher") == 0)
			current->field = BIBTEX_FIELD_PUBLISHER;
		else if(strcmp(buf, "school") == 0)
			current->field = BIBTEX_FIELD_SCHOOL;
		else if(strcmp(buf, "series") == 0)
			current->field = BIBTEX_FIELD_SERIES;
		else if(strcmp(buf, "title") == 0)
			current->field = BIBTEX_FIELD_TITLE;
		else if(strcmp(buf, "type") == 0)
			current->field = BIBTEX_FIELD_TYPE;
		else if(strcmp(buf, "volume") == 0)
			current->field = BIBTEX_FIELD_VOLUME;
		else if(strcmp(buf, "year") == 0)
			current->field = BIBTEX_FIELD_YEAR;
		else {
			current->field = BIBTEX_FIELD_OTHER;
			current->key = safe_strdup(buf);
		}
		//skip whitespace
		while(isspace(c)){
		   c = fgetc(istream);
		}

		//parse '='
		if(c != '='){
			EXPECT("'='", c);
			if(current->field = BIBTEX_FIELD_OTHER)
				free(current->key);
			free(current);
			free(obj->identifier);
			free(obj);
			return NULL;
		}

		//skip whitespace
		do {
			c = fgetc(istream);
		} while(isspace(c));
		bufloc = 0;
		if(isdigit(c)){
			buf[bufloc++] = c;
			while(isdigit(c = fgetc(istream))){
				buf[bufloc++] = c;
			}
			buf[bufloc] = '\0';
		} else if (c == '"') {
			buf[bufloc++] = c;
			while(c = fgetc(istream)){
				buf[bufloc++] = c;
				if(c == '\\') {
					buf[bufloc++] = fgetc(istream);
				} else if(c == '"') {
					break;
				}
			}
			c = fgetc(istream);
			buf[bufloc] = '\0';
		} else if (c == '{') {
			int depth = 1;
			buf[bufloc++] = c;
			while(c = fgetc(istream)){
				buf[bufloc++] = c;
				if(c == '\\') {
					buf[bufloc++] = fgetc(istream);
				} else if(c == '{'){
					depth++;
				} else if (c == '}'){
					depth--;
					if(depth == 0){
						break;
					}
				}
			}
			c = fgetc(istream);
		} else {
			EXPECT("digit, '\"' or '{'", c);
			if(current->field = BIBTEX_FIELD_OTHER)
				free(current->key);
			free(current);
			free(obj->identifier);
			free(obj);
			return NULL;
		}
		//skip whitespace
		while(isspace(c)){
		   c = fgetc(istream);
		}

		current->value = safe_strdup(buf);
		current->next = obj->head;
		obj->head = current;
	}
	if(c != '}'){
		EXPECT("'}'", c);
		free(obj->identifier);
		free(obj);
		return NULL;
	}
	return obj;
}

void bibtex_print(FILE *ostream, struct bibtex_object *obj)
{
	struct bibtex_entry *head = obj->head;
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

//Small stub for testing the parser
//int main(void)
//{
//	char *errmsg;
//	struct bibtex_object *bt = bibtex_parse(stdin, &errmsg);
//	if(bt == NULL){
//		printf("err: %s\n", errmsg);
//	} else {
//		bibtex_print(stdout, bt);
//		bibtex_free(bt);
//	}
//	return 0;
//}
