# $Id$ 
#

include build2/generated_lists

all: configure config.h.in

configure: aclocal.m4 config.m4 php_gtk.m4 configure.in $(config_m4_files)
	@echo rebuilding $@
	@autoconf

php_gtk.m4: $(php_gtk_m4)
	cp $(php_gtk_m4) php_gtk.m4

config.h.in: configure
# explicitly remove target since autoheader does not seem to work 
# correctly otherwise (timestamps are not updated)
	@echo rebuilding $@
	@rm -f $@
	@autoheader
