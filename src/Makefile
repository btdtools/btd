PROGRAM:=btd
OBJS:=config.o misc.o db.o bibtex.o

CFLAGS:=-g -O3 -Wextra -Wall -Werror -std=gnu11
LDLIBS:=-lsqlite3

all: $(PROGRAM) bt_lex.c

$(PROGRAM): $(PROGRAM).o $(OBJS)
	$(CC) $< $(OBJS) $(LDLIBS) -o $@

clean:
	$(RM) -v $(PROGRAM) $(PROGRAM).o $(OBJS)
