
$(builddir)/php_imlib.lo: $(srcdir)/gen_ce_imlib.h

$(srcdir)/gen_imlib.c: $(srcdir)/imlib.defs $(srcdir)/imlib.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o $(srcdir)/imlib.overrides -p GdkImlib -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/imlib.defs > $(srcdir)/gen_imlib.c

$(srcdir)/gen_ce_imlib.h: $(srcdir)/gen_imlib.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_imlib.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_imlib.h

