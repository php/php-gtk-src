dnl $Id$
dnl config.m4 for scrollpane module

PHP_GTK_ARG_ENABLE(scrollpane,for GtkScrollpane support,
[
  --disable-scrollpane    Disable GtkScrollpane support],yes)

if test "$PHP_GTK_SCROLLPANE" != "no"; then

  AC_DEFINE(HAVE_SCROLLPANE,1,[scrollpane support])
  PHP_GTK_EXTENSION(scrollpane, $php_gtk_ext_shared, php_scrollpane.c gtkscrollpane.c, gen_scrollpane.c)
fi
