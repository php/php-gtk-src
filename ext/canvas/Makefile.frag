
$(builddir)/php_canvas.lo: $(srcdir)/gen_ce_canvas.h

$(srcdir)/gen_canvas.c: $(srcdir)/canvas.defs $(srcdir)/canvas.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o $(srcdir)/canvas.overrides -p Canvas -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/canvas.defs > $(srcdir)/gen_canvas.c

$(srcdir)/gen_ce_canvas.h: $(srcdir)/gen_canvas.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_canvas.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_canvas.h

