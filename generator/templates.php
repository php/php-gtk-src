<?php

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
