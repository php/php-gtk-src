
$(builddir)/php_piemenu.lo: $(srcdir)/gen_ce_piemenu.h

$(srcdir)/gen_piemenu.c: $(srcdir)/piemenu.defs $(srcdir)/piemenu.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o $(srcdir)/piemenu.overrides -p PieMenu -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/piemenu.defs > $(srcdir)/gen_piemenu.c

$(srcdir)/gen_ce_piemenu.h: $(srcdir)/gen_piemenu.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_piemenu.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_piemenu.h

