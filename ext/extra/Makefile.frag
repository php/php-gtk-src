
$(builddir)/php_extra.lo: $(srcdir)/gen_ce_extra.h

$(srcdir)/gen_extra.c: $(srcdir)/extra.defs $(srcdir)/extra.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o $(srcdir)/extra.overrides -p extra -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/extra.defs > $(srcdir)/gen_extra.c

$(srcdir)/gen_ce_extra.h: $(srcdir)/gen_extra.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_extra.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_extra.h

