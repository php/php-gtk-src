
$(builddir)/php_spaned.lo: $(srcdir)/gen_ce_spaned.h

$(srcdir)/gen_spaned.c: $(srcdir)/spaned.defs $(srcdir)/spaned.overrides
	$(PHP) -q $(top_srcdir)/generator/generator.php -o $(srcdir)/spaned.overrides -p SPaned -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/spaned.defs > $(srcdir)/gen_spaned.c

$(srcdir)/gen_ce_spaned.h: $(srcdir)/gen_spaned.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_spaned.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_spaned.h

