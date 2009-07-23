dnl
dnl $Id$
dnl

AC_MSG_CHECKING(for pconn SAPI)

PHP_ARG_ENABLE(pconn,,
[  --enable-pconn          Build PHP as pconn test module (for extension developers], no)

if test "$PHP_PCON" != "no"; then
  PHP_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/pconn/Makefile.frag,$abs_srcdir/pconn/pconn,sapi/pconn)
  SAPI_PCONN_PATH=sapi/pconn/pconn
  PHP_SUBST(SAPI_PCONN_PATH)
  PHP_SELECT_SAPI(pconn, program, main.c  pconnect-module.c  pconnect-sapi.c, "", '$(SAPI_PCONN_PATH)') 
  case $host_alias in
  *darwin*)
    BUILD_PCONN="\$(CC) \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(NATIVE_RPATHS) \$(PHP_GLOBAL_OBJS:.lo=.o) \$(PHP_SAPI_OBJS:.lo=.o) \$(PHP_FRAMEWORKS) \$(EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_PCONN_PATH)"
    ;;
  *cygwin*)
    SAPI_PCONN_PATH=sapi/pconn/pconn.exe
    BUILD_PCONN="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(PHP_RPATHS) \$(PHP_GLOBAL_OBJS) \$(PHP_SAPI_OBJS) \$(EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_PCONN_PATH)"
    ;;
  *)
    BUILD_PCONN="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(PHP_RPATHS) \$(PHP_GLOBAL_OBJS) \$(PHP_SAPI_OBJS) \$(EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_PCONN_PATH)"
    ;;
  esac

  PHP_SUBST(BUILD_PCONN)
fi

AC_MSG_RESULT($PHP_PCONN)
