#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bibtex.h"
#include "misc.h"

#define BUFSIZE 4048

#define EXPECT(c, err) {\
	if(c == EOF) asprintf(errmsg, "Early EOF\n"); \
	else asprintf(errmsg, "Expected " err " but got '%c'\n", c);\
	bibtex_free(obj);\
	return NULL;}
#define SKIP_WHITE(c, istream)\
	while(isspace(c))\
		c = fgetc(istream)

char *bibtex_field_str(bibtex_field field, char *other)
{
	switch(field){
	case BIBTEX_FIELD_ADDRESS: return "address";
	case BIBTEX_FIELD_ANNOTE: return "annote";
	case BIBTEX_FIELD_AUTHOR: return "author";
	case BIBTEX_FIELD_BOOKTITLE: return "booktitle";
	case BIBTEX_FIELD_CHAPTER: return "chapter";
	case BIBTEX_FIELD_CROSSREF: return "crossref";
	case BIBTEX_FIELD_EDITION: return "edition";
	case BIBTEX_FIELD_EDITOR: return "editor";
	case BIBTEX_FIELD_HOWPUBLISHED: return "howpublished";
	case BIBTEX_FIELD_INSTITUTION: return "institution";
	case BIBTEX_FIELD_JOURNAL: return "journal";
	case BIBTEX_FIELD_KEY: return "key";
	case BIBTEX_FIELD_MONTH: return "month";
	case BIBTEX_FIELD_NOTE: return "note";
	case BIBTEX_FIELD_NUMBER: return "number";
	case BIBTEX_FIELD_ORGANIZATION: return "organization";
	case BIBTEX_FIELD_PAGES: return "pages";
	case BIBTEX_FIELD_PUBLISHER: return "publisher";
	case BIBTEX_FIELD_SCHOOL: return "school";
	case BIBTEX_FIELD_SERIES: return "series";
	case BIBTEX_FIELD_TITLE: return "title";
	case BIBTEX_FIELD_TYPE: return "type";
	case BIBTEX_FIELD_VOLUME: return "volume";
	case BIBTEX_FIELD_YEAR: return "year";
	default:;
	}
	return other;
}

bibtex_field bibtex_str_field(char *str)
{
	if(strcmp(str, "address") == 0) return BIBTEX_FIELD_ADDRESS;
	if(strcmp(str, "annote") == 0) return BIBTEX_FIELD_ANNOTE;
	if(strcmp(str, "author") == 0) return BIBTEX_FIELD_AUTHOR;
	if(strcmp(str, "booktitle") == 0) return BIBTEX_FIELD_BOOKTITLE;
	if(strcmp(str, "chapter") == 0) return BIBTEX_FIELD_CHAPTER;
	if(strcmp(str, "crossref") == 0) return BIBTEX_FIELD_CROSSREF;
	if(strcmp(str, "edition") == 0) return BIBTEX_FIELD_EDITION;
	if(strcmp(str, "editor") == 0) return BIBTEX_FIELD_EDITOR;
	if(strcmp(str, "howpublished") == 0) return BIBTEX_FIELD_HOWPUBLISHED;
	if(strcmp(str, "institution") == 0) return BIBTEX_FIELD_INSTITUTION;
	if(strcmp(str, "journal") == 0) return BIBTEX_FIELD_JOURNAL;
	if(strcmp(str, "key") == 0) return BIBTEX_FIELD_KEY;
	if(strcmp(str, "month") == 0) return BIBTEX_FIELD_MONTH;
	if(strcmp(str, "note") == 0) return BIBTEX_FIELD_NOTE;
	if(strcmp(str, "number") == 0) return BIBTEX_FIELD_NUMBER;
	if(strcmp(str, "organization") == 0) return BIBTEX_FIELD_ORGANIZATION;
	if(strcmp(str, "pages") == 0) return BIBTEX_FIELD_PAGES;
	if(strcmp(str, "publisher") == 0) return BIBTEX_FIELD_PUBLISHER;
	if(strcmp(str, "school") == 0) return BIBTEX_FIELD_SCHOOL;
	if(strcmp(str, "series") == 0) return BIBTEX_FIELD_SERIES;
	if(strcmp(str, "title") == 0) return BIBTEX_FIELD_TITLE;
	if(strcmp(str, "type") == 0) return BIBTEX_FIELD_TYPE;
	if(strcmp(str, "volume") == 0) return BIBTEX_FIELD_VOLUME;
	if(strcmp(str, "year") == 0) return BIBTEX_FIELD_YEAR;
	return BIBTEX_FIELD_OTHER;
}


