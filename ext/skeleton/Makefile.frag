
$(builddir)/php_skeleton.lo: $(builddir)/gen_ce_skeleton.h

$(builddir)/gen_skeleton.lo: $(builddir)/gen_skeleton.c

$(builddir)/gen_skeleton.c: $(srcdir)/skeleton.defs $(srcdir)/skeleton.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o $(srcdir)/skeleton.overrides -p Skeleton -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/skeleton.defs > $@

$(builddir)/gen_ce_skeleton.h: $(builddir)/gen_skeleton.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_skeleton.c | sed -e "s!^!extern !" > $@

