dnl $Id$
dnl config.m4 for libsexy module

define(libsexy_required_version,   0.1.10)

PHP_GTK_ARG_WITH(libsexy,for libsexy support,
[  --with-libsexy          Enable libsexy support],no)

if test "$PHP_GTK_LIBSEXY" != "no"; then
  PKG_CHECK_MODULES(LIBSEXY, [libsexy >= libsexy_required_version], have_libsexy=yes, have_libsexy=no)
  if test "$have_libsexy" != "yes"; then
    AC_MSG_RESULT([Unable to locate libsexy version libsexy_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_LIBSEXY, 1, [libsexy support])
    PHP_EVAL_INCLINE($LIBSEXY_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($LIBSEXY_LIBS, LIBSEXY_SHARED_LIBADD)
      PHP_SUBST(LIBSEXY_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($LIBSEXY_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(libsexy, $php_gtk_ext_shared, php_libsexy.c, gen_libsexy.c)
  fi
fi
