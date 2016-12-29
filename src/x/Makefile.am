bin_PROGRAMS = somax

localedir = $(datadir)/locale
INCLUDES = -DLOCALEDIR=\"$(localedir)\" -DPATH_ICON=\"$(datadir)/@PACKAGE@/somax.png\" -DEDITOR=\"$(bindir)/somax-editor\" -DCONFIG=\"$(bindir)/somax-config\" -DNEXTITEM=\"$(bindir)/somax-nextitem\"
EXTRA_DIST = somax.h

somax_SOURCES = \
	main.c \
	interface.c \
	timeout.c \
	buttons.c \
	time.c \
	list.c \
	tlist.c \
	tooltips.c \
	daemon.c \
	nextitem.c \
	change.c \
	maker.c \
	preferences.c

if ENABLE_CONFUSE
confuse = ../confuse/libconfuse.la
endif

somax_LDADD = \
	../commons/libcommons.la \
	../maker/libmaker.la \
	../draw/libdraw.la \
	../timer/libtimer.la \
	../plugins/libplugins.la \
	../modules/libmodules.la \
	../nextitem/libnextitem.la \
	../palinsesto/libpalinsesto.la \
	../filechooser/libfilechooser.la \
        $(confuse) \
	../ghthash/libghthash.la
