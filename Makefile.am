ACLOCAL_AMFLAGS = -I m4

SUBDIRS = firmware udev

lib_LTLIBRARIES = libqhyccd.la

libqhyccd_la_CFLAGS = $(VISIBILITY_CFLAGS) $(AM_CFLAGS)
libqhyccd_la_SOURCES = src/qhyccd.h src/qhyccd.cpp src/common.h src/common.cpp src/qhy5lii.cpp src/qhy5lii.h src/qhy5ii.cpp src/qhy5ii.h src/qhy9.cpp src/qhy9.h src/qhycam.cpp src/qhycam.h src/ic8300.cpp src/ic8300.h src/qhy22.cpp src/qhy22.h src/interguider.cpp src/interguider.h src/cssl.cpp src/cssl.h src/qhy6.cpp src/qhy6.h src/qhy21.cpp src/qhy21.h src/qhy23.cpp src/qhy23.h src/qhy11.cpp src/qhy11.h src/qhy16000.cpp src/qhy16000.h

pkginclude_HEADERS = src/qhyccd.h src/common.h src/interguider.h

