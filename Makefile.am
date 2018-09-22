SUBDIRS        = src
dist_man1_MANS = light.1
doc_DATA       = README.md COPYING ChangeLog.md
EXTRA_DIST     = README.md COPYING ChangeLog.md

if UDEV
udev_DATA      = 90-backlight.rules
EXTRA_DIST    += $(top_srcdir)/90-backlight.rules
endif

# lintian --profile debian -i -I --show-overrides ../$PKG.changes
deb:
	dpkg-buildpackage -uc -us -B

#
# Target to run when building a release
#
release: distcheck
	@for file in $(DIST_ARCHIVES); do	\
		md5sum $$file > ../$$file.md5;	\
	done
	@mv $(DIST_ARCHIVES) ../
	@echo
	@echo "Resulting release files:"
	@echo "================================================================="
	@for file in $(DIST_ARCHIVES); do					\
		printf "%-32s Distribution tarball\n" $$file;			\
		printf "%-32s " $$file.md5; cat ../$$file.md5 | cut -f1 -d' ';	\
	done
	@for file in `cd ..; ls $(PACKAGE)_$(VERSION)*`; do			\
		printf "%-32s Debian/Ubuntu file\n" $$file;			\
	done
