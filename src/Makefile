PROGRAM:=btd
OBJS:=config.o misc.o db.o bibtex.o xdg.o

CFLAGS:=-g -O3 -Wextra -Wall -Werror -std=gnu11
LDLIBS:=-lsqlite3

all: $(PROGRAM)

$(PROGRAM): $(PROGRAM).o $(OBJS)
	$(CC) $< $(OBJS) $(LDLIBS) -o $@

clean:
	$(RM) -v $(PROGRAM) $(PROGRAM).o $(OBJS)
