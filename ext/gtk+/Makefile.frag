#$(builddir)/php_gtk+.o $(builddir)/php_gtk+.lo: $(srcdir)/gen_gtk.h

#$(srcdir)/gen_gtk.c: $(srcdir)/gtk.defs $(srcdir)/gtk-extrafuncs.defs $(srcdir)/gtk.overrides
#	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o ext/gtk%2b/gtk.overrides -p Gtk -r $(top_srcdir)/ext/gtk%2b/gdk.defs ext/gtk%2b/gtk.defs > $(srcdir)/gen_gtk.c
# $(PHP) -f $(top_srcdir)/generator/generator.php -- -o ext/gtk%2b/gtk.overrides -p Gtk ext/gtk%2b/gtk.defs > $(srcdir)/gen_gtk.c

#$(srcdir)/gen_gdk.c: $(srcdir)/gdk.defs $(srcdir)/gdk.overrides
#	$(PHP) -f $(top_srcdir)/generator/generator.php -- -o ext/gtk%2b/gdk.overrides -p Gdk ext/gtk%2b/gdk.defs > $(srcdir)/gen_gdk.c
#
#$(srcdir)/gen_gtk.h: $(srcdir)/gen_gtk.c $(srcdir)/gen_gdk.c
#$(srcdir)/gen_gtk.h: $(srcdir)/gen_gtk.c 
#	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_gtk.c $(srcdir)/gen_gdk.c | sed -e "s!^!extern !" > $(srcdir)/gen_gtk.h
#	grep -h "^PHP_GTK_EXPORT_CE" $(srcdir)/gen_gtk.c | sed -e "s!^!extern !" > $(srcdir)/gen_gtk.h
#	grep -h "PHP_GTK_EXPORT_FUNC" $(srcdir)/gen_gtk.c $(srcdir)/gen_gdk.c | sed -e 's!$$!;!' >> $(srcdir)/gen_gtk.h

COMMONDEFS=$(srcdir)/gtk-types.defs $(srcdir)/gdk-types.defs \
		   $(srcdir)/atk-types.defs $(srcdir)/pango-types.defs

$(srcdir)/gen_gtk.c: $(srcdir)/gtk.defs $(COMMONDEFS) $(srcdir)/gtk-extrafuncs.defs $(srcdir)/gtk.overrides
$(srcdir)/gen_gdk.c: $(srcdir)/gdk.defs $(COMMONDEFS) $(srcdir)/gdk.overrides
$(srcdir)/gen_atk.c: $(COMMONDEFS) $(srcdir)/atk.overrides
$(srcdir)/gen_pango.c: $(COMMONDEFS) $(srcdir)/pango.overrides

gen_%.c : %.defs
	( \
	 cd ext/gtk+ \
     && $(PHP) $(top_srcdir)/generator/generator.php \
        -r atk-types.defs \
        -r pango-types.defs \
        -r gdk-types.defs \
        -r gtk-types.defs \
        -o $(*F).overrides \
        -p $(*F) \
        -f $(@F) $(*F).defs 2>&1 | tee gen_$(*F).log \
     && grep -h "^PHP_GTK_EXPORT_CE" $(@F) | sed -e "s!^!extern !" > gen_$(*F).h \
	)
