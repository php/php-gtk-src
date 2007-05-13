dnl $Id$
dnl config.m4 for scintilla module

define(scintilla_required_version,   0.1.0)

PHP_GTK_ARG_ENABLE(scintilla,for scintilla support,
[  --enable-scintilla      Enable Scintilla support],no)

if test "$PHP_GTK_SCINTILLA" != "no"; then
  PKG_CHECK_MODULES(SCINTILLA, [GtkScintilla-2.0 >= scintilla_required_version],
  have_scintilla=yes, have_scintilla=no)
  if test "$have_scintilla" != "yes"; then
    AC_MSG_RESULT([Unable to locate scintilla version scintilla_required_version or higher: not building])
  else
    AC_DEFINE(HAVE_SCINTILLA, 1, [scintilla support])
    PHP_EVAL_INCLINE($SCINTILLA_CFLAGS)
    if test "$php_gtk_ext_shared" = "yes"; then
      PHP_EVAL_LIBLINE($SCINTILLA_LIBS, SCINTILLA_SHARED_LIBADD)
      PHP_SUBST(SCINTILLA_SHARED_LIBADD)
    else
      PHP_EVAL_LIBLINE($SCINTILLA_LIBS, PHP_GTK2_SHARED_LIBADD)
    fi
    PHP_GTK_EXTENSION(scintilla, $php_gtk_ext_shared, php_scintilla.c, gen_scintilla.c)
  fi
fi
