/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001,2002 Andrei Zmievski <andrei@php.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
/* $Id$ */

#include "php_spell.h"
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

	gtkspell_ce = phpg_register_class("GtkSpell", gtkspell_methods, gobject_ce, 0, NULL, NULL, 0 TSRMLS_CC);
}

void phpg_gtkspell_register_constants(const char *strip_prefix)
{
    TSRMLS_FETCH();


    /* register gtype constants for all classes */

	phpg_register_int_constant(gtkspell_ce, "gtype", sizeof("gtype")-1, 0);

}
#endif /* HAVE_PHP_GTK */

#if HAVE_SPELL

#ifdef PHP_GTK_COMPILE_DL_SPELL
PHP_GTK_GET_EXTENSION(spell)
#endif

PHP_GTK_XINIT_FUNCTION(spell)
{
	phpg_gtkspell_register_classes();
    phpg_gtkspell_register_constants("SPELL_");

	return SUCCESS;
}

php_gtk_ext_entry spell_ext_entry = {
	"spell",
	PHP_GTK_XINIT(spell),
	NULL,
};

#endif	/* HAVE_SPELL */
