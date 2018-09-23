AC_INIT([light], [1.2], [https://github.com/haikarainen/light/issues])
AM_INIT_AUTOMAKE([1.11 foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/light.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_PROG_CC
AC_PROG_INSTALL
AC_HEADER_STDC

AC_ARG_WITH([udev],
	AS_HELP_STRING([--with-udev@<:@=PATH@:>@], [use udev instead of SUID root, optional rules.d path]),
	[udev=$withval], [udev=no])

AC_MSG_CHECKING(for udev rules.d)
AS_IF([test "x$udev" != "xno"], [
	AS_IF([test "x$udev" = "xyes"], [
		udevdir="\${prefix}/lib/udev/rules.d"
	],[
		udevdir="$udev"
	])
	AC_SUBST(udevdir)
	AC_MSG_RESULT([$udevdir])
],[
	AC_MSG_RESULT([disabled, classic SUID root mode])
])

# Allow classic SUID root behavior if udev rule is not used
AM_CONDITIONAL(UDEV,    [test "x$udev" != "xno"])
AM_CONDITIONAL(CLASSIC, [test "x$udev"  = "xno"])

AC_OUTPUT
