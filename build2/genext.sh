#! /bin/sh

# $Id: genext.sh,v 1.4 2001-11-20 20:36:30 andrei Exp $
# replacement for genif.pl

infile="$1"
shift
srcdir="$1"
shift
extra_module_ptrs="$1"
shift
awk="$1"
shift

if test "$infile" = "" -o "$srcdir" = ""; then
	echo "please supply infile and srcdir"
	exit 1
fi

module_ptrs="$extra_module_ptrs"
header_list=""
olddir=`pwd`
cd $srcdir

for ext in ${1+"$@"} ; do
	if test "$ext" = "gtk+"; then
		continue
	fi
	ext_canonical=`echo ${ext} | sed -e 's![^a-zA-Z0-9_]!_!'`
	module_ptrs="	php_gtk_ext_${ext_canonical}_ptr,@NEWLINE@$module_ptrs"
	header_list="$header_list ext/$ext/*.h"
done

if test -n "$header_list"; then
    includes=`$awk -f ./build2/print_include.awk $header_list`
fi

cd $olddir

cat $infile | \
	sed \
	-e "s'@EXT_INCLUDE_CODE@'$includes'" \
	-e "s'@EXT_PTRS@'$module_ptrs'" \
	-e 's/@NEWLINE@/\
/g'