char *bibtex_entry_str(bibtex_entrytype type)
{
	switch(type){
	case BIBTEX_ENTRY_ARTICLE: return "article";
	case BIBTEX_ENTRY_BOOK: return "book";
	case BIBTEX_ENTRY_BOOKLET: return "booklet";
	case BIBTEX_ENTRY_CONFERENCE: return "conference";
	case BIBTEX_ENTRY_INBOOK: return "inbook";
	case BIBTEX_ENTRY_INCOLLECTION: return "incollection";
	case BIBTEX_ENTRY_INPROCEEDINGS: return "inproceedings";
	case BIBTEX_ENTRY_MANUAL: return "manual";
	case BIBTEX_ENTRY_MASTERSTHESIS: return "mastersthesis";
	case BIBTEX_ENTRY_MISC: return "misc";
	case BIBTEX_ENTRY_PHDTHESIS: return "phdthesis";
	case BIBTEX_ENTRY_PROCEEDINGS: return "proceedings";
	case BIBTEX_ENTRY_TECHREPORT: return "techreport";
	case BIBTEX_ENTRY_UNPUBLISHED: return "unpublished";
	default:;
	}
	return NULL;
}

bibtex_entrytype bibtex_str_entry(char *str)
{
	if(strcmp(str, "article") == 0) return BIBTEX_ENTRY_ARTICLE;
	if(strcmp(str, "book") == 0) return BIBTEX_ENTRY_BOOK;
	if(strcmp(str, "booklet") == 0) return BIBTEX_ENTRY_BOOKLET;
	if(strcmp(str, "conference") == 0) return BIBTEX_ENTRY_CONFERENCE;
	if(strcmp(str, "inbook") == 0) return BIBTEX_ENTRY_INBOOK;
	if(strcmp(str, "incollection") == 0) return BIBTEX_ENTRY_INCOLLECTION;
	if(strcmp(str, "inproceedings") == 0) return BIBTEX_ENTRY_INPROCEEDINGS;
	if(strcmp(str, "manual") == 0) return BIBTEX_ENTRY_MANUAL;
	if(strcmp(str, "mastersthesis") == 0) return BIBTEX_ENTRY_MASTERSTHESIS;
	if(strcmp(str, "misc") == 0) return BIBTEX_ENTRY_MISC;
	if(strcmp(str, "phdthesis") == 0) return BIBTEX_ENTRY_PHDTHESIS;
	if(strcmp(str, "proceedings") == 0) return BIBTEX_ENTRY_PROCEEDINGS;
	if(strcmp(str, "techreport") == 0) return BIBTEX_ENTRY_TECHREPORT;
	if(strcmp(str, "unpublished") == 0) return BIBTEX_ENTRY_UNPUBLISHED;
	return BIBTEX_ENTRY_UNKNOWN;
}

char *bibtex_get_field_str(struct bibtex_object *obj, char *fieldstr)
{
	bibtex_field field = bibtex_str_field(fieldstr);
	for(struct bibtex_entry *hd = obj->head; hd != NULL; hd = hd->next){
		if((hd->field == BIBTEX_FIELD_OTHER && strcmp(fieldstr, hd->key) == 0)
				|| hd->field == field){
			return hd->value;
		}
	}
	return NULL;
}

struct bibtex_object *bibtex_parse(FILE *istream, char **errmsg)
{
	struct bibtex_object *obj = malloc(sizeof (struct bibtex_object));
	struct bibtex_entry *current;
	char buf[BUFSIZE+1];
	char c;
	int bufloc = 0;
	int depth;

	//Init object
	obj->identifier = NULL;
	obj->head = NULL;

	//Skip whitespace
	c = fgetc(istream);
	SKIP_WHITE(c, istream);

