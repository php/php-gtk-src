dnl $Id$
dnl config.m4 for mozembed module

define(mozembed_required_version,   1.7.0)

PHP_GTK_ARG_ENABLE(mozembed,for GtkMozEmbed support,
[  --enable-mozembed      Enable GtkMozEmbed support],no)

if test "$PHP_GTK_MOZEMBED" != "no"; then
  PKG_CHECK_MODULES(MOZEMBED, [mozilla-gtkmozembed >= mozembed_required_version],
  have_mozembed=yes, have_mozembed=no)
  if test "$have_mozembed" != "yes"; then
    AC_MSG_RESULT([Unable to locate mozembed version mozembed_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_MOZEMBED, 1, [mozembed support])
    PHP_EVAL_INCLINE($MOZEMBED_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($MOZEMBED_LIBS, MOZEMBED_SHARED_LIBADD)
      PHP_SUBST(MOZEMBED_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($MOZEMBED_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(mozembed, $php_gtk_ext_shared, php_mozembed.c, gen_mozembed.c)
  fi
fi
