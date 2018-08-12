SUBDIRS        = src
dist_man1_MANS = light.1
doc_DATA       = README.md LICENSE ChangeLog.md
EXTRA_DIST     = README.md LICENSE ChangeLog.md

if UDEV
udev_DATA      = 90-backlight.rules
EXTRA_DIST    += $(top_srcdir)/90-backlight.rules
endif

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
		printf "$$file    \tDistribution tarball\n";			\
		printf "$$file.md5\t"; cat ../$$file.md5 | cut -f1 -d' ';	\
	done
