
$(builddir)/php_combobutton.lo: $(srcdir)/gen_ce_combobutton.h

$(srcdir)/gen_combobutton.c: $(srcdir)/combobutton.defs $(srcdir)/combobutton.overrides
	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o $(srcdir)/combobutton.overrides -p ComboButton -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/combobutton.defs > $(srcdir)/gen_combobutton.c

$(srcdir)/gen_ce_combobutton.h: $(srcdir)/gen_combobutton.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_combobutton.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_combobutton.h
