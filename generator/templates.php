<?php
/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001 Andrei Zmievski <andrei@ispi.net>
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

?>
