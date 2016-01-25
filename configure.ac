

AC_PREREQ(2.65)
AC_INIT([eds-to-idevice], [0.1.0, nospam@nowhere.com])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign dist-bzip2 no-dist-gzip subdir-objects])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_HEADER_STDC
AC_PROG_INSTALL
AC_PROG_LN_S
LT_INIT
PKG_PROG_PKG_CONFIG
AC_PROG_LIBTOOL

dnl warnings bits, copied from rhythmbox configure.in
dnl Turn on the additional warnings last, so -Werror doesn't affect other tests.
AC_ARG_ENABLE(more-warnings,
[  --enable-more-warnings  Maximum compiler warnings],
set_more_warnings="$enableval",[
if test -d "$srcdir/.svn" || test -d "$srcdir/{arch}" || test -d "$srcdir/CVS" || test -d "$srcdir/_darcs" || test -d "$srcdir/.git"; then
	set_more_warnings=yes
else
	set_more_warnings=no
fi
])
ETI_CFLAGS_NOWARN=$ETI_CFLAGS
AC_SUBST(ETI_CFLAGS_NOWARN)
AC_MSG_CHECKING(for more warnings, including -Werror)
if test "$GCC" = "yes" -a "$set_more_warnings" != "no"; then
	AC_MSG_RESULT(yes)
	WARN_CFLAGS="\
	-Wcomment -Wformat -Wnonnull -Wimplicit-int -Wimplicit \
	-Wmain -Wmissing-braces -Wparentheses -Wsequence-point \
	-Wreturn-type -Wswitch -Wtrigraphs -Wunused-function \
	-Wunused-label -Wunused-value \
	-Wchar-subscripts -Wmissing-declarations -Wmissing-prototypes \
	-Wnested-externs -Wpointer-arith \
	-Wcast-align -Wall \
	-Werror -std=gnu89"

	if echo "$CFLAGS" | grep -q -- '-O' && echo "$CFLAGS" | grep -q -v -- '-O0'; then
		WARN_CFLAGS="-Wuninitialized $WARN_CFLAGS"
	fi

	for option in $WARN_CFLAGS; do
		SAVE_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $option"
		AC_MSG_CHECKING([whether gcc understands $option])
		AC_TRY_COMPILE([], [],
			has_option=yes,
			has_option=no,)
		if test x$has_option = xyes; then
			ETI_CFLAGS="$ETI_CFLAGS $option"
		fi
		AC_MSG_RESULT($has_option)
		CFLAGS="$SAVE_CFLAGS"
		unset has_option
		unset SAVE_CFLAGS
	done
	unset option
	AC_SUBST(WARN_CFLAGS)

	SAVE_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -Wno-error"
	AC_TRY_COMPILE([], [],
		WNOERROR_CFLAGS="-Wno-error",
		WNOERROR_CFLAGS="")
	AC_SUBST(WNOERROR_CFLAGS)
	CFLAGS="$SAVE_CFLAGS"
	unset SAVE_CFLAGS
	unset has_wnoerror
else
	AC_MSG_RESULT(no)
fi

PKG_CHECK_MODULES(LIBIMOBILEDEVICE, [libimobiledevice-1.0 >= 1.1])
PKG_CHECK_MODULES(LIBPLIST, [libplist])
dnl need glib 2.26 for GDateTime
PKG_CHECK_MODULES(GLIB2, [glib-2.0 >= 2.26])
PKG_CHECK_MODULES(EDS, [libebook-1.2])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
