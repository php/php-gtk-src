dnl
dnl PHP_GTK_EXTENSION(extname)
dnl
dnl Includes an extension in the build.
dnl
dnl "extname" is the name of the ext/ subdir where the extension resides
dnl
AC_DEFUN(PHP_GTK_EXTENSION,[
  PHP_GTK_EXT_SUBDIRS="$PHP_GTK_EXT_SUBDIRS $1"
  
  php_gtk_ext_builddir=ext/$1
  php_gtk_ext_srcdir=$abs_srcdir/ext/$1

  LIB_BUILD($php_gtk_ext_builddir)
  PHP_GTK_EXT_LTLIBS="$PHP_GTK_EXT_LTLIBS $php_gtk_ext_builddir/lib$1.la"
  PHP_GTK_EXTENSIONS="$PHP_GTK_EXTENSIONS $1"

  PHP_FAST_OUTPUT($php_gtk_ext_builddir/Makefile)
])

PHP_SUBST(PHP_GTK_EXT_SUBDIRS)
PHP_SUBST(PHP_GTK_EXT_LTLIBS)
PHP_SUBST(PHP_GTK_EXTENSIONS)

dnl
dnl PHP_GTK_ARG_ENABLE(arg-name, check message, help text[, default-val])
dnl Sets PHP_GTK_ARG_NAME either to the user value or to the default value.
dnl default-val defaults to no.
dnl
AC_DEFUN(PHP_GTK_ARG_ENABLE,[
PHP_GTK_REAL_ARG_ENABLE([$1],[$2],[$3],[$4],PHP_GTK_[]translit($1,a-z-,A-Z_))
])

AC_DEFUN(PHP_GTK_REAL_ARG_ENABLE,[
AC_MSG_CHECKING($2)
AC_ARG_ENABLE($1,[$3],$5=[$]enableval,$5=ifelse($4,,no,$4))
case [$]$5 in
no)
  AC_MSG_RESULT(no)
  ;;
*)
  AC_MSG_RESULT(yes)
  ;;
esac
])
