#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tintin], [1.99.6], [glenn@glennklockwood.com])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_LANG([C])
AC_HEADER_STDC

# Checks for libraries.
AX_CHECK_ZLIB

# Checks for header files.
AC_CHECK_HEADERS([sys/param.h arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/ioctl.h sys/socket.h sys/time.h termios.h unistd.h pty.h util.h])

AC_CHECK_HEADER([pcre.h], [], AC_MSG_FAILURE([pcre.h not found]))

AC_CHECK_LIB([util], [forkpty])
AC_CHECK_LIB([pcre], [pcre_compile], [], [AC_MSG_FAILURE([libpcre not found])])
AC_CHECK_LIB([z], [pcre_compile], [], [AC_MSG_FAILURE([libpcre not found])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 gethostbyname gethostname gettimeofday inet_ntoa memmove memset mkdir pow select socket sqrt strcasecmp strchr strdup strerror strncasecmp strndup strstr utime forkpty])

AC_OUTPUT(Makefile src/Makefile)
