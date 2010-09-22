dnl $Id$
dnl config.m4 for extension php-gtk

PHP_ARG_ENABLE(php-gtk,for PHP-GTK support,
[
                             PHP-GTK Options
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  --enable-php-gtk        Enable PHP-GTK support])

if test "$PHP_PHP_GTK" != "no"; then
  AC_MSG_CHECKING(for PHP executable)
  PHP_PREFIX=`$PHP_CONFIG --prefix 2>/dev/null`
  PHP=`$PHP_CONFIG --php-binary 2>/dev/null`
  if test -x "$PHP" -o -x "$PHP_PREFIX/bin/php"; then 
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
    AC_MSG_RESULT(found version $PHP_VERSION)

    case $PHP_VERSION in
      4*|5.0*|5.1.0)
        AC_MSG_ERROR([
Could not locate PHP 5.1.1 or higher version executable.
Please use the --with-php-config option to specify
the location of php-config for the required version.])
        ;;
    esac
	if test ! -x "$PHP"; then
		PHP=$PHP_PREFIX/bin/php
	fi
  else
    AC_MSG_ERROR(Could not locate PHP executable)
  fi

  PHP_SAPI=`$PHP --version | head -1 | sed -n 's/.*(\(...\)).*/\1/p'`
  if test "$PHP_SAPI" != "cli" ; then
    AC_MSG_ERROR(PHP CLI version is required[,] $PHP_SAPI found)
  fi

  AC_PROG_AWK
  PHP_SUBST(AWK)

  PHP_SUBST(PHP)

  BSD_MAKEFILE=no
  case $host_alias in
    *bsdi*)
    BSD_MAKEFILE=yes;;
  esac

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

dnl reading config stubs
esyscmd(./build2/config-stubs ext)

PHP_SUBST(PHP_GTK_EXTENSIONS)
PHP_SUBST(PHP_GTK_GEN_SOURCES)

PHP_NEW_EXTENSION(php_gtk2, main/php_gtk.c main/phpg_support.c main/phpg_gtype.c \
                            main/phpg_exceptions.c main/php_gtk_util.c main/phpg_gvalue.c \
                            main/phpg_closure.c main/phpg_gboxed.c main/phpg_gpointer.c \
                            main/phpg_atom.c main/phpg_paramspec.c main/phpg_gobject.c,
                            $ext_shared,, -I@ext_srcdir@/main)

PHP_MODULES="$PHP_MODULES $PHP_GTK_MODULES"

EXT_PHPGTK_HEADERS="main/php_gtk_api.h"
ifdef([PHP_INSTALL_HEADERS], [
	PHP_INSTALL_HEADERS(ext/php_gtk2, $EXT_PHPGTK_HEADERS)
])

PHP_ADD_SOURCES_X(/main, php_gtk_ext.c,, shared_objects_php_gtk2)
PHP_ADD_MAKEFILE_FRAGMENT($abs_srcdir/main/Makefile.frag, $abs_srcdir/main, main)

PHP_HELP_SEPARATOR([Libtool options:])

echo "creating main/php_gtk_ext.c"
sh $srcdir/build2/genext.sh $srcdir/main/php_gtk_ext.c.in $srcdir "" $AWK $PHP_GTK_EXTENSIONS > $srcdir/main/php_gtk_ext.c
