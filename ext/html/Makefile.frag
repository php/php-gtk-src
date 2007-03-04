
$(builddir)/gen_html.c: $(srcdir)/html.defs $(srcdir)/html.overrides
	( \
     $(PHP) $(top_srcdir)/generator/generator.php \
	 	-l $(@D)/$(*F).log \
        -r ext/gtk+/atk-types.defs   \
        -r ext/gtk+/pango-types.defs \
        -r ext/gtk+/gdk-types.defs   \
        -r ext/gtk+/gtk-types.defs   \
		-r ext/html/html-types.defs \
		-o ext/html/html.overrides \
        -p GtkHTML \
		-f $@ ext/html/html.defs \
     && grep -h "^PHP_GTK_EXPORT_CE" $@ | sed -e "s!^!extern !" > $(@D)/$(*F).h \
	)
