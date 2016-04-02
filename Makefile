PROGRAM:=btd

BINDIR:=bin
SRCDIR:=src

HELP2MAN:=help2man
HELP2MANFLAGS:=--section=1 --no-discard-stderr --no-info

GZIP:=gzip
GZIPFLAGS:=-9 --verbose

$(BINDIR)/$(PROGRAM): $(SRCDIR)/$(PROGRAM) $(BINDIR)
	cp $< $@

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
