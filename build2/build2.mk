# $Id$ 
#

include build2/generated_lists

all: configure config.h

configure: aclocal.m4 config.m4 php_gtk.m4 configure.in $(config_m4_files)
	@echo rebuilding $@
	@autoconf

config.h: configure
# explicitly remove target since autoheader does not seem to work 
# correctly otherwise (timestamps are not updated)
	@echo rebuilding $@
	@rm -f $@
	@autoheader
