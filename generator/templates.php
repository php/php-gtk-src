<?php
/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2008 Andrei Zmievski <andrei@php.net>
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
%(var_list)
    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list)))
		return;
%(pre_code)
    %(return)%(cname)(%(arg_list));
%(post_code)
}\n\n";

const method_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)
    NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list)))
		return;
%(pre_code)
    %(return)%(cname)(%(cast)(PHPG_GOBJECT(this_ptr))%(arg_list));
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
	wrapped_obj = (GObject *) %(cname)(%(arg_list));
%(post_code)
	if (!wrapped_obj) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
%(post_create_code)
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
	wrapped_obj = (GObject *) %(cname)(%(arg_list));
%(post_code)
	if (!wrapped_obj) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
	}
    phpg_gobject_new(&return_value, wrapped_obj TSRMLS_CC);
    g_object_unref(wrapped_obj); /* phpg_gobject_new() increments reference count */
}\n\n";

const boxed_constructor_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)
    phpg_gboxed_t *pobj = NULL;

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
%(var_list)
    NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list)))
		return;
%(pre_code)
    %(return)%(cname)((%(cast))PHPG_GBOXED(this_ptr)%(arg_list));
%(post_code)
}\n\n";

const pointer_method_body = "
static PHP_METHOD(%(class), %(name))
{
%(var_list)
    NOT_STATIC_METHOD();

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list)))
		return;
%(pre_code)
    %(return)%(cname)((%(cast))PHPG_GPOINTER(this_ptr)%(arg_list));
%(post_code)
e\n\n";

const constructor_with_props = "
static PHP_METHOD(%(class), %(name))
{
    GParameter params[%(n_params)];
	zval *php_args[%(n_args)] = { NULL, };
	char *prop_names[] = { %(props) NULL };
	GType gtype = phpg_gtype_from_zval(this_ptr);
    GObject *wrapped_obj;
    guint i, n_params;

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"%(specs)\"%(parse_list))) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
    }

	memset(&params, 0, %(n_params) * sizeof(GParameter));
    if (!phpg_parse_ctor_props(gtype, php_args, params, &n_params, prop_names TSRMLS_CC)) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
    }
%(pre_code)
    wrapped_obj = (GObject *) g_object_newv(gtype, n_params, params);
%(post_code)
    if (!wrapped_obj) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
    }

	for (i = 0; i < n_params; i++) {
		g_value_unset(&params[i].value);
    }
    phpg_gobject_set_wrapper(this_ptr, wrapped_obj TSRMLS_CC);
}\n\n";

