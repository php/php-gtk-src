# $Id$ 
#

include build2/generated_lists

all: configure config.h.in

configure.ac: configure.in $(config_m4_files)
	@cat configure.in $(config_m4_files) > $@
	@aclocal > /dev/null 2>&1

configure: aclocal.m4 config.m4 php_gtk.m4 configure.ac
	@echo rebuilding $@
	@aclocal
	@autoconf 

config.h.in: configure
# explicitly remove target since autoheader does not seem to work 
# correctly otherwise (timestamps are not updated)
	@echo rebuilding $@
	@rm -f $@
	@autoheader
