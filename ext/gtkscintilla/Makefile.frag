
$(builddir)/gen_gtkscintilla.c: $(srcdir)/gtkscintilla.defs $(srcdir)/gtkscintilla.overrides
	( \
     $(PHP) $(top_srcdir)/generator/generator.php \
	 	-l $(@D)/$(*F).log \
        -r ext/gtk+/atk-types.defs   \
        -r ext/gtk+/pango-types.defs \
        -r ext/gtk+/gdk-types.defs   \
        -r ext/gtk+/gtk-types.defs   \
		-o ext/gtkscintilla/gtkscintilla.overrides \
        -p GtkScintilla \
		-f $@ ext/gtkscintilla/gtkscintilla.defs \
     && grep -h "^PHP_GTK_EXPORT_CE" $@ | sed -e "s!^!extern !" > $(@D)/$(*F).h \
	)