const constructor_without_props = "
static PHP_METHOD(%(class), %(name))
{
    GObject *wrapped_obj;

    if (!php_gtk_parse_args(ZEND_NUM_ARGS(), \"\")) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
    }

%(pre_code)
    wrapped_obj = (GObject *) g_object_newv(phpg_gtype_from_zval(this_ptr), 0, NULL);
%(post_code)
    if (!wrapped_obj) {
        PHPG_THROW_CONSTRUCT_EXCEPTION(%(class));
    }

    phpg_gobject_set_wrapper(this_ptr, wrapped_obj TSRMLS_CC);
}\n\n";
const deprecation_msg = "\n\tphpg_warn_deprecated(%s TSRMLS_CC);\n";
const method1_call = "%s(%s(PHP_GTK_GET(this_ptr))%s)";
const method2_call = "%s(PHP_%s_GET(this_ptr)%s)";

const gtk_object_init = "
	php_gtk_object_init(wrapped_obj, this_ptr);\n";

const non_gtk_object_init = "
	php_gtk_set_object(this_ptr, wrapped_obj, le_%s);\n";

const method_entry = "\tPHP_ME(%s, %s, %s, %s)\n";
const abs_method_entry = "\tZEND_FENTRY(%s, ZEND_MN(%s_%s), %s, %s)\n";
const function_entry = "#if ZEND_EXTENSION_API_NO > 220051025\n\tPHP_ME_MAPPING(%s, %s, NULL, 0)\n#else\n\tPHP_ME_MAPPING(%s, %s, NULL)\n#endif\n";
const alias_entry = "\tPHP_MALIAS(%s, %s, %s, %s, %s)\n";
const functions_decl = "
static zend_function_entry %s_methods[] = {
";
const functions_decl_end = "\t{ NULL, NULL, NULL }\n};\n";

const custom_handlers_init = "\t%(class)_object_handlers = %(orig_handlers);\n";
const custom_handler_set = "\t%(class)_object_handlers.%(handler) = phpg_%(class)_%(handler)_handler;\n";

const custom_create_func = "
static zend_object_handlers %(class)_object_handlers;

static zend_object_value phpg_create_%(class)(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value zov;

    zov = %(create_func)(ce TSRMLS_CC);
    zov.handlers = &%(class)_object_handlers;

    return zov;
}
";

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
void phpg_%s_register_classes(TSRMLS_D)
{
%s}\n";

const register_class = "
	%(ce) = phpg_register_class(\"%(class)\", %(methods), %(parent), %(ce_flags), %(propinfo), %(create_func), %(typecode) TSRMLS_CC);\n%(extra_reg_info)";

const register_boxed = "
    %(ce) = phpg_register_boxed(\"%(class)\", %(methods), %(propinfo), %(create_func), %(typecode) TSRMLS_CC);\n%(extra_reg_info)";

const register_pointer = "
    %(ce) = phpg_register_pointer(\"%(class)\", %(methods), %(create_func), %(typecode) TSRMLS_CC);\n%(extra_reg_info)";


const register_interface = "
	%(ce) = phpg_register_interface(\"%(class)\", %(methods), %(typecode) TSRMLS_CC);\n%(extra_reg_info)";

const implement_interface = "\tzend_class_implements(%(ce) TSRMLS_CC, %(num_ifaces), %(ifaces));\n";
const gtype_constant = "\tphpg_register_int_constant(%s, \"gtype\", sizeof(\"gtype\")-1, %s);\n";

const class_entry = "PHP_GTK_EXPORT_CE(%s);\n";

const prop_info_header = "\nstatic prop_info_t %s_prop_info[] = {\n";
const prop_info_entry  = "\t{ \"%s\", %s, %s },\n";
const prop_info_footer = "\t{ NULL, NULL, NULL },
};\n\n";

const prop_reader = "
PHPG_PROP_READER(%(class), %(name))
{
%(var_list)
    if (((phpg_gobject_t *)object)->obj == NULL) {
        return FAILURE;
    }

    php_retval = %(prop_access);
%(post_code)
    return SUCCESS;
}\n\n";

const prop_access = "%(cast)(((phpg_gobject_t *)object)->obj)->%(name)";
const boxed_prop_access = "((%(cast))((phpg_gboxed_t *)object)->boxed)->%(name)";

const register_constants = "
void phpg_%s_register_constants(const char *strip_prefix TSRMLS_DC)
{
%s
    /* register gtype constants for all classes */

%s
}\n";

const register_enum = "\tphpg_register_%s(%s, strip_prefix, %s);\n";

const reflection_arginfo_begin = "
PHPGTK_ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(%s, 0)
";
const reflection_arginfoex_begin = "
PHPGTK_ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(%s, 0, 0, %d)
";
const reflection_arginfo_end = "ZEND_END_ARG_INFO();
";
const reflection_arg = "    ZEND_ARG_INFO(0, %s)
";
const reflection_objarg = "    ZEND_ARG_OBJ_INFO(0, %s, %s, 1)
";

const reflection_if = "
#if ENABLE_REFLECTION";
const reflection_else = "#else /* ENABLE_REFLECTION */";
const reflection_endif = "#endif /* ENABLE_REFLECTION */
";

}

/* vim: set et sts=4: */
?>
