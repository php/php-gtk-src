dnl $Id$
dnl config.m4 for spaned module

PHP_GTK_ARG_ENABLE(spaned,for GtkSPaned support,
[
  --disable-spaned        Disable GtkSPaned support], yes)

if test "$PHP_GTK_SPANED" != "no"; then

  AC_DEFINE(HAVE_SPANED,1,[spaned support])
  PHP_GTK_EXTENSION(spaned, $php_gtk_ext_shared, php_spaned.c gtkspaned.c gtksvpaned.c, gen_spaned.c)
fi
