# $Id$ 
#
#
# Makefile to generate build tools
#

STAMP = buildmk.stamp

ALWAYS = build2/generated_lists

all: $(ALWAYS) $(STAMP)
	@$(MAKE) -s -f build2/build2.mk

build2/generated_lists:
	@echo config_m4_files = ext/*/config*.m4 >> $@
	if test -f Makefile.global; then echo php_gtk_m4 = php_gtk_new.m4 >> $@; cp php_gtk_new.m4 php_gtk.m4; else echo php_gtk_m4 = php_gtk_old.m4 >> $@; cp php_gtk_old.m4 php_gtk.m4; fi

$(STAMP):
	phpize
	if test -f php_gtk.m4; then rm php_gtk.m4; fi
	touch $(STAMP)
