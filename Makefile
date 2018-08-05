ifeq ($(PREFIX),)
	PREFIX := /usr
endif
BINDIR=$(DESTDIR)$(PREFIX)/bin
MANDIR=$(DESTDIR)$(PREFIX)/share/man/man1

CC=gcc
CFLAGS=-std=c89 -O2 -pedantic -Wall -I"./include" -D_XOPEN_SOURCE=500

light: src/helpers.c src/light.c src/main.c
	$(CC) $(CFLAGS) -g -o $@ $^

install: light
	install -dZ $(BINDIR)
	install -DZ -m 4755 ./light -t $(BINDIR)
	install -dZ $(MANDIR)
	install -DZ light.1 -t $(MANDIR)

uninstall:
	rm -f $(BINDIR)/light
	rm -rf /etc/light
	rm -f $(MANDIR)/light.1.gz

clean:
	rm -vfr *~ light light.1.gz

.PHONY: man install uninstall clean
