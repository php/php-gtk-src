dnl $Id$
dnl config.m4 for extension php-gtk

PHP_ARG_ENABLE(php-gtk,for PHP-GTK support,
[  --enable-php-gtk        Enable PHP-GTK support])

if test "$PHP_PHP_GTK" != "no"; then
  AC_PROG_AWK
  PHP_SUBST(AWK)

  sinclude(php_gtk.m4)

  BSD_MAKEFILE=no
  case $host_alias in
	*bsdi*)
		BSD_MAKEFILE=yes;;
  esac

  # reading config stubs
  esyscmd(./build2/config-stubs ext)

  INCLUDES="$INCLUDES -I\$(top_srcdir)/main"

  PHP_EXTENSION(php-gtk, $ext_shared)
  AC_DEFINE(HAVE_PHP_GTK, 1, [If PHP-GTK support is enabled])
  PHP_FAST_OUTPUT(main/Makefile ext/Makefile)
fi
