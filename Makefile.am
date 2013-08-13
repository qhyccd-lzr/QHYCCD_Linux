ACLOCAL_AMFLAGS = -I m4

SUBDIRS = firmware udev

lib_LTLIBRARIES = libqhyccd.la

libqhyccd_la_CFLAGS = $(VISIBILITY_CFLAGS) $(AM_CFLAGS)
libqhyccd_la_SOURCES = src/qhyccd.h src/qhyccd.c src/common.h src/common.c src/qhy5lii.c src/qhy5lii.h src/qhy5ii.c src/qhy5ii.h src/qhy9.c src/qhy9.h src/QHYCAM.c src/QHYCAM.h src/qhy9l.c src/qhy9l.h src/qhy22.c src/qhy22.h

#pkginclude_DATA = src/qhyccd.h src/common.h

