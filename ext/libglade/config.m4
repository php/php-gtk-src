dnl $Id$
dnl config.m4 for libglade module

define(libglade_required_version,   2.4.0)

PHP_GTK_ARG_WITH(libglade,for libglade support,
[  --without-libglade      Disable libglade support],yes)

if test "$PHP_GTK_LIBGLADE" != "no"; then
  PKG_CHECK_MODULES(LIBGLADE, [libglade-2.0 >= libglade_required_version], have_libglade=yes, have_libglade=no)
  if test "$have_libglade" != "yes"; then
    AC_MSG_RESULT([Unable to locate libglade version libglade_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_LIBGLADE, 1, [libglade support])
    PHP_EVAL_INCLINE($LIBGLADE_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($LIBGLADE_LIBS, LIBGLADE_SHARED_LIBADD)
      PHP_SUBST(LIBGLADE_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($LIBGLADE_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(libglade, $php_gtk_ext_shared, php_libglade.c, gen_libglade.c)
  fi
fi
