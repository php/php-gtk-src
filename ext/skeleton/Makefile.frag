
$(builddir)/php_skeleton.lo: $(srcdir)/gen_ce_skeleton.h

$(srcdir)/gen_skeleton.c: $(srcdir)/skeleton.defs $(srcdir)/skeleton.overrides
	$(PHP) -q $(top_srcdir)/generator/generator.php -o $(srcdir)/skeleton.overrides -p Skeleton -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/skeleton.defs > $(srcdir)/gen_skeleton.c

$(srcdir)/gen_ce_skeleton.h: $(srcdir)/gen_skeleton.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_skeleton.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_skeleton.h

