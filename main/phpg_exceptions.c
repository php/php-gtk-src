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

#include "php_gtk.h"

#if HAVE_PHP_GTK

PHP_GTK_API zend_class_entry *phpg_generic_exception = NULL;
PHP_GTK_API zend_class_entry *phpg_construct_exception = NULL;
PHP_GTK_API zend_class_entry *phpg_type_exception = NULL;
PHP_GTK_API zend_class_entry *phpg_gerror_exception = NULL;

void phpg_register_exceptions()
{
	zend_class_entry ce;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(ce, "PhpGtkException", NULL);
#if ZEND_EXTENSION_API_NO > 220051025
	phpg_generic_exception = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
#else
	phpg_generic_exception = zend_register_internal_class_ex(&ce, zend_exception_get_default(), NULL TSRMLS_CC);
#endif
	phpg_generic_exception->ce_flags |= ZEND_ACC_FINAL;
	phpg_generic_exception->constructor->common.fn_flags |= ZEND_ACC_PROTECTED;

	INIT_CLASS_ENTRY(ce, "PhpGtkConstructException", NULL);
	phpg_construct_exception = zend_register_internal_class_ex(&ce, phpg_generic_exception, NULL TSRMLS_CC);
	phpg_construct_exception->ce_flags |= ZEND_ACC_FINAL;
	phpg_construct_exception->constructor->common.fn_flags |= ZEND_ACC_PROTECTED;

	INIT_CLASS_ENTRY(ce, "PhpGtkTypeException", NULL);
	phpg_type_exception = zend_register_internal_class_ex(&ce, phpg_generic_exception, NULL TSRMLS_CC);
	phpg_type_exception->ce_flags |= ZEND_ACC_FINAL;
	phpg_type_exception->constructor->common.fn_flags |= ZEND_ACC_PROTECTED;

	INIT_CLASS_ENTRY(ce, "PhpGtkGErrorException", NULL);
	phpg_gerror_exception = zend_register_internal_class_ex(&ce, phpg_generic_exception, NULL TSRMLS_CC);
	phpg_gerror_exception->ce_flags |= ZEND_ACC_FINAL;
	phpg_gerror_exception->constructor->common.fn_flags |= ZEND_ACC_PROTECTED;
	zend_declare_property_string(phpg_gerror_exception, "domain", sizeof("domain")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(phpg_gerror_exception, "message", sizeof("message")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(phpg_gerror_exception, "code", sizeof("code")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);
}

PHP_GTK_API zval* phpg_throw_gerror_exception(const char *domain, long code, const char *message TSRMLS_DC)
{
	zval *exc;

	MAKE_STD_ZVAL(exc);
	object_init_ex(exc, phpg_gerror_exception);
	zend_update_property_string(phpg_gerror_exception, exc, "domain", sizeof("domain")-1, (char *)domain TSRMLS_CC);
	zend_update_property_long(phpg_gerror_exception, exc, "code", sizeof("code")-1, code TSRMLS_CC);

	if (message) {
		zend_update_property_string(phpg_gerror_exception, exc, "message", sizeof("message")-1, (char *)message TSRMLS_CC);
	}

	zend_throw_exception_object(exc TSRMLS_CC);

	return exc;
}

#endif /* HAVE_PHP_GTK */
