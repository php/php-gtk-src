dnl $Id$
dnl config.m4 for mozembed module

define(firefox_required_version, 1.5.0)
define(mozilla_required_version, 1.7.0)
define(xulrunner_required_version, 1.7.0) #Just a wild guess

PHP_GTK_ARG_WITH(mozembed,for GtkMozEmbed support,
[  --with-mozembed         Enable GtkMozEmbed support],no)

dnl first check for firefox, then mozilla
if test "$PHP_GTK_MOZEMBED" != "no"; then
  PKG_CHECK_MODULES(MOZEMBED, [firefox-gtkmozembed >= firefox_required_version],
  have_mozembed=yes, have_mozembed=no)
  if test "$have_mozembed" != "yes"; then
    PKG_CHECK_MODULES(MOZEMBED, [mozilla-gtkmozembed >= mozilla_required_version],
    have_mozembed=yes, have_mozembed=no)
  fi
  if test "$have_mozembed" != "yes"; then
    PKG_CHECK_MODULES(MOZEMBED, [xulrunner-gtkmozembed >= xulrunner_required_version],
    have_mozembed=yes, have_mozembed=no)
  fi
  if test "$have_mozembed" != "yes"; then
    AC_MSG_RESULT([Unable to locate firefox-gtkmozembed version firefox_required_version, mozilla-gtkmozembed version mozilla_required_version, xulrunner-gtkmozembed xulrunner_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_MOZEMBED, 1, [mozembed support])
    PHP_EVAL_INCLINE($MOZEMBED_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($MOZEMBED_LIBS, PHP_GTK2_MOZEMBED_SHARED_LIBADD)
      PHP_SUBST(PHP_GTK2_MOZEMBED_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($MOZEMBED_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(mozembed, $php_gtk_ext_shared, php_mozembed.c, gen_mozembed.c)
  fi
fi
