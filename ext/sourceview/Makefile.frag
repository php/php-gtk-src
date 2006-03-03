
$(builddir)/gen_sourceview.c: $(srcdir)/sourceview.defs $(srcdir)/sourceview.overrides
	( \
     $(PHP) $(top_srcdir)/generator/generator.php \
	 	-l $(@D)/$(*F).log \
        -r ext/gtk+/atk-types.defs   \
        -r ext/gtk+/pango-types.defs \
        -r ext/gtk+/gdk-types.defs   \
        -r ext/gtk+/gtk-types.defs   \
		-o ext/sourceview/sourceview.overrides \
        -p GtkSourceView \
		-f $@ ext/sourceview/sourceview.defs \
     && grep -h "^PHP_GTK_EXPORT_CE" $@ | sed -e "s!^!extern !" > $(@D)/$(*F).h \
	)
