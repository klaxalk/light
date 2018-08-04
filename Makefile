PREFIX=$(DESTDIR)/usr
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man/man1

CC=gcc
CFLAGS=-std=c89 -O2 -pedantic -Wall -I"./include" -D_XOPEN_SOURCE=500

light: src/helpers.c src/light.c src/main.c
	$(CC) $(CFLAGS) -g -o $@ $^

install: light
	mkdir -p $(BINDIR)
	cp -f ./light $(BINDIR)/light
	chown root $(BINDIR)/light
	chmod 4755 $(BINDIR)/light
	mkdir -p $(MANDIR)
	cp -f light.1 $(MANDIR)

uninstall:
	rm $(BINDIR)/light
	rm -rf /etc/light
	rm $(MANDIR)/light.1.gz

clean:
	rm -vfr *~ light light.1.gz

.PHONY: man install uninstall clean
