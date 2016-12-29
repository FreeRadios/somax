bin_PROGRAMS = somax-editor

localedir = $(datadir)/locale
INCLUDES = -DLOCALEDIR=\"$(localedir)\" -DPATH_ICON=\"$(datadir)/@PACKAGE@/somax-editor.png\" -DPATH_IMAGE=\"$(datadir)/@PACKAGE@/bootsplash.png\"

EXTRA_DIST = editor.h

somax_editor_SOURCES = \
	main.c \
	edit.c \
	file.c \
	maker.c \
	color.c \
	view.c \
	daemon.c

if ENABLE_CONFUSE
confuse = ../confuse/libconfuse.la
endif
	
somax_editor_LDADD = \
	../commons/libcommons.la \
	../plugins/libplugins.la \
	../modules/libmodules.la \
	../maker/libmaker.la \
	../draw/libdraw.la \
	../filechooser/libfilechooser.la \
	../palinsesto/libpalinsesto.la \
	$(confuse) \
	../ghthash/libghthash.la

