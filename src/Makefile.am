bin_PROGRAMS   = light
light_SOURCES  = main.c light.c light.h helpers.c helpers.h
light_CPPFLAGS = -I../include -D_XOPEN_SOURCE=500
light_CFLAGS   = -W -Wall -Wextra -std=gnu99

install-exec-hook:
	chmod 4755 $(DESTDIR)$(bindir)/light
