dnl $Id$
dnl config.m4 for skeleton module

PHP_GTK_ARG_ENABLE(skeleton,for Skeleton support,
[  --enable-skeleton      Enable Skeleton support])

if test "$PHP_GTK_SKELETON" != "no"; then
  for x in /usr/local /usr; do
	  if test -f $x/include/gtkskeleton.h; then
		  SKELETON_DIR=$x
		  SKELETON_INCDIR=$x/include
	  fi
  done

  if test -z "$SKELETON_DIR"; then
	  AC_MSG_ERROR(Cannot locate Skeleton)
  fi

  SKELETON_LIBDIR=$SKELETON_DIR/lib

  AC_DEFINE(HAVE_SKELETON,1,[skeleton support])
  PHP_ADD_INCLUDE($SKELETON_INCDIR)
  PHP_ADD_LIBRARY_WITH_PATH(gtkskeleton, $SKELETON_LIBDIR, SKELETON_SHARED_LIBADD)
  PHP_SUBST(SKELETON_SHARED_LIBADD)
  PHP_GTK_EXTENSION(skeleton)
fi
