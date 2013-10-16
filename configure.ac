#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([libqhyccd], [0.1.0], [yangxudong.cn@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/qhyccd.cpp], [src/qhyccd.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for fxload, which is used to load the firmware into the ram
# of the CCD
AC_CHECK_FILE([/sbin/fxload], [],
        AC_MSG_ERROR([*** Unable to find fxload program
                (http://sourceforge.net/projects/linux-hotplug/files/fxload/2008_10_13/fxload-2008_10_13.tar.gz/download)]))

# Checks for libraries.
AC_CHECK_LIB([usb], [libusb_init])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC


AM_CFLAGS="-Wall -Wundef -Wunused -Wstrict-prototypes -Werror-implicit-function-declaration -Wshadow"

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_LDFLAGS)

AC_CONFIG_FILES([Makefile
                 firmware/Makefile
                 udev/Makefile])
AC_OUTPUT
