# $Id$ 
#
#
# Makefile to generate build tools
#

STAMP = buildmk.stamp

ALWAYS = generated_lists

all: $(STAMP) $(ALWAYS)
	@$(MAKE) -s -f build2/build2.mk

generated_lists:
	@echo config_m4_files = ext/*/config*.m4 >> $@

$(STAMP):
	phpize && touch $(STAMP)
