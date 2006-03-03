
$(builddir)/gen_scintilla.c: $(srcdir)/scintilla.defs $(srcdir)/scintilla.overrides
	( \
     $(PHP) $(top_srcdir)/generator/generator.php \
	 	-l $(@D)/$(*F).log \
        -r ext/gtk+/atk-types.defs   \
        -r ext/gtk+/pango-types.defs \
        -r ext/gtk+/gdk-types.defs   \
        -r ext/gtk+/gtk-types.defs   \
		-o ext/scintilla/scintilla.overrides \
        -p GtkScintilla \
		-f $@ ext/scintilla/scintilla.defs \
     && grep -h "^PHP_GTK_EXPORT_CE" $@ | sed -e "s!^!extern !" > $(@D)/$(*F).h \
	)