	//Read @
	if(c != '@')
		EXPECT(c, "'@'");

	//Read entrytype
	while(isalpha(c = fgetc(istream)) && bufloc < BUFSIZE)
		buf[bufloc++] = c;
	buf[bufloc] = '\0';
	if((obj->type = bibtex_str_entry(buf)) == BIBTEX_ENTRY_UNKNOWN){
		asprintf(errmsg, "Invalid bibtex entry type: '%s'\n", buf);
		bibtex_free(obj);
		return NULL;
	}

	//Read openbrace
	if(c != '{')
		EXPECT(c, "'{'");
	//Read ident
	bufloc = 0;
	while((isalnum(c = fgetc(istream)) || 
			c == '|' || c == '+' || c == '_' || c == '-') 
			&& bufloc < BUFSIZE) {
		buf[bufloc++] = c;
	}
	buf[bufloc] = '\0';
	if(bufloc == 0){
		asprintf(errmsg, "Identifier can't be empty\n");
		bibtex_free(obj);
		return NULL;
	}
	obj->identifier = safe_strdup(buf);
	
	//Skip whitespace
	SKIP_WHITE(c, istream);

	//Read key-valuepairs
	while(c == ','){
		current = malloc(sizeof (struct bibtex_entry));
		current->key = NULL;
		//skip whitespace
		c = fgetc(istream);
		SKIP_WHITE(c, istream);

		//parse ident
		bufloc = 0;
		buf[bufloc++] = c;
		while(isalpha(c = fgetc(istream)) && bufloc < BUFSIZE) {
			buf[bufloc++] = c;
		}
		buf[bufloc] = '\0';
		if((current->field = bibtex_str_entry(buf)) == BIBTEX_FIELD_OTHER)
			current->key = safe_strdup(buf);

		//skip whitespace
		SKIP_WHITE(c, istream);

		//parse '='
		if(c != '=')
			EXPECT(c, "'='");

		//skip whitespace
		c = fgetc(istream);
		SKIP_WHITE(c, istream);

		//Parse ident
		bufloc = 0;
		buf[bufloc++] = c;
		if(isdigit(c))
			while(isdigit(c = fgetc(istream)))
				buf[bufloc++] = c;
		else if (c == '"') {
			while((c = fgetc(istream)) != EOF){
				buf[bufloc++] = c;
				if(c == '\\')
					buf[bufloc++] = fgetc(istream);
				else if(c == '"')
					break;
			}
			c = fgetc(istream);
		} else if (c == '{') {
			depth = 1;
			while((c = fgetc(istream) != EOF)){
				buf[bufloc++] = c;
				if(c == '\\')
					buf[bufloc++] = fgetc(istream);
				else if(c == '{')
					depth++;
				else if (c == '}')
					if(--depth == 0)
						break;
			}
			c = fgetc(istream);
		} else {
			EXPECT(c, "digit, '\"' or '{'");
			bibtex_free(obj);
			return NULL;
		}
		buf[bufloc] = '\0';

		//skip whitespace
		SKIP_WHITE(c, istream);

		current->value = safe_strdup(buf);
		current->next = obj->head;
		obj->head = current;
	}
	if(c != '}')
		EXPECT(c, "'}'");
	return obj;
}

void bibtex_print(FILE *ostream, struct bibtex_object *obj)
{
	struct bibtex_entry *head = obj->head;
	fprintf(ostream, "@%s{%s\n", bibtex_entry_str(obj->type), obj->identifier);
	for(struct bibtex_entry *hd = obj->head; hd != NULL; hd = hd->next){
		fprintf(ostream, "\t,%s = %s\n",
				bibtex_field_str(head->field, head->key), head->value);
	}
	fprintf(ostream, "}");
}

void bibtex_free(struct bibtex_object *obj)
{
	struct bibtex_entry *head = obj->head;
	struct bibtex_entry *next;
	free(obj->identifier);
	while(head != NULL) {
		if(head->field == BIBTEX_FIELD_OTHER)
			free(head->key);
		free(head->value);
		next = head->next;
		free(head);
		head = next;
	}
	free(obj);
}
