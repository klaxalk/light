bin_PROGRAMS   = light
light_SOURCES  = main.c light.c light.h helpers.c helpers.h impl/sysfs.c impl/sysfs.h impl/util.h impl/util.c impl/razer.h impl/razer.c
light_CPPFLAGS = -I../include -D_GNU_SOURCE
light_CFLAGS   = -W -Wall -Wextra -std=gnu99 -Wno-type-limits -Wno-format-truncation -Wno-unused-parameter -fcommon

if CLASSIC
install-exec-hook:
	chmod 6755 $(DESTDIR)$(bindir)/light
endif
