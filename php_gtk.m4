dnl
dnl PHP_GTK_EXTENSION(extname [, shared])
dnl
dnl Includes an extension in the build.
dnl
dnl "extname" is the name of the ext/ subdir where the extension resides
dnl
AC_DEFUN(PHP_GTK_EXTENSION,[
  if test "$1" != "gtk+"; then
    PHP_GTK_EXT_SUBDIRS="$PHP_GTK_EXT_SUBDIRS $1"
  fi
  
  php_gtk_ext_builddir=ext/$1
  php_gtk_ext_srcdir=$abs_srcdir/ext/$1

  if test "$2" != "shared" && test "$2" != "yes"; then
dnl ---------------------------------------------- Static module
	  LIB_BUILD($php_gtk_ext_builddir)
	  PHP_GTK_EXT_LTLIBS="$PHP_GTK_EXT_LTLIBS $php_gtk_ext_builddir/lib$1.la"
	  PHP_GTK_EXTENSIONS="$PHP_GTK_EXTENSIONS $1"
  else
dnl ---------------------------------------------- Shared module
	  LIB_BUILD($php_gtk_ext_builddir,yes)
	  AC_DEFINE_UNQUOTED([PHP_GTK_COMPILE_DL_]translit($1,a-z+-,A-Z__), 1, Whether to build $1 as dynamic module)
  fi

  PHP_FAST_OUTPUT($php_gtk_ext_builddir/Makefile)
])

PHP_SUBST(PHP_GTK_EXT_SUBDIRS)
PHP_SUBST(PHP_GTK_EXT_LTLIBS)
PHP_SUBST(PHP_GTK_EXTENSIONS)


AC_DEFUN(PHP_GTK_ARG_ANALYZE,[
case [$]$1 in
shared,*)
  php_gtk_ext_output="yes, shared"
  php_gtk_ext_shared=yes
  $1=`echo "[$]$1"|sed 's/^shared,//'`
  ;;
shared)
  php_gtk_ext_output="yes, shared"
  php_gtk_ext_shared=yes
  $1=yes
  ;;
no)
  php_gtk_ext_output=no
  php_gtk_ext_shared=no
  ;;
*)
  php_gtk_ext_output=yes
  php_gtk_ext_shared=no
  ;;
esac

AC_MSG_RESULT([$php_gtk_ext_output])
])

dnl
dnl PHP_GTK_ARG_ENABLE(arg-name, check message, help text[, default-val])
dnl Sets PHP_GTK_ARG_NAME either to the user value or to the default value.
dnl default-val defaults to no.  This will also set the variable
dnl php_gtk_ext_shared, and will overwrite any previous variable of that name.
dnl
AC_DEFUN(PHP_GTK_ARG_ENABLE,[
PHP_GTK_REAL_ARG_ENABLE([$1],[$2],[$3],[$4],PHP_GTK_[]translit($1,a-z-,A-Z_))
])

AC_DEFUN(PHP_GTK_REAL_ARG_ENABLE,[
AC_MSG_CHECKING($2)
AC_ARG_ENABLE($1,[$3],$5=[$]enableval,$5=ifelse($4,,no,$4))
PHP_GTK_ARG_ANALYZE($5)
])

dnl
dnl PHP_GTK_ARG_WITH(arg-name, check message, help text[, default-val])
dnl Sets PHP_GTK_ARG_NAME either to the user value or to the default value.
dnl default-val defaults to no.  This will also set the variable
dnl php_gtk_ext_shared, and will overwrite any previous variable of that name.
dnl
AC_DEFUN(PHP_GTK_ARG_WITH,[
PHP_GTK_REAL_ARG_WITH([$1],[$2],[$3],[$4],PHP_GTK_[]translit($1,a-z0-9-,A-Z0-9_))
])

AC_DEFUN(PHP_GTK_REAL_ARG_WITH,[
AC_MSG_CHECKING([$2])
AC_ARG_WITH($1,[$3],$5=[$]withval,$5=ifelse($4,,no,$4))
PHP_GTK_ARG_ANALYZE($5)
])
