dnl $Id$
dnl config.m4 for libglade module

dnl PHP_PATH_LIBGLADE([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]])
dnl Test to see if libglade is installed, and define LIBGLADE_CFLAGS, LIBS
dnl
AC_DEFUN(PHP_PATH_LIBGLADE,
[dnl
dnl Get the cflags and libraries from the libglade-config script
dnl
AC_ARG_WITH(libglade-config,
[  --with-libglade-config=LIBGLADE_CONFIG  Location of libglade-config],
LIBGLADE_CONFIG="$withval")

module_args=
for module in . $3; do
  case "$module" in
    gnome)
      module_args="$module_args gnome"
      ;;
    bonobo)
      module_args="$module_args bonobo"
      ;;
  esac
done

AC_PATH_PROG(LIBGLADE_CONFIG, libglade-config, no)
AC_MSG_CHECKING(for libglade)
if test "$LIBGLADE_CONFIG" = "no"; then
  AC_MSG_RESULT(no)
  ifelse([$2], , :, [$2])
else
  if $LIBGLADE_CONFIG --check $module_args; then
    LIBGLADE_CFLAGS=`$LIBGLADE_CONFIG --cflags $module_args`
    LIBGLADE_LIBS=`$LIBGLADE_CONFIG --libs $module_args`
    AC_MSG_RESULT(yes)
    ifelse([$1], , :, [$1])
  else
    echo "*** libglade was not compiled with support for $module_args" 1>&2
    AC_MSG_RESULT(no)
    ifelse([$2], , :, [$2])
  fi
fi
AC_SUBST(LIBGLADE_CFLAGS)
AC_SUBST(LIBGLADE_LIBS)
])

PHP_GTK_ARG_ENABLE(libglade,for libglade support,
[  --disable-libglade      Disable libglade support],yes)

if test "$PHP_GTK_LIBGLADE" != "no"; then
  PHP_PATH_LIBGLADE(have_libglade=yes,have_libglade=no)
  if test "$have_libglade" != "yes"; then
    AC_MSG_ERROR(Unable to locate libglade)
  else
	AC_DEFINE(HAVE_LIBGLADE,1,[libglade support])
	PHP_EVAL_INCLINE($LIBGLADE_CFLAGS)
	PHP_EVAL_LIBLINE($LIBGLADE_LIBS, LIBGLADE_SHARED_LIBADD)
	PHP_SUBST(LIBGLADE_SHARED_LIBADD)
	PHP_GTK_EXTENSION(libglade)
  fi
fi
