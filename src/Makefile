PROGRAM:=btd
OBJS:=$(addsuffix .o,$(basename $(wildcard *.c)))

CFLAGS:=-g -Wextra -Wall -Werror -pedantic -std=gnu11
LDLIBS:=-lsqlite3

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(LINK.c) $(LDLIBS) $^ $(OUTPUT_OPTION)

clean:
	$(RM) $(PROGRAM) $(OBJS)
