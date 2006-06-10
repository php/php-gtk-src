
$(builddir)/php_gtk_ext.c: $(srcdir)/php_gtk_ext.c.in
	@echo "creating main/php_gtk_ext.c"
	sh $(top_srcdir)/build2/genext.sh $(srcdir)/php_gtk_ext.c.in $(top_srcdir) "" $(AWK) $(PHP_GTK_EXTENSIONS) > $@

gen-sources: $(PHP_GTK_GEN_SOURCES)

cvsclean:
	@for i in `find . -name .cvsignore`; do \
		(cd `dirname $$i` 2>/dev/null && rm -rf `cat .cvsignore` *.o *.a || true); \
	done
	@rm -f $(SUBDIRS) 2>/dev/null || true

clean-caches:
	find ext -name \*.cache | xargs rm -f

clean-gen:
	find ext -name gen\*.[ch] | xargs rm -f

.PHONY: gen-sources clean-caches clean-gen
