dnl $Id$
dnl config.m4 for combobutton module

PHP_GTK_ARG_ENABLE(combobutton,for GtkComboButton support,
[
  --disable-combobutton   Disable GtkComboButton support
],yes)

if test "$PHP_GTK_COMBOBUTTON" != "no"; then

  AC_DEFINE(HAVE_COMBOBUTTON,1,[combobutton support])
  PHP_GTK_EXTENSION(combobutton, $php_gtk_ext_shared)
fi
