dnl $Id$
dnl config.m4 for extension php-gtk

PHP_ARG_ENABLE(php-gtk,for PHP-GTK support,
[
                             PHP-GTK Options
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  --enable-php-gtk        Enable PHP-GTK support])

if test "$PHP_PHP_GTK" != "no"; then

  PHP_PREFIX=`php-config --prefix`
  AC_MSG_CHECKING(for PHP executable in $PHP_PREFIX/bin)
  if test -x $PHP_PREFIX/bin/php; then
    PHP_VERSION=`$PHP_CONFIG --version`
    case $PHP_VERSION in
       4*) AC_MSG_ERROR(
Could not locate PHP 5 or higher version executable.
Please use the --with-php-config option to specify
the location of php-config for the required version.) ;;
    esac
    PHP=$PHP_PREFIX/bin/php
    AC_MSG_RESULT(found)
  else
    AC_MSG_ERROR(Could not locate PHP executable)
  fi

  AC_PROG_AWK
  PHP_SUBST(AWK)

  PHP_SUBST(PHP)

  BSD_MAKEFILE=no
  case $host_alias in
	*bsdi*)
		BSD_MAKEFILE=yes;;
  esac

  sinclude(php_gtk.m4)

  AC_DEFINE(HAVE_PHP_GTK, 1, [If PHP-GTK support is enabled])
fi

PHP_GTK_ARG_ENABLE(debug, whether to include debugging symbols,
[  --enable-debug          Compile with debugging symbols.], no)
if test "$PHP_GTK_DEBUG" = "yes"; then
  echo " $CFLAGS" | grep ' -g' >/dev/null || DEBUG_CFLAGS="-g"
  if test "$CFLAGS" = "-g -O2"; then
  	CFLAGS=-g
  fi
  test -n "$GCC" && DEBUG_CFLAGS="$DEBUG_CFLAGS -Wall"
fi

test -n "$DEBUG_CFLAGS" && CFLAGS="$CFLAGS $DEBUG_CFLAGS"

CFLAGS_CLEAN=$CFLAGS
PHP_SUBST(CFLAGS_CLEAN)
