<?php
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

class Templates {
const function_call = "%s(%s)";

const function_body = "
static PHP_METHOD(%(scope), %(name))
{
%(var_list)\tif (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list)))
		return;
%(pre_code)
    %(return)%(cname)(%(arg_list));
%(post_code)
}\n\n";

const method_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)\tNOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list)))
		return;
%(pre_code)
    %(return)%(cname)(%(cast)(PHPG_GET(this_ptr))%(arg_list));
%(post_code)
}\n\n";

const constructor_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)\tGObject *wrapped_obj;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list))) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
%(pre_code)
	wrapped_obj = (GObject *)%(cname)(%(arg_list));
%(post_code)
	if (!wrapped_obj) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
    phpg_gobject_set_wrapper(this_ptr, wrapped_obj TSRMLS_CC);
}\n\n";

const static_constructor_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)\tGObject *wrapped_obj;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list))) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
%(pre_code)
	wrapped_obj = (GObject *)%(cname)(%(arg_list));
%(post_code)
	if (!wrapped_obj) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
    phpg_gobject_new(&return_value, wrapped_obj TSRMLS_CC);
}\n\n";

const boxed_constructor_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)\tphpg_gboxed_t *pobj = NULL;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list))) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
%(pre_code)
    pobj = zend_object_store_get_object(this_ptr TSRMLS_CC);
    pobj->gtype = %(typecode);
    pobj->boxed = %(cname)(%(arg_list));
%(post_code)
	if (!pobj->boxed) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
    pobj->free_on_destroy = TRUE;
}\n\n";

const boxed_static_constructor_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)\t%(class) *wrapped_obj = NULL;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list))) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
%(pre_code)
    wrapped_obj = %(cname)(%(arg_list));
%(post_code)
	if (!wrapped_obj) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
    phpg_gboxed_new(&return_value, %(typecode), wrapped_obj, FALSE, TRUE TSRMLS_CC);
}\n\n";

const boxed_method_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)\tNOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list)))
		return;
%(pre_code)
    %(return)%(cname)(phpg_gboxed_get(this_ptr, %(typecode))%(arg_list));
%(post_code)
}\n\n";

const deprecation_msg = "\n\tphpg_warn_deprecated(%s TSRMLS_CC);\n";
const method1_call = "%s(%s(PHP_GTK_GET(this_ptr))%s)";
const method2_call = "%s(PHP_%s_GET(this_ptr)%s)";

const gtk_object_init = "
	php_gtk_object_init(wrapped_obj, this_ptr);\n";

const non_gtk_object_init = "
	php_gtk_set_object(this_ptr, wrapped_obj, le_%s);\n";

const function_entry = "\tPHP_ME(%s, %s, %s, %s)\n";
const functions_decl = "
static function_entry %s_methods[] = {
";
const functions_decl_end = "\t{NULL, NULL, NULL}\n};\n\n";

const register_getter = "\tphp_gtk_register_prop_getter(%s, %s_get_property);\n";

const prop_check =
"%sif (!strcmp(prop_name, \"%s\")) {
	%s
	}";

const prop_getter = "
static void %s_get_property(zval *return_value, zval *object, char *prop_name, int *result)
{
	*result = SUCCESS;

%s

	*result = FAILURE;
}\n\n";

const init_class = "
	INIT_OVERLOADED_CLASS_ENTRY(ce, \"%s\", php_%s_functions, NULL, %s, php_gtk_set_property);\n";

const struct_class = "
	INIT_CLASS_ENTRY(ce, \"%s\", php_%s_functions);\n";

const get_type = "
PHP_FUNCTION(%s_get_type)
{
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"\"))
		return;

	RETURN_LONG(%s_get_type());
}\n\n";

const register_classes = "
void phpg_%s_register_classes(void)
{
	TSRMLS_FETCH();
%s}\n";

const register_class = "
	%(ce) = phpg_register_class(\"%(class)\", %(methods), %(parent), %(ce_flags), %(propinfo), NULL, %(typecode) TSRMLS_CC);\n";

const register_boxed = "
    %(ce) = phpg_register_boxed(\"%(class)\", %(methods), %(propinfo), %(typecode) TSRMLS_CC);\n";

const class_entry = "PHP_GTK_EXPORT_CE(%s);\n";

const class_prop_list_header = "static char *php_%s_properties[] = {\n";
const class_prop_list_footer = "\tNULL\n};\n\n";

const struct_init = "
zval *PHP_GTK_EXPORT_FUNC(php_%s_new)(%s *obj)
{
	zval *result;
	TSRMLS_FETCH();

    if (!obj) {
        MAKE_STD_ZVAL(result);
        ZVAL_NULL(result);
        return result;
    }

    MAKE_STD_ZVAL(result);
    object_init_ex(result, %s);

%s
    return result;
}\n\n";

const struct_construct = "
PHP_FUNCTION(%s)
{
%s	NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%s\"%s)) {
		php_gtk_invalidate(this_ptr);
		return;
	}

%s}\n\n";

const struct_get = "
zend_bool PHP_GTK_EXPORT_FUNC(php_%s_get)(zval *wrapper, %s *obj)
{
    zval **item;
	TSRMLS_FETCH();

    if (!php_gtk_check_class(wrapper, %s))
        return 0;

%s
    return 1;
}\n\n";

const register_constants = "
void phpg_%s_register_constants(const char *strip_prefix)
{
    TSRMLS_FETCH();

%s
}\n";

const register_enum = "\tphpg_register_%s(%s, strip_prefix, %s);\n";

}

/* vim: set et sts=4: */
?>
