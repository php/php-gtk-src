#include "php_gtk.h"

#if HAVE_PHP_GTK
#include <gtkspell/gtkspell.h>
#include "ext/gtk+/php_gtk+.h"
#include "php_gtk_api.h"

PHP_GTK_EXPORT_CE(gtkspell_ce);

static PHP_METHOD(GtkSpell, __construct)
{
    GObject *wrapped_obj;
    GtkTextView *view;
    zval *php_view;

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "N", &php_view, gtktextview_ce)) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(GtkSpell);
    }
    
    view = GTK_TEXT_VIEW(PHPG_GOBJECT(php_view));

    wrapped_obj = (GObject *) gtkspell_new_attach(view, NULL, NULL);

	if (!wrapped_obj) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(GtkSpell);
	}

    phpg_gobject_set_wrapper(this_ptr, wrapped_obj TSRMLS_CC);
}

static PHP_METHOD(GtkSpell, detach)
{
    NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

    gtkspell_detach((GtkSpell*)(PHPG_GOBJECT(this_ptr)));
}

static PHP_METHOD(GtkSpell, recheck_all)
{
    NOT_STATIC_METHOD();

    if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
        return;

    gtkspell_recheck_all((GtkSpell*)(PHPG_GOBJECT(this_ptr)));
}

static PHP_METHOD(GtkSpell, set_language)
{
    gchar *lang;
     
    NOT_STATIC_METHOD();

    if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &lang))
        return;

    RETVAL_BOOL(gtkspell_set_language((GtkSpell*)(PHPG_GOBJECT(this_ptr)), (const gchar*) lang, NULL));
}

static function_entry gtkspell_methods[] = {
	PHP_ME(GtkSpell, __construct,  NULL, ZEND_ACC_PUBLIC)
	PHP_ME(GtkSpell, detach,       NULL, ZEND_ACC_PUBLIC)
    PHP_ME(GtkSpell, recheck_all,  NULL, ZEND_ACC_PUBLIC)
    PHP_ME(GtkSpell, set_language, NULL, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

void phpg_gtkspell_register_classes(void)
{
	TSRMLS_FETCH();

	gtkspell_ce = phpg_register_class("GtkSpell", gtkspell_methods, gtktextview_ce, 0, NULL, NULL, GTK_TYPE_TEXT_BUFFER TSRMLS_CC);
}

void phpg_gtkspell_register_constants(const char *strip_prefix)
{
    TSRMLS_FETCH();


    /* register gtype constants for all classes */

	phpg_register_int_constant(gtkspell_ce, "gtype", sizeof("gtype")-1, GTK_TYPE_TEXT_BUFFER);

}

#endif /* HAVE_PHP_GTK */
