SHELL:=/bin/bash
PROGRAM:=btd
HEADERS:=doc.h
OBJS:=$(addsuffix .o,$(basename $(wildcard *.c)))

CFLAGS=-g -Wextra -Wall -Werror -pedantic -std=gnu11
LDLIBS=-lsqlite3 -luuid

all: $(PROGRAM)

doc.h: ../PROTOCOL.txt
	echo -e "#ifndef DOC_H\n#define DOC_H\n\nconst char *protocol_doc =\\" > $@
	sed -e 's/$$/\\n"\\/g' -e 's/^/\t"/g' $< >> $@
	echo -e ";\n#endif" >> $@

$(PROGRAM): $(HEADERS) $(OBJS)
	$(LINK.c) $(LDLIBS) $^ $(OUTPUT_OPTION)

clean:
	$(RM) $(PROGRAM) $(OBJS) $(HEADERS)
