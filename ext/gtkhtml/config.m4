dnl $Id$
dnl config.m4 for gtkhtml module

dnl PHP_PATH_GNOME_CONFIG([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]])
dnl Test to see if gtkhtml is installed, and define GTKHTML_CFLAGS, LIBS
dnl
AC_DEFUN(PHP_PATH_GNOME_CONFIG,
[dnl
dnl Get the cflags and libraries from the gnome-config script
dnl
AC_ARG_WITH(gnome-config,
[  --with-gnome-config=GNOME_CONFIG  Location of gnome-config],
GNOME_CONFIG="$withval")

module_args=gtkhtml
 
AC_PATH_PROG(GNOME_CONFIG, gnome-config, no)
AC_MSG_CHECKING(for gnome-config)
if test "$GNOME_CONFIG " = "no"; then
  AC_MSG_RESULT(no)
  ifelse([$2], , :, [$2])
else
  HTML_CFLAGS=`$GNOME_CONFIG  --cflags $module_args`
  HTML_LIBS=`$GNOME_CONFIG  --libs $module_args`
  
  HTML_HAVE_GCONFTEST=`$GNOME_CONFIG  --cflags $module_args | grep GTKHTML_HAVE_GCONF`
  AC_MSG_RESULT(yes)
  ifelse([$1], , :, [$1])
fi
AC_SUBST(HTML_CFLAGS)
AC_SUBST(HTML_LIBS)
AC_SUBST(HTML_HAVE_GCONFTEST)
])

 
PHP_GTK_ARG_ENABLE(gtkhtml,for gtkhtml support,
[  --enable-gtkhtml        Enable gtkhtml support])

if test "$PHP_GTK_HTML" != "no"; then
  PHP_PATH_GNOME_CONFIG(have_gnomeconfig=yes,have_gnomeconfig=no)
  if test "$have_gnomeconfig" != "yes"; then
    AC_MSG_ERROR(Unable to locate gnomeconfig)
  else
    AC_DEFINE(HAVE_HTML,1,[gtkhtml support])
    PHP_EVAL_INCLINE($HTML_CFLAGS)
    if test "$HTML_HAVE_GCONFTEST no" != " no"; then
      AC_DEFINE(GTKHTML_HAVE_GCONF,1,[  ]) 
    fi   
    PHP_EVAL_LIBLINE($HTML_LIBS, HTML_SHARED_LIBADD)
    PHP_SUBST(HTML_SHARED_LIBADD)

    PHP_GTK_EXTENSION(gtkhtml, $php_gtk_ext_shared)
  fi  
fi
