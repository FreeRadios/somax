noinst_LTLIBRARIES = libpalinsesto.la

EXTRA_DIST = palinsesto.h

libpalinsesto_la_SOURCES = \
	palinsesto.c \
	spot.c \
	timer.c

if ENABLE_CONFUSE
confuse = ../confuse/libconfuse.la
endif

libpalinsesto_la_LIBADD = $(confuse)

