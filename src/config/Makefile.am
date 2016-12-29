bin_PROGRAMS = somax-config

localedir = $(datadir)/locale
INCLUDES = -DLOCALEDIR=\"$(localedir)\" -DPATH_ICON=\"$(datadir)/@PACKAGE@/somax.png\" -DEDITOR=\"$(bindir)/somax-editor\" -DX=\"$(bindir)/somax\" -DNEXTITEM=\"$(bindir)/somax-nextitem\"

EXTRA_DIST = cfg.h comments.h

somax_config_SOURCES = \
	main.c \
	file.c \
	exec.c \
	host.c

if ENABLE_CONFUSE
confuse = ../confuse/libconfuse.la
endif

somax_config_LDADD = \
	../commons/libcommons.la \
	$(confuse) \
	../filechooser/libfilechooser.la

