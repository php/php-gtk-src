dnl $Id$
dnl config.m4 for skeleton module

dnl  PHP_GTK_ARG_ENABLE(skeleton,for GtkSkeleton support,
dnl  [  --enable-skeleton       Enable GtkSkeleton support])

dnl  if test "$PHP_GTK_SKELETON" != "no"; then
dnl    for x in /usr/local /usr; do
dnl  	  if test -f $x/include/gtkskeleton.h; then
dnl  		  SKELETON_DIR=$x
dnl  		  SKELETON_INCDIR=$x/include
dnl  	  fi
dnl    done
dnl  
dnl    if test -z "$SKELETON_DIR"; then
dnl  	  AC_MSG_ERROR(Cannot locate Skeleton)
dnl    fi
dnl  
dnl    SKELETON_LIBDIR=$SKELETON_DIR/lib
dnl  
dnl    AC_DEFINE(HAVE_SKELETON,1,[skeleton support])
dnl    PHP_ADD_INCLUDE($SKELETON_INCDIR)
dnl    PHP_ADD_LIBRARY_WITH_PATH(gtkskeleton, $SKELETON_LIBDIR, SKELETON_SHARED_LIBADD)
dnl    PHP_SUBST(SKELETON_SHARED_LIBADD)
dnl    PHP_GTK_EXTENSION(skeleton)
dnl  fi
