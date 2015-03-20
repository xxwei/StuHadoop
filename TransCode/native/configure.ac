#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(TransCode,1.0.1,xiaowei.chai@allegion.com)
AC_CONFIG_SRCDIR([main.cpp])
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE(TransCode,1.0.1,xiaowei.chai@allegion.com)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
#mutilthread check
#LT_INIT
AC_PROG_RANLIB
# dynmic lib
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([avformat],[av_register_all])
AC_CHECK_LIB([x264],[x264_nal_encode])
# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T


AC_CONFIG_MACRO_DIR([m4])

# Checks for library functions.

AC_CONFIG_FILES([makefile])
AC_OUTPUT
