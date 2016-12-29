AC_PREREQ(2.54)

AC_INIT(src/x/main.c)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_PROG_MAKE_SET

AM_SANITY_CHECK
AM_INIT_AUTOMAKE(somax, 1.3)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_LN_S
AC_ISC_POSIX
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AM_PROG_CC_STDC

AC_C_CONST
AC_C_INLINE

AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS(stdio.h sys/stat.h sys/types.h unistd.h fcntl.h string.h dirent.h malloc.h pwd.h grp.h)

CFLAGS="$CFLAGS -Wall"

ALL_LINGUAS="it fr el es"
AM_GNU_GETTEXT

AC_FUNC_VPRINTF

AC_CHECK_HEADER(dlfcn.h)
AC_CHECK_FUNC(dlopen, ,
  AC_CHECK_LIB(dl, dlopen, ,
    AC_MSG_ERROR(dynamic load support is required to compile somax)
  )
)

AC_CHECK_HEADER(confuse.h,
  AC_CHECK_LIB(confuse, cfg_init, NEED_CONFUSE=no, NEED_CONFUSE=yes),
  NEED_CONFUSE=yes
)

AH_VERBATIM([ENABLE_CONFUSE],[#undef ENABLE_CONFUSE])

if test "$NEED_CONFUSE" = yes; then
  AC_DEFINE(ENABLE_CONFUSE, 1, [Define this symbol if you have libconfuse])
else
  LIBS="$LIBS -lconfuse"
fi

AM_CONDITIONAL(ENABLE_CONFUSE, test "$NEED_CONFUSE" = yes)

AC_ARG_WITH(somad, AC_HELP_STRING([--with-somad=[where/is/somad]],[set your somad]),
  if ! test x$withval = x; then
    AC_CHECK_FILES($withval, somad=$withval,
      AC_MSG_ERROR(I can't find $withval program)
    )
  fi
)

if test x$somad = x; then
  AC_PATH_PROG(somad, somad)
fi

if test "$somad" != ""; then
  CFLAGS="$CFLAGS -DSOMAD_CMD=\"\\\"$somad\\\"\""

  VTE="vte"
  AM_CONDITIONAL(ENABLE_VTE, ! test "$VTE" = "")
  AC_DEFINE(ENABLE_VTE, 1, [Lib Vte support])
else
  VTE=
fi

AH_VERBATIM([ENABLE_VTE],[#undef ENABLE_VTE])

AC_ARG_ENABLE(gtksourceview,AC_HELP_STRING([--enable-gtksourceview],[enable gtksourceview library support (default=yes)]), gtksourceview=$enableval, gtksourceview=yes) 
if test "$gtksourceview" = yes; then
  GTKSOURCEVIEW="gtksourceview-1.0"
  AM_CONDITIONAL(ENABLE_GTKSOURCEVIEW, ! test "$GTKSOURCEVIEW" = "")
  AC_DEFINE(ENABLE_GTKSOURCEVIEW, 1, [GtkSourceView support])
else
  GTKSOURCEVIEW=
fi

AH_VERBATIM([ENABLE_GTKSOURCEVIEW],[#undef ENABLE_GTKSOURCEVIEW])

AC_ARG_ENABLE(gnomeprint,AC_HELP_STRING([--enable-gnomeprint],[enable gnome print (default=yes)]), gnomeprint=$enableval, gnomeprint=yes) 
if test "$gnomeprint" = yes; then
  PKG_CHECK_MODULES(lib_gnome_print, [
    libxml-2.0 >= 2.0.0
    glib-2.0 >= 2.0.0 
    gmodule-2.0 >= 2.0.0 
    gthread-2.0 >= 2.0.0
    gtk+-2.0 >= 2.0.0 
    pango >= 1.5.0
    libgnomeprint-2.2 >= 2.8.0
    libgnomeprintui-2.2 >= 2.8.0
    $VTE
    $GTKSOURCEVIEW
  ], gnomeprint=yes, gnomeprint=no)
fi

if test "$gnomeprint" = no; then
  PKG_CHECK_MODULES(graphic, [
    glib-2.0 >= 2.0.0 
    gmodule-2.0 >= 2.0.0 
    gthread-2.0 >= 2.0.0
    gtk+-2.0 >= 2.0.0 
    pango >= 1.0.0
    $VTE
    $GTKSOURCEVIEW
  ])
fi

AM_CONDITIONAL(ENABLE_PRINT, test "$gnomeprint" = yes)

if test "$gnomeprint" = no; then
  LDFLAGS="$LDFLAGS $graphic_LIBS"
  CFLAGS="$CFLAGS $graphic_CFLAGS"
else
  LDFLAGS="$LDFLAGS $lib_gnome_print_LIBS"
  CFLAGS="$CFLAGS $lib_gnome_print_CFLAGS"
fi

AC_PATH_PROG(somaconfigpath, soma-config)
AC_MSG_CHECKING(for soma through soma-config)
if test "$somaconfigpath" != ""; then
  MAJOR=`$somaconfigpath --version | cut -d\. -f1`
  MINOR=`$somaconfigpath --version | cut -d\. -f2`

  if test "$MAJOR" = "2" &&
     test "$MINOR" -le "4"; then
    CFLAGS="$CFLAGS `$somaconfigpath --cflags`"
    LIBS="$LIBS `$somaconfigpath --libs` -lsoma"
    AC_MSG_RESULT(yes)
 
  else
    AC_MSG_ERROR(Soma must be 2.x release!);
  fi
else
  AC_MSG_ERROR(No soma-config founded!);
fi

LIBS="$LIBS"
CFLAGS="$CFLAGS $GRAPHIC_CFLAGS -DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"

AC_OUTPUT([
Makefile
src/Makefile
src/confuse/Makefile
src/ghthash/Makefile
src/filechooser/Makefile
src/palinsesto/Makefile
src/maker/Makefile
src/draw/Makefile
src/timer/Makefile
src/nextitem/Makefile
src/x/Makefile
src/editor/Makefile
src/plugins/Makefile
src/plugins/plugins/Makefile
src/modules/Makefile
src/config/Makefile
src/commons/Makefile
intl/Makefile
po/Makefile.in
icons/Makefile
somax.pc
])

