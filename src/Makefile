PROGRAM:=btd
OBJS:=config.o misc.o db.o

CFLAGS:=-g -O3 -Wextra -Wall -Werror -std=gnu11
LDLIBS:=-lbtparse -lsqlite3

all: $(PROGRAM)

$(PROGRAM): $(PROGRAM).o $(OBJS)
	$(CC) $< $(OBJS) $(LDLIBS) -o $@

clean:
	$(RM) -v $(PROGRAM) $(PROGRAM).o $(OBJS)
