
$(builddir)/php_libglade.lo: $(builddir)/gen_ce_libglade.h

$(builddir)/gen_libglade.c: $(srcdir)/libglade.defs $(srcdir)/libglade.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o $(srcdir)/libglade.overrides -p libglade -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/libglade.defs > $@

$(builddir)/gen_ce_libglade.h: $(builddir)/gen_libglade.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_libglade.c | sed -e "s!^!extern !" > $@

