#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(cocodis, 0.11.1)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([g09dis/dasmedit.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h libgen.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STAT

AC_CHECK_FUNCS([memset strcasecmp strchr strdup stpcpy strncasecmp strstr isblank dirname])

AC_ARG_WITH([win09dis],
               [AS_HELP_STRING([--with-win09dis],
	                       [include win09dis (default=no)])],
	       [case "${withval}" in
	         yes) compile_win09dis=true;;
		 no)  compile_win09dis=false;;
		 *) AC_MSG_ERROR(bad value ${withval} for --with-win09dis) ;;
	       [esac]],[compile_win09dis=false])
AM_CONDITIONAL(WITHWIN09DIS, test x$compile_win09dis = xtrue)

AC_SUBST(compile_win09dis)

# Establish pkg-config paths
AC_ARG_WITH([g09dis],
               [AS_HELP_STRING([--with-g09dis],
	                       [include g09dis (requires pkg-config) (default=no)])],
	       [case "${withval}" in
	         yes) compile_g09dis=true
	              PKG_CHECK_MODULES(pkgcfg, gtk+-2.0);;
		 no)  compile_g09dis=false;;
		 *) AC_MSG_ERROR(bad value ${withval} for --with-g09dis) ;;
	       [esac]],[compile_g09dis=false])
AM_CONDITIONAL(WITHG09DIS, test x$compile_g09dis = xtrue)

AC_SUBST(compile_g09dis)

AC_CANONICAL_HOST
case $host_os in
    *mingw32*)
        winflags=" -mms-bitfields -mwindows -mno-cygwin"
        gwinflags=" -mms-bitfields -mno-cygwin"
    ;;
    *cygwin)
        winflags="-mwindows"
    ;;
esac

AC_SUBST(winflags)
AC_SUBST(gwinflags)

AC_CONFIG_FILES([Makefile
                 g09dis/Makefile
		 win09dis/Makefile
                 os9disasm/Makefile])
AC_OUTPUT
