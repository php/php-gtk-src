
$(builddir)/php_scrollpane.lo: $(srcdir)/gen_ce_scrollpane.h

$(srcdir)/gen_scrollpane.c: $(srcdir)/scrollpane.defs $(srcdir)/scrollpane.overrides
	$(PHP) -q $(top_srcdir)/generator/generator.php -o $(srcdir)/scrollpane.overrides -p Scrollpane -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/scrollpane.defs > $(srcdir)/gen_scrollpane.c

$(srcdir)/gen_ce_scrollpane.h: $(srcdir)/gen_scrollpane.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_scrollpane.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_scrollpane.h

