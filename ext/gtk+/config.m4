dnl $Id$
dnl config.m4 for gtk+ module

define(glib_required_version,  2.6.0)
define(gtk_required_version,   2.6.0)
define(pango_required_version, 1.8.0)
define(atk_required_version,   1.8.0)

dnl Check for glib
AM_PATH_GLIB_2_0(glib_required_version,,[AC_MSG_ERROR(PHP-GTK 2.x requires Gtk+ 2.x)],gobject)
PHP_EVAL_INCLINE($GLIB_CFLAGS)
PHP_EVAL_LIBLINE($GLIB_LIBS, PHP_GTK2_SHARED_LIBADD)

dnl Check for Gtk+
AM_PATH_GTK_2_0(gtk_required_version,,[AC_MSG_ERROR(PHP-GTK 2.x requires Gtk+ 2.x)])
PHP_EVAL_INCLINE($GTK_CFLAGS)
PHP_EVAL_LIBLINE($GTK_LIBS, PHP_GTK2_SHARED_LIBADD)

dnl Check for atk
PKG_CHECK_MODULES(ATK, atk >= atk_required_version,,[AC_MSG_ERROR(PHP-GTK 2.x requires Gtk+ 2.x)])
PHP_EVAL_INCLINE($ATK_CFLAGS)
PHP_EVAL_LIBLINE($ATK_LIBS, PHP_GTK2_SHARED_LIBADD)

dnl Check for pango
PKG_CHECK_MODULES(PANGO, pango >= pango_required_version,,[AC_MSG_ERROR(PHP-GTK 2.x requires Gtk+ 2.x)])
PHP_EVAL_INCLINE($PANGO_CFLAGS)
PHP_EVAL_LIBLINE($PANGO_LIBS, PHP_GTK2_SHARED_LIBADD)

PHP_SUBST(PHP_GTK2_SHARED_LIBADD)
