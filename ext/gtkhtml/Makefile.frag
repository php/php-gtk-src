
$(builddir)/php_gtkhtml.lo: $(srcdir)/gen_ce_gtkhtml.h

$(srcdir)/gen_gtkhtml.c: $(srcdir)/gtkhtml.defs $(srcdir)/gtkhtml.overrides
	$(PHP) -q $(top_srcdir)/generator/generator.php -o $(srcdir)/gtkhtml.overrides -p gtkhtml -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/gtkhtml.defs > $(srcdir)/gen_gtkhtml.c

$(srcdir)/gen_ce_gtkhtml.h: $(srcdir)/gen_gtkhtml.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_gtkhtml.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_gtkhtml.h

