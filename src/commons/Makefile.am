noinst_LTLIBRARIES = libcommons.la

INCLUDES =  -DPATH_IMAGE=\"$(datadir)/@PACKAGE@/bootsplash.png\"

EXTRA_DIST = \
	commons.h \
	data.h \
	save.h \
	x.h \
	splash.h \
	stat.h \
	help.h \
	interface.h

libcommons_la_SOURCES = \
	commons.c \
	dialog.c \
	statusbar.c \
	save.c \
	x.c \
	splash.c \
	stat.c \
	help.c \
	interface.c 

inc_DATA = data.h
incdir = $(includedir)/@PACKAGE@
