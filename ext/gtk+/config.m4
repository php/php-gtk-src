dnl $Id$
dnl config.m4 for gtk+ module

define(glib_required_version,  2.6.0)
define(gtk_required_version,   2.6.0)
define(pango_required_version, 1.8.0)
define(atk_required_version,   1.9.0)
define(cairo_required_version, 1.4.0)

dnl Check for glib
AM_PATH_GLIB_2_0(glib_required_version,,[AC_MSG_ERROR(PHP-GTK 2.x requires GLib glib_required_version or higher)],gobject)
PHP_EVAL_INCLINE($GLIB_CFLAGS)
PHP_EVAL_LIBLINE($GLIB_LIBS, PHP_GTK2_SHARED_LIBADD)

dnl Check for Gtk+
AM_PATH_GTK_2_0(gtk_required_version,GTK_LIBVERSION=$GTK_VERSION,[AC_MSG_ERROR(PHP-GTK 2.x requires GTK+ gtk_required_version or higher)])
PHP_EVAL_INCLINE($GTK_CFLAGS)
PHP_EVAL_LIBLINE($GTK_LIBS, PHP_GTK2_SHARED_LIBADD)
PHP_SUBST(GTK_LIBVERSION)

dnl Check for atk
PKG_CHECK_MODULES(ATK,[atk >= atk_required_version],,[AC_MSG_ERROR(PHP-GTK 2.x requires ATK atk_required_version or higher)])
PHP_EVAL_INCLINE($ATK_CFLAGS)
PHP_EVAL_LIBLINE($ATK_LIBS, PHP_GTK2_SHARED_LIBADD)

dnl Check for pango
PKG_CHECK_MODULES(PANGO,[pango >= pango_required_version],,[AC_MSG_ERROR(PHP-GTK 2.x requires Pango pango_required_version or higher)])
PHP_EVAL_INCLINE($PANGO_CFLAGS)
PHP_EVAL_LIBLINE($PANGO_LIBS, PHP_GTK2_SHARED_LIBADD)

dnl Check for libcairo and cairo extension
if test $gtk_config_major_version -ge 2 -a $gtk_config_minor_version -ge 8; then
	PKG_CHECK_MODULES(CAIRO, [cairo >= cairo_required_version],[has_cairo=1],[has_cairo=0])
	if test $has_cairo -eq 1; then
		PHP_EVAL_INCLINE($CAIRO_CFLAGS)
		PHP_EVAL_LIBLINE($CAIRO_LIBS, PHP_GTK2_SHARED_LIBADD)
	else
		AC_MSG_ERROR(cairo library not found.)
	fi

dnl AC_CHECK_HEADER ignored my CPPFLAGS/CFLAGS/CXXFLAGS/AM_CFLAGS/AM_CPPFLAGS/
dnl LDFLAGS/AM_LDFLAGS settings so i ignored it.
	AC_MSG_CHECKING(for cairo php extension)
	if test -f "$phpincludedir/ext/cairo/php_cairo_api.h"; then
		PHP_ADD_INCLUDE($phpincludedir/ext/cairo)
		PHP_DEF_HAVE(CAIRO)
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		AC_MSG_ERROR(cairo php extension not found.)
	fi
fi

PHP_SUBST(PHP_GTK2_SHARED_LIBADD)

PHP_GTK_EXTENSION([gtk+], no, [php_gtk+.c], [gen_atk.c gen_pango.c gen_gdk.c gen_gtk.c gen_gtk-1.c php_gdk.c php_gtk+_types.c phpg_custom_tree_model.c])
