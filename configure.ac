#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(cocodis, 0.1)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([g09dis/dasmedit.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STAT

AC_CHECK_FUNCS([memset strcasecmp strchr strncasecmp strstr])

# Establish pkg-config paths
PKG_CHECK_MODULES(pkgcfg, gtk+-2.0)

AC_CANONICAL_HOST
case $host_os in
    *mingw32*)
        CFLAGS="$CFLAGS -mms-bitfields -mno-cygwin"
    ;;
    *cygwin)
        CFLAGS="$CFLAGS -mms-bitfields"
   ;;
esac

AC_CONFIG_FILES([Makefile
                 g09dis/Makefile
                 os9disasm/Makefile])
AC_OUTPUT
