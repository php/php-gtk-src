dnl $Id$
dnl config.m4 for canvas module

PHP_GTK_ARG_ENABLE(canvas,for GtkCanvas support,
[
  --enable-canvas      Enable GtkCanvas support])

if test "$PHP_GTK_CANVAS" != "no"; then
  for x in /usr /usr/local; do
	  if test -f $x/include/gtk-canvas.h; then
		  CANVAS_DIR=$x
		  CANVAS_INCDIR=$x/include
	  fi
  done

  if test -z "$CANVAS_DIR"; then
	  AC_MSG_ERROR(Cannot locate GtkCanvas)
  fi

  CANVAS_LIBDIR=$CANVAS_DIR/lib

  AC_DEFINE(HAVE_CANVAS,1,[GtkCanvas support])
  PHP_ADD_INCLUDE($CANVAS_INCDIR)
  if test $php_gtk_ext_shared = "yes"; then
	PHP_ADD_LIBRARY_WITH_PATH(gtk-canvas, $CANVAS_LIBDIR, CANVAS_SHARED_LIBADD)
	PHP_SUBST(CANVAS_SHARED_LIBADD)
  else
	PHP_ADD_LIBRARY_WITH_PATH(gtk-canvas, $CANVAS_LIBDIR, PHP_GTK_SHARED_LIBADD)
  fi
  PHP_GTK_EXTENSION(canvas, $php_gtk_ext_shared, php_canvas.c, gen_canvas.c)
fi
