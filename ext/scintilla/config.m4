dnl $Id$
dnl config.m4 for scintilla module

PHP_GTK_ARG_ENABLE(scintilla,for Scintilla support,
[
  --enable-scintilla      Enable Scintilla support])

if test "$PHP_GTK_SCINTILLA" != "no"; then
  for x in /usr /usr/local; do
	  if test -f $x/include/gtkscintilla.h; then
		  SCINTILLA_DIR=$x
		  SCINTILLA_INCDIR=$x/include
	  fi
  done

  if test -z "$SCINTILLA_DIR"; then
	  AC_MSG_ERROR(Cannot locate Scintilla)
  fi

  SCINTILLA_LIBDIR=$SCINTILLA_DIR/lib

  AC_DEFINE(HAVE_SCINTILLA,1,[scintilla support])
  PHP_ADD_INCLUDE($SCINTILLA_INCDIR)
  if test $php_gtk_ext_shared = "yes"; then
	PHP_ADD_LIBRARY_WITH_PATH(gtkscintilla, $SCINTILLA_LIBDIR, SCINTILLA_SHARED_LIBADD)
	PHP_SUBST(SCINTILLA_SHARED_LIBADD)
  else
	PHP_ADD_LIBRARY_WITH_PATH(gtkscintilla, $SCINTILLA_LIBDIR, PHP_GTK_SHARED_LIBADD)
  fi
  PHP_GTK_EXTENSION(scintilla, $php_gtk_ext_shared, php_scintilla.c, gen_scintilla.c)
fi
