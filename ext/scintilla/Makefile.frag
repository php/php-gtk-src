
$(builddir)/php_scintilla.lo: $(srcdir)/gen_ce_scintilla.h

$(srcdir)/gen_scintilla.c: $(srcdir)/scintilla.defs $(srcdir)/scintilla.overrides
	$(PHP) -q $(top_srcdir)/generator/generator.php -o $(srcdir)/scintilla.overrides -p Scintilla -r $(top_srcdir)/ext/gtk%2b/gtk.defs $(srcdir)/scintilla.defs > $(srcdir)/gen_scintilla.c

$(srcdir)/gen_ce_scintilla.h: $(srcdir)/gen_scintilla.c
	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_scintilla.c | sed -e "s!^!extern !" > $(srcdir)/gen_ce_scintilla.h

