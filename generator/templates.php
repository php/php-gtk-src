<?php
/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001 Andrei Zmievski <andrei@php.net>
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

$function_tpl = "
PHP_FUNCTION(wrap_%s)
{
%s	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%s\"%s))
		return;

%s%s
}\n\n";

$function_call_tpl = "%s(%s)";

$method_tpl = "
PHP_FUNCTION(wrap_%s)
{
%s	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%s\"%s))
		return;

%s%s
}\n\n";

$method_call_tpl = "%s(%s(PHP_GTK_GET(this_ptr))%s)";

$constructor_tpl = "
PHP_FUNCTION(wrap_%s)
{
%s	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%s\"%s)) {
		php_gtk_invalidate(this_ptr);
		return;
	}

%s	wrapped_obj = (GtkObject *)%s(%s);
	if (!wrapped_obj) {
		php_error(E_WARNING, \"%%s(): could not create %s object\",
				  get_active_function_name());
		return;
	}

	php_gtk_object_init(wrapped_obj, this_ptr);
}\n\n";

$function_entry_tpl = "\t{\"%s\",	PHP_FN(wrap_%s),	%s},\n";

$register_getter_tpl = "\tphp_gtk_register_prop_getter(%s, %s_get_property);\n";

$prop_check_tpl =
"%sif (!strcmp(prop_name, \"%s\")) {
	%s
	}";

$prop_getter_tpl = "
static void %s_get_property(zval *return_value, zval *object, zend_llist_element **element, int *found)
{
	char *prop_name = Z_STRVAL(((zend_overloaded_element *)(*element)->data)->element);

	ZVAL_NULL(return_value);
	*found = SUCCESS;

%s

	*found = FAILURE;
}\n\n";

$init_class_tpl = "\n\tINIT_OVERLOADED_CLASS_ENTRY(ce, \"%s\", %s_functions, NULL, %s, NULL);\n";

$get_type_tpl = "
PHP_FUNCTION(wrap_%s_get_type)
{
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"\"))
		return;

	RETURN_LONG(%s_get_type());
}\n\n";

?>
