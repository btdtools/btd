PROGRAM:=btd
OBJS:=config.o misc.o

HELP2MAN:=help2man
HELP2MANFLAGS:=--section=1 --no-discard-stderr --no-info

GZIP:=gzip
GZIPFLAGS:=-9 --verbose

CFLAGS:=-g -O3 -Wextra -Wall -Werror -std=gnu11
LDLIBS:=-lbtparse

all: $(PROGRAM)

man: $(PROGRAM).1.gz

$(PROGRAM): $(PROGRAM).o $(OBJS)
	$(CC) $< $(OBJS) $(LDLIBS) -o $@

%.1.gz: %
	$(HELP2MAN) $(HELP2MANFLAGS) ./$< | $(GZIP) $(GZIPFLAGS) > $@

clean:
	$(RM) -v $(PROGRAM) $(PROGRAM).o $(OBJS) $(PROGRAM).1.gz
