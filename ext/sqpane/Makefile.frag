
$(builddir)/php_sqpane.lo: $(srcdir)/gen_ce_sqpane.h

$(srcdir)/gen_sqpane.c: $(srcdir)/sqpane.defs $(srcdir)/sqpane.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o $(srcdir)/sqpane.overrides -p SQPane -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/sqpane.defs > $(srcdir)/gen_sqpane.c

$(srcdir)/gen_ce_sqpane.h: $(srcdir)/gen_sqpane.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_sqpane.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_sqpane.h

