dnl php_gtk.m4 file for the new build system

dnl
dnl PHP_GTK_EXTENSION(extname, shared [, sources [, gen_sources [, extra-cflags]]])
dnl
dnl Includes an extension in the build.
dnl
dnl "extname" is the name of the ext/ subdir where the extension resides
dnl
AC_DEFUN(PHP_GTK_EXTENSION,[
  ext_builddir=[]PHP_EXT_BUILDDIR($1)
  ext_srcdir=[]PHP_EXT_SRCDIR($1)
  php_gtk_ext_builddir=$ext_builddir/ext/$1
  php_gtk_ext_srcdir=$ext_srcdir/ext/$1

  ifelse($5,,,[ac_extra=`echo $ac_n "$5$ac_c"|sed s#@ext_srcdir@#$php_gtk_ext_srcdir#g`])

  if test "$2" != "shared" && test "$2" != "yes"; then
dnl ---------------------------------------------- Static module
    # static module
	ifelse($3,,,PHP_ADD_SOURCES_X($php_gtk_ext_builddir,$3,$ac_extra,shared_objects_php_gtk))
	ifelse($4,,,PHP_ADD_SOURCES_X(/ext/$1,$4,,shared_objects_php_gtk))
	PHP_GTK_EXTENSIONS="$PHP_GTK_EXTENSIONS $1"
  else
dnl ---------------------------------------------- Shared module
    # shared module
	ifelse($3,,,PHP_ADD_SOURCES_X($php_gtk_ext_builddir,$3,$ac_extra,[shared_objects_]translit($1,a-z-+,A-Z__),yes))
	ifelse($4,,,PHP_ADD_SOURCES_X(/ext/$1,$4,$ac_extra,[shared_objects_]translit($1,a-z-+,A-Z__),yes))
	PHP_SHARED_MODULE($1,[shared_objects_]translit($1,a-z-+,A-Z__), $php_gtk_ext_builddir)
	AC_DEFINE_UNQUOTED([PHP_GTK_COMPILE_DL_]translit($1,a-z-+,A-Z__), 1, Whether to build $1 as dynamic module)
  fi

  PHP_ADD_MAKEFILE_FRAGMENT(ext/$1/Makefile.frag, ext/$1, ext/$1)
  PHP_ADD_BUILD_DIR($ext_builddir)
])

if test ! $NEW_BUILD_SYSTEM; then
  PHP_SUBST(PHP_GTK_EXT_SUBDIRS)
  PHP_SUBST(PHP_GTK_EXT_LTLIBS)
fi
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

dnl PHP_GTK_EXTENSION(gtk+, no, php_gtk+.c php_gdk.c php_gtk+_types.c, gen_gtk.c gen_gdk.c)
PHP_NEW_EXTENSION(php_gtk, main/php_gtk.c main/php_gtk_object.c main/php_gtype.c main/php_gtk_util.c, $ext_shared,, -I@ext_srcdir@/main)

dnl reading config stubs
dnl esyscmd(./build2/config-stubs ext)
sinclude(ext/gtk+/config.m4)

dnl PHP_ADD_SOURCES_X(/main, php_gtk_ext.c,, shared_objects_php_gtk)
PHP_ADD_MAKEFILE_FRAGMENT(main/Makefile.frag, main)

