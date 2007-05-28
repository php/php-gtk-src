dnl $Id$
dnl config.m4 for gtkscintilla module

define(gtkscintilla_required_version,   0.1.0)

PHP_GTK_ARG_WITH(gtkscintilla,for gtkscintilla support,
[  --with-gtkscintilla     Enable GtkScintilla support],no)

if test "$PHP_GTK_GTKSCINTILLA" != "no"; then
  PKG_CHECK_MODULES(GTKSCINTILLA, [GtkScintilla-2.0 >= gtkscintilla_required_version],
  have_gtkscintilla=yes, have_gtkscintilla=no)
  if test "$have_gtkscintilla" != "yes"; then
    AC_MSG_RESULT([Unable to locate gtkscintilla version gtkscintilla_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_GTKSCINTILLA, 1, [gtkscintilla support])
    PHP_EVAL_INCLINE($GTKSCINTILLA_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($GTKSCINTILLA_LIBS, GTKSCINTILLA_SHARED_LIBADD)
      PHP_SUBST(GTKSCINTILLA_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($GTKSCINTILLA_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(gtkscintilla, $php_gtk_ext_shared, php_gtkscintilla.c, gen_gtkscintilla.c)
  fi
fi
