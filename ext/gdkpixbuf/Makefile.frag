$(builddir)/php_gdkpixbuf.lo: $(srcdir)/gen_ce_gdkpixbuf.h

$(srcdir)/gen_gdkpixbuf.c: $(srcdir)/gdkpixbuf.defs $(srcdir)/gdkpixbuf.overrides
	$(PHP) -q $(top_srcdir)/generator/generator.php -o $(srcdir)/gdkpixbuf.overrides -p gdk_pixbuf -c GdkPixbuf -r $(top_srcdir)/ext/gtk%2b/gtk.defs -r $(top_srcdir)/ext/gtk%2b/gdk.defs $(srcdir)/gdkpixbuf.defs > $(srcdir)/gen_gdkpixbuf.c

$(srcdir)/gen_ce_gdkpixbuf.h: $(srcdir)/gen_gdkpixbuf.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_gdkpixbuf.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_gdkpixbuf.h

