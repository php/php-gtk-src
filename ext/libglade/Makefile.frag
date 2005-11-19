
$(builddir)/gen_libglade.c: $(srcdir)/libglade.defs $(srcdir)/libglade.overrides
	( \
     $(PHP) $(top_srcdir)/generator/generator.php \
	 	-l $(@D)/$(*F).log \
        -r ext/gtk+/atk-types.defs   \
        -r ext/gtk+/pango-types.defs \
        -r ext/gtk+/gdk-types.defs   \
        -r ext/gtk+/gtk-types.defs   \
        -o ext/libglade/libglade.overrides \
        -p Glade \
        -f $@ ext/libglade/libglade.defs \
     && grep -h "^PHP_GTK_EXPORT_CE" $@ | sed -e "s!^!extern !" > $(@D)/$(*F).h \
	)
