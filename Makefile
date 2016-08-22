PROGRAM:=btd

PREFIX:=/usr/local

BINDIR:=bin
SRCDIR:=src

HELP2MAN:=help2man
HELP2MANFLAGS:=--section=1 --no-discard-stderr --no-info

INSTALL:=install
INSTALLFLAGS:=-v -D

GZIP:=gzip
GZIPFLAGS:=-9 --verbose

$(BINDIR)/$(PROGRAM): $(SRCDIR)/$(PROGRAM) $(BINDIR)
	cp $< $@

install: $(BINDIR)/$(PROGRAM)
	$(INSTALL) $(INSTALLFLAGS) $< -t $(PREFIX)/bin
	$(INSTALL) $(INSTALLFLAGS) $(PROGRAM).1.gz -t $(PREFIX)/share/man/man1
	$(INSTALL) $(INSTALLFLAGS) LICENSE README.md config.example -t $(PREFIX)/share/doc/$(PROGRAM)

uninstall:
	$(RM) -rv $(PREFIX)/bin/$(PROGRAM) $(PREFIX)/share/man/man1/$(PROGRAM).1.gz $(PREFIX)/share/doc/$(PROGRAM)

man: $(PROGRAM).1.gz 

$(PROGRAM).1.gz: $(BINDIR)/$(PROGRAM)
	$(HELP2MAN) $(HELP2MANFLAGS) $< | $(GZIP) $(GZIPFLAGS) > $@

$(SRCDIR)/%:
	make -C $(SRCDIR)

$(BINDIR):
	mkdir $@

clean:
	$(RM) -rv $(BINDIR) 
	make -C $(SRCDIR) clean

.PHONY: man clean install uninstall
