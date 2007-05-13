dnl $Id$
dnl config.m4 for sourceview module

define(sourceview_required_version,   1.2.0)

PHP_GTK_ARG_ENABLE(sourceview,for sourceview support,
[  --enable-sourceview     Enable SourceView support],no)

if test "$PHP_GTK_SOURCEVIEW" != "no"; then
  PKG_CHECK_MODULES(SOURCEVIEW, [gtksourceview-1.0 >= sourceview_required_version],
  have_sourceview=yes, have_sourceview=no)
  if test "$have_sourceview" != "yes"; then
    AC_MSG_RESULT([Unable to locate sourceview version sourceview_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_SOURCEVIEW, 1, [sourceview support])
    PHP_EVAL_INCLINE($SOURCEVIEW_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($SOURCEVIEW_LIBS, SOURCEVIEW_SHARED_LIBADD)
      PHP_SUBST(SOURCEVIEW_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($SOURCEVIEW_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(sourceview, $php_gtk_ext_shared, php_sourceview.c, gen_sourceview.c)
  fi
fi
