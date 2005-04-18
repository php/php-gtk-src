# $Id$ 
#
#
# Makefile to generate build tools
#

STAMP = buildmk.stamp

ALWAYS = build2/generated_lists

all: $(STAMP) $(ALWAYS)
	@$(MAKE) -s -f build2/build2.mk

build2/generated_lists:
	@echo config_m4_files = `build2/config-stubs ext ext.m4` > $@

$(STAMP): 
	touch ext.m4
	$(PHPIZE)
	touch $(STAMP)

cvsclean-work:
	@for i in `find . -name .cvsignore`; do \
		(cd `dirname $$i` 2>/dev/null && rm -rf `cat .cvsignore | grep -v config.nice` *.o *.a .libs || true); \
	done

.PHONY: $(ALWAYS)
