bin_PROGRAMS   = light
light_SOURCES  = main.c light.c ../include/light.h helpers.c ../include/helpers.h
light_CPPFLAGS = -I../include -D_XOPEN_SOURCE=500
light_CFLAGS   = -W -Wall -Wextra -pedantic -std=c89

install-exec-hook:
	chmod 4755 $(DESTDIR)$(bindir)/light
