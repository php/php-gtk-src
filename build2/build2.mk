# $Id$ 
#

include build2/generated_lists

all: configure config.h.in

ext.m4: $(config_m4_files)
	@echo rebuilding $@
	@cat $(config_m4_files) > $@

aclocal.m4: configure.in acinclude.m4 ext.m4
	@echo rebuilding $@
	@aclocal
	
configure: aclocal.m4 config.m4 php_gtk.m4 ext.m4
	@echo rebuilding $@
	@autoconf 
	@chmod 755 $@
	
config.h.in: configure
# explicitly remove target since autoheader does not seem to work 
# correctly otherwise (timestamps are not updated)
	@echo rebuilding $@
	@rm -f $@
	@autoheader
