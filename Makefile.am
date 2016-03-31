ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = eds-to-idevice
noinst_LTLIBRARIES = lib/libeti.la

eds_to_idevice_CPPFLAGS = -I$(top_srcdir)/lib 
eds_to_idevice_CFLAGS = $(GLIB2_CFLAGS) $(EDS_CFLAGS) $(WARN_CFLAGS) $(GTK3_CFLAGS)
eds_to_idevice_LDADD = $(top_builddir)/lib/libeti.la $(GLIB2_LIBS) $(EDS_LIBS) $(GTK3_LIBS)
eds_to_idevice_SOURCES = src/econtact.c src/eti-eds.c src/main.c

lib_libeti_la_CFLAGS = $(LIBIMOBILEDEVICE_CFLAGS) $(LIBPLIST_CFLAGS) $(WARN_CFLAGS) $(GLIB2_CFLAGS) 
lib_libeti_la_LIBADD = $(LIBIMOBILEDEVICE_LIBS) $(LIBPLIST_LIBS)
lib_libeti_la_SOURCES = lib/eti-contact.c \
                    lib/eti-contact-plist-builder.c \
                    lib/eti-contact-plist-parser.c \
                    lib/eti-plist.c \
                    lib/eti-sync.c

noinst_HEADERS = lib/eti-contact.h \
                 lib/eti-contact-plist-builder.h \
                 lib/eti-contact-plist-parser.h \
                 lib/eti-plist.h \
                 lib/eti-sync.h \
                 src/eti-eds.h

