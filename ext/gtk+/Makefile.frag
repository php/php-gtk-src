COMMONDEFS=$(srcdir)/gtk-types.defs $(srcdir)/gdk-types.defs \
		   $(srcdir)/atk-types.defs $(srcdir)/pango-types.defs

GTK_OVERRIDES = \
	$(srcdir)/gtk.overrides \
	$(srcdir)/gtkcontainer.overrides \
	$(srcdir)/gtkstyle.overrides \
	$(srcdir)/gtktextview.overrides \
	$(srcdir)/gtktreeview.overrides

$(srcdir)/gen_gtk.c: $(srcdir)/gtk.defs $(COMMONDEFS) $(srcdir)/gtk-extrafuncs.defs $(GTK_OVERRIDES)
$(srcdir)/gen_gdk.c: $(srcdir)/gdk.defs $(COMMONDEFS) $(srcdir)/gdk.overrides
$(srcdir)/gen_atk.c: $(COMMONDEFS) $(srcdir)/atk.overrides
$(srcdir)/gen_pango.c: $(COMMONDEFS) $(srcdir)/pango.overrides

gen_%.c : %.defs
	( \
     $(PHP) $(top_srcdir)/generator/generator.php \
	 	-l $(@D)/gen_$(*F).log \
        -r $(*D)/atk-types.defs \
        -r $(*D)/pango-types.defs \
        -r $(*D)/gdk-types.defs \
        -r $(*D)/gtk-types.defs \
        -o $*.overrides \
        -p $(*F) \
        -f $@ $*.defs \
     && grep -h "^PHP_GTK_EXPORT_CE" $@ | sed -e "s!^!extern !" > $(@D)/gen_$(*F).h \
	)
