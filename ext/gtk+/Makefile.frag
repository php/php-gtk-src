$(builddir)/php_gtk+.o $(builddir)/php_gtk+.lo: $(srcdir)/gen_gtk.h

$(srcdir)/gen_gtk.c: $(srcdir)/gtk.defs $(srcdir)/gtk-extrafuncs.defs $(srcdir)/gtk.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o ext/gtk%2b/gtk.overrides -p Gtk -r $(top_srcdir)/ext/gtk%2b/gdk.defs ext/gtk%2b/gtk.defs > $(srcdir)/gen_gtk.c

$(srcdir)/gen_gdk.c: $(srcdir)/gdk.defs $(srcdir)/gdk.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o ext/gtk%2b/gdk.overrides -p Gdk ext/gtk%2b/gdk.defs > $(srcdir)/gen_gdk.c

$(srcdir)/gen_gtk.h: $(srcdir)/gen_gtk.c $(srcdir)/gen_gdk.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_gtk.c $(srcdir)/gen_gdk.c | sed -e "s!^!extern !" > $(srcdir)/gen_gtk.h
	grep -h "PHP_GTK_EXPORT_FUNC" $(srcdir)/gen_gtk.c $(srcdir)/gen_gdk.c | sed -e 's!$$!;!' >> $(srcdir)/gen_gtk.h
