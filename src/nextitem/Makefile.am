noinst_LTLIBRARIES = libnextitem.la

EXTRA_DIST = nextitem.h

libnextitem_la_SOURCES = nextitem.c

bin_PROGRAMS = somax-nextitem

localedir = $(datadir)/locale
INCLUDES = -DLOCALEDIR=\"$(localedir)\"  -DPATH_ICON=\"$(datadir)/@PACKAGE@/somax-nextitem.png\" -DEDITOR=\"$(bindir)/somax-editor\" -DX=\"$(bindir)/somax\" -DCONFIG=\"$(bindir)/somax-config\"

somax_nextitem_SOURCES = \
	main.c \
	nextitem.c
somax_nextitem_CFLAGS = $(AM_CFLAGS)

if ENABLE_CONFUSE
confuse = ../confuse/libconfuse.la
endif

somax_nextitem_LDADD = \
	../commons/libcommons.la \
	../filechooser/libfilechooser.la \
	$(confuse) \
	../ghthash/libghthash.la

