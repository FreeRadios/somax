EXTRA_DIST = \
	print.c \
	editor.c \
	plugin.h \
	parser.c \
	plugin.c \
	plugin_internal.h \
	parser.h

if ENABLE_PRINT
print = somax_print_plugin
endif

if ENABLE_GTKSOURCEVIEW
pls_PROGRAMS = $(print) somax_editor_plugin
plsdir = $(datadir)/@PACKAGE@/plugins
endif

if ENABLE_CONFUSE
confuse = ../../confuse/libconfuse.la
endif

somax_print_plugin_SOURCES = print.c plugin.c
somax_print_plugin_LDADD = \
	../../palinsesto/libpalinsesto.la \
        $(confuse) \
	../../commons/libcommons.la
somax_print_plugin_CFLAGS = $(AM_CFLAGS)

somax_editor_plugin_SOURCES = editor.c plugin.c
somax_editor_plugin_LDADD = \
	../../palinsesto/libpalinsesto.la \
        $(confuse) \
	../../commons/libcommons.la
somax_editor_plugin_CFLAGS = $(AM_CFLAGS)


lib_LTLIBRARIES = libsomax_plgn.la libsomax.la

libsomax_plgn_la_SOURCES = plugin.c
libsomax_plgn_la_LDFLAGS = -version-info 1:0:0
libsomax_plgn_la_LIBADD = \
	../../palinsesto/libpalinsesto.la

libsomax_la_SOURCES = parser.c
libsomax_la_LDFLAGS = -version-info 1:0:0
libsomax_la_LIBADD = \
	../../palinsesto/libpalinsesto.la

inc_DATA = plugin.h parser.h
incdir = $(includedir)/@PACKAGE@
