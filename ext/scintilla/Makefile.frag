
EXTRA_CXXFLAGS = $(CXXFLAGS) -DSCI_LEXER

$(builddir)/gen_scintilla.c: 
	( \
     $(PHP) ext/scintilla/sci_generator.php \
     	-i ext/scintilla/libscintilla \
	 && grep -h "^PHP_GTK_EXPORT_CE" ext/scintilla/gen_scintilla.c | sed -e "s!^!PHP_GTK_API extern !" > ext/scintilla/gen_scintilla.h \
	)

