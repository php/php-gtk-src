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
	@echo config_m4_files = ext/*/config*.m4 >> $@

$(STAMP):
	phpize
	touch $(STAMP)
