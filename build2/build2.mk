# $Id$ 
#

include build2/generated_lists

all: configure config.h.in

ext.m4: $(config_m4_files)
	@cat $(config_m4_files) > $@

configure: aclocal.m4 config.m4 php_gtk.m4 ext.m4
	@echo rebuilding $@
	@aclocal
	@autoconf 

config.h.in: configure
# explicitly remove target since autoheader does not seem to work 
# correctly otherwise (timestamps are not updated)
	@echo rebuilding $@
	@rm -f $@
	@autoheader
