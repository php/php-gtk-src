# $Id$ 
#

include build2/generated_lists

PHP_AUTOCONF ?= 'autoconf'
PHP_AUTOHEADER ?= 'autoheader'

all: configure config.h.in

aclocal.m4: configure.in acinclude.m4
	@echo rebuilding $@
	@if [ -f /usr/share/aclocal/ltsugar.m4 ]; then cat /usr/share/aclocal/lt~obsolete.m4 /usr/share/aclocal/ltoptions.m4 /usr/share/aclocal/ltsugar.m4 /usr/share/aclocal/ltversion.m4 >> ./build/libtool.m4; fi
	cat acinclude.m4 ./build/libtool.m4 php_gtk.m4 > $@
	
configure: aclocal.m4 php_gtk.m4 config.m4 $(config_m4_files)
	@echo rebuilding $@
	$(PHP_AUTOCONF)
	@chmod 755 $@
	
config.h.in: configure
# explicitly remove target since autoheader does not seem to work 
# correctly otherwise (timestamps are not updated)
	@echo rebuilding $@
	@rm -f $@
	$(PHP_AUTOHEADER)
