dnl $Id$
dnl config.m4 for imlib module

PHP_GTK_ARG_ENABLE(imlib,for Gdkimlib support,
[  --enable-imlib          Enable Gdkimlib support])

if test "$PHP_GTK_IMLIB" != "no"; then
  for x in /usr /usr/local; do
    if test -f $x/include/gdk_imlib.h; then
      IMLIB_DIR=$x
      IMLIB_INCDIR=$x/include
    fi
  done

  if test -z "$IMLIB_DIR"; then
    AC_MSG_ERROR(Cannot locate imlib)
  fi

  IMLIB_LIBDIR=$IMLIB_DIR/lib

  AC_DEFINE(HAVE_IMLIB,1,[imlib support])
  PHP_ADD_INCLUDE($IMLIB_INCDIR)
  if test "$php_gtk_ext_shared" = "yes"; then
    PHP_ADD_LIBRARY_WITH_PATH(gdk_imlib, $IMLIB_LIBDIR, IMLIB_SHARED_LIBADD)
    PHP_SUBST(IMLIB_SHARED_LIBADD)
  else
    PHP_ADD_LIBRARY_WITH_PATH(gdk_imlib, $IMLIB_LIBDIR, PHP_GTK_SHARED_LIBADD)
  fi
  PHP_GTK_EXTENSION(imlib, $php_gtk_ext_shared,php_imlib.c, gen_imlib.c)
fi
 
