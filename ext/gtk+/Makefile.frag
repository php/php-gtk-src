
COMMONDEFS=$(srcdir)/gtk-types.defs $(srcdir)/gdk-types.defs \
		   $(srcdir)/atk-types.defs $(srcdir)/pango-types.defs

GTK_OVERRIDES = \
	$(srcdir)/gtk.overrides \
	$(srcdir)/gtkclipboard.overrides \
	$(srcdir)/gtkcontainer.overrides \
	$(srcdir)/gtkstyle.overrides \
	$(srcdir)/gtktextview.overrides \
	$(srcdir)/gtktreeview.overrides \
	$(srcdir)/gtkdrag.overrides

$(builddir)/gen_gtk.c: $(srcdir)/gtk.defs $(COMMONDEFS) $(srcdir)/gtk-extrafuncs.defs $(GTK_OVERRIDES)
$(builddir)/gen_gdk.c: $(srcdir)/gdk.defs $(COMMONDEFS) $(srcdir)/gdk.overrides
$(builddir)/gen_atk.c: $(COMMONDEFS) $(srcdir)/atk.overrides
$(builddir)/gen_pango.c: $(COMMONDEFS) $(srcdir)/pango.overrides

gen_%.c : %.defs
	( \
     $(PHP) $(top_srcdir)/generator/generator.php \
	 	-l $(@D)/gen_$(*F).log \
        -r ext/gtk+/atk-types.defs   \
        -r ext/gtk+/pango-types.defs \
        -r ext/gtk+/gdk-types.defs   \
        -r ext/gtk+/gtk-types.defs   \
        -o $*.overrides \
        -p $(*F) \
        -f $@ $*.defs \
		-v $(GTK_LIBVERSION) \
     && grep -h "^PHP_GTK_EXPORT_CE" $@ | sed -e "s!^!extern !" > $(@D)/gen_$(*F).h \
	)
