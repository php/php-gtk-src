dnl $Id$
dnl config.m4 for extra module

PHP_GTK_ARG_ENABLE(extra,for Gtkextra support, [  --enable-extra       Enable Gtkextra support])
  for x in /usr /usr/local; do
	  if test -f $x/include/gtkextra/gtkextra.h; then
		  EXTRA_DIR=$x
		  EXTRA_INCDIR=$x/include
   	fi
  done
  
  if test -z "$EXTRA_DIR"; then
    AC_MSG_ERROR(Cannot locate extra)
  fi

  EXTRA_LIBDIR=$EXTRA_DIR/lib

  AC_DEFINE(HAVE_EXTRA,1,[extra support])
  PHP_ADD_INCLUDE($EXTRA_INCDIR)
  if test "$php_gtk_ext_shared" = "yes"; then
    PHP_ADD_LIBRARY_WITH_PATH(gtkextra, $EXTRA_LIBDIR, EXTRA_SHARED_LIBADD)
    PHP_SUBST(EXTRA_SHARED_LIBADD)
  else
    PHP_ADD_LIBRARY_WITH_PATH(gtkextra, $EXTRA_LIBDIR, PHP_GTK_SHARED_LIBADD)
  fi
  PHP_GTK_EXTENSION(extra, $php_gtk_ext_shared, php_extra.c, gen_extra.c)
