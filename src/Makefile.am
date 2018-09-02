bin_PROGRAMS   = light
light_SOURCES  = main.c light.c light.h helpers.c helpers.h impl/sysfs.c impl/sysfs.h
light_CPPFLAGS = -I../include -D_GNU_SOURCE
light_CFLAGS   = -W -Wall -Wextra -std=gnu99 -Wno-type-limits

if CLASSIC
install-exec-hook:
	chmod 4755 $(DESTDIR)$(bindir)/light
endif
