dnl $Id$
dnl config.m4 for sqpane module

PHP_GTK_ARG_ENABLE(sqpane,for SQPane support,
[  --disable-sqpane        Disable SQPane support],yes)

if test "$PHP_GTK_SQPANE" != "no"; then

  AC_DEFINE(HAVE_SQPANE,1,[sqpane support])
  PHP_GTK_EXTENSION(sqpane)
fi
