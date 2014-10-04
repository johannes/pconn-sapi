dnl
dnl $Id$
dnl

AC_MSG_CHECKING(for pconn SAPI)

PHP_ARG_ENABLE(pconn,,
[  --enable-pconn          Build PHP as pconn test module (for extension developers], no)

if test "$PHP_PCONN" != "no"; then
  PHP_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/pconn/Makefile.frag)
  SAPI_PCONN_PATH=sapi/pconn/php-pconn
  PHP_SUBST(SAPI_PCONN_PATH)
  PHP_SELECT_SAPI(pconn, program, main.c  pconnect-module.c  pconnect-sapi.c pconnect-phptparser.c, "", '$(SAPI_PCONN_PATH)')
  if test $PHP_VERSION_ID -ge 50400; then
    BUILD_PCONN="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(PHP_RPATHS) \$(PHP_GLOBAL_OBJS) \$(PHP_BINARY_OBJS) \$(PHP_PCONN_OBJS) \$(EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_PCONN_PATH)"
  else
    BUILD_PCONN="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(PHP_RPATHS) \$(PHP_GLOBAL_OBJS) \$(PHP_SAPI_OBJS) \$(EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_PCONN_PATH)"
  fi

  PHP_SUBST(BUILD_PCONN)
fi

AC_MSG_RESULT($PHP_PCONN)
