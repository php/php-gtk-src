<?php
/*
 * Significant portions of this generator are based on the pygtk code generator
 * developed by James Henstridge <jamesh@daa.com.au>.
 *
 */

require "arg_types.php";
require "override.php";
require "scheme.php";
require "templates.php";

class Generator {
	var $parser 	= null;
	var $overrides  = null;
	var $prefix		= null;

	var $constants	= "/* Generated from gtk.defs */\n\n";

	var $function_entry = "\nstatic function_entry %s_functions[] = {\n";
	var $function_entry_end = "\t{NULL, NULL, NULL}\n};\n\n";
	var $register_classes = "\nvoid php_gtk_register_classes(void)\n{\n\tzend_class_entry ce;\n";

	var $byref_def	= array();

	function Generator(&$parser, &$overrides, $prefix)
	{
		$this->parser 	 = &$parser;
		$this->overrides = &$overrides;
		$this->prefix	 = $prefix;
	}
	
	function register_types()
	{
		global	$matcher;

		foreach ($this->parser->objects as $object)
			$matcher->register_object($object->c_name);

		foreach ($this->parser->enums as $enum) {
			if ($enum->def_type == 'flags')
				$matcher->register_flag($enum->c_name);
			else
				$matcher->register_enum($enum->c_name);
		}
	}

	function parse_byref($parse_type)
	{
		$byref_name = "NULL";
		if (trim(implode('', $parse_type))) {
			$byref_name = str_replace(' ', 'A', implode('', $parse_type));
			if (!isset($this->byref_def[$byref_name])) {
				$this->byref_def[$byref_name] = sprintf("unsigned char byref_" . $byref_name . "[] = {%d, %s};\n", sizeof($parse_type), str_replace(' ', 'BYREF_ALLOW', str_replace('F', 'BYREF_FORCE', implode(',', $parse_type))));
			}
			$byref_name = "byref_$byref_name";
		}
		return $byref_name;
	}
	
	function write_constants($fp)
	{
		fwrite($fp, "\nvoid php_gtk_register_constants(int module_number ELS_DC)\n");
		fwrite($fp, "{\n");
		foreach ($this->parser->enums as $enum) {
			if ($this->overrides->is_ignored($enum->c_name)) continue;
			fwrite($fp, "\n	/* " . $enum->def_type . " " . $enum->c_name . " */\n");
			foreach ($enum->values as $enum_value) {
				fwrite($fp, "	REGISTER_LONG_CONSTANT(\"$enum_value[1]\", $enum_value[1], CONST_CS | CONST_PERSISTENT);\n");
			}
		}
		fwrite($fp, "}\n\n");
	}

	function write_method($fp, $obj_name, $method)
	{
		global	$matcher,
				$method_call_tpl,
				$method_tpl;

		$specs 		= '';
		$var_list 	= new Var_List();
		$parse_list = array('');
		$arg_list 	= array('');
		$extra_code = array();
		$parse_type = array();

		if ($method->varargs)
			trigger_error("varargs functions not supported", E_USER_ERROR);

		foreach ($method->params as $params_array) {
			list($param_type, $param_name, $param_default, $param_null) = $params_array;

			if (isset($param_default) && strpos($specs, '|') === false)
				$specs .= '|';

			$handler = &$matcher->get($param_type);
			if ($handler === null) {
				error_log("Could not write method $method->name (parameter type $param_type)");
				return;
			}
			$specs .= $handler->write_param($param_type,
											$param_name,
											$param_default,
											$param_null,
											$var_list,
											$parse_list,
											$arg_list,
											$extra_code,
											$parse_type,
											false);
		}

		$arg_list 	= implode(', ', $arg_list);
		$parse_list = implode(', ', $parse_list);
		$extra_code = implode('', $extra_code);

		$method_call = sprintf($method_call_tpl,
							   $method->c_name,
							   substr(convert_typename($obj_name), 1),
							   $arg_list);
		$ret_handler = &$matcher->get($method->return_type);
		if ($ret_handler === null) {
			error_log("Could not write function $method->name (return type $method->return_type)");
			return;
		}
		$return_tpl = $ret_handler->write_return($method->return_type, $var_list);
		$return_code = sprintf($return_tpl,
							   $method_call);
		
		$method_code = sprintf($method_tpl,
							   strtolower($method->c_name),
							   $var_list->to_string(),
							   $specs,
							   $parse_list,
							   $extra_code,
							   $return_code);

		fwrite($fp, $method_code);

		return $this->parse_byref($parse_type);
	}

	function write_constructor($fp, $obj_name, $constructor)
	{
		global	$matcher,
				$constructor_tpl;

		$specs 		= '';
		$var_list 	= new Var_List();
		$parse_list = array('');
		$arg_list 	= array();
		$extra_code = array();
		$parse_type = array();

		foreach ($constructor->params as $params_array) {
			list($param_type, $param_name, $param_default, $param_null) = $params_array;

			if (isset($param_default) && strpos($specs, '|') === false)
				$specs .= '|';

			$handler = &$matcher->get($param_type);
			if ($handler === null) {
				error_log("Could not write constructor $constructor->name (parameter type $param_type)");
				return;
			}
			$specs .= $handler->write_param($param_type,
											$param_name,
											$param_default,
											$param_null,
											$var_list,
											$parse_list,
											$arg_list,
											$extra_code,
											$parse_type,
											true);
		}

		$arg_list 	= implode(', ', $arg_list);
		$parse_list = implode(', ', $parse_list);
		$extra_code = implode('', $extra_code);

		$var_list->add('GtkObject', '*wrapped_obj');

		$constructor_code = sprintf($constructor_tpl,
									strtolower($constructor->c_name),
									$var_list->to_string(),
									$specs,
									$parse_list,
									$extra_code,
									$constructor->c_name,
									$arg_list,
									$obj_name);

		fwrite($fp, $constructor_code);

		return $this->parse_byref($parse_type);
	}

	function write_function($fp, $function)
	{
		global	$matcher,
				$function_call_tpl,
				$function_tpl;

		$specs 		= '';
		$var_list 	= new Var_List();
		$parse_list = array('');
		$arg_list 	= array();
		$extra_code = array();
		$parse_type = array();

		if ($function->varargs)
			trigger_error("varargs functions not supported", E_USER_ERROR);

		foreach ($function->params as $params_array) {
			list($param_type, $param_name, $param_default, $param_null) = $params_array;

			if (isset($param_default) && strpos($specs, '|') === false)
				$specs .= '|';

			$handler = &$matcher->get($param_type);
			if ($handler === null) {
				error_log("Could not write function $function->name (parameter type $param_type)");
				return;
			}
			$specs .= $handler->write_param($param_type,
											$param_name,
											$param_default,
											$param_null,
											$var_list,
											$parse_list,
											$arg_list,
											$extra_code,
											$parse_type,
											false);
		}

		$arg_list 	= implode(', ', $arg_list);
		$parse_list = implode(', ', $parse_list);
		$extra_code = implode('', $extra_code);

		$function_call = sprintf($function_call_tpl,
								 $function->c_name,
								 $arg_list);
		$ret_handler = &$matcher->get($function->return_type);
		if ($ret_handler === null) {
			error_log("Could not write function $function->name (return type $function->return_type)");
			return;
		}
		$return_tpl = $ret_handler->write_return($function->return_type, $var_list);
		$return_code = sprintf($return_tpl,
							   $function_call);

		$function_code = sprintf($function_tpl,
								 strtolower($function->c_name),
								 $var_list->to_string(),
								 $specs,
								 $parse_list,
								 $extra_code,
								 $return_code);

		fwrite($fp, $function_code);

		return $this->parse_byref($parse_type);
	}

	function write_static_vars($fp) {
		fwrite($fp, "\n/* static variables  */\n");
		foreach ($this->parser->objects as $object)
			fwrite($fp, "static zend_class_entry *$object->ce;\n");
	}
	
	function write_objects($fp)
	{
		global $function_entry_tpl;
		
		foreach ($this->parser->objects as $object) {
			fwrite($fp, "\n/* object $object->c_name  */\n");
			$this->register_classes .= "\n\tINIT_CLASS_ENTRY(ce, \"gtk" . strtolower($object->name) . "\", " . $this->prefix . "_" . strtolower($object->name) . "_functions);\n";
			if ($object->parent == null)
				$this->register_classes .= "\t$object->ce = zend_register_internal_class_ex(&ce, NULL, NULL);\n";
			else {
				$parent_obj = $this->parser->find_parent($object);
				$this->register_classes .= "\t$object->ce = zend_register_internal_class_ex(&ce, $parent_obj->ce, NULL);\n"
										. "\tg_hash_table_insert(php_gtk_class_hash, g_strdup(\"Gtk$object->name\"), $object->ce);\n";
			}
			$function_entry = sprintf($this->function_entry, $this->prefix . "_" . strtolower($object->name));
			$constructor = $this->parser->find_constructor($object);
			if ($this->overrides->is_overriden($constructor->c_name)) {
				list(, $constructor_override) = $this->overrides->get_override($constructor->c_name);
				fwrite($fp, $constructor_override . "\n");
				$function_entry .= sprintf($function_entry_tpl,
										   strtolower($constructor->is_constructor_of),
										   strtolower($constructor->c_name),
										   'NULL');
			}
			else if ($constructor != null && !$this->overrides->is_ignored($constructor->c_name)) {
				if ($byref = $this->write_constructor($fp, $object->c_name, $constructor)) {
					$function_entry .= sprintf($function_entry_tpl,
											   strtolower($constructor->is_constructor_of),
											   strtolower($constructor->c_name),
											   $byref);
				} else {
					$function_entry .= sprintf($function_entry_tpl,
											   strtolower($constructor->is_constructor_of),
											   'no_constructor', 'NULL');
				}
			}

			foreach ($this->parser->find_methods($object) as $method) {
				if ($this->overrides->is_overriden($method->c_name)) {
					list($method_name, $method_override) = $this->overrides->get_override($method->c_name);
					fwrite($fp, $method_override . "\n");
					if (!isset($method_name))
						$method_name = $method->name;
					$function_entry .= sprintf($function_entry_tpl,
											   strtolower($method->name),
											   strtolower($method->c_name),
											   'NULL');
				}
				else if (!$this->overrides->is_ignored($method->c_name)) {
					if ($byref = $this->write_method($fp, $object->c_name, $method)) {
						$function_entry .= sprintf($function_entry_tpl,
												   strtolower($method->name),
												   strtolower($method->c_name),
												   $byref);
					}
				}

			}
			$function_entry .= $this->function_entry_end;
			fwrite($fp, $function_entry);
		}
	}

	function write_functions($fp)
	{
		global $function_entry_tpl;
		
		fwrite($fp, "\n/* object Gtk  */\n");
		fwrite($fp, "static zend_class_entry *gdk_ce;\n");
		fwrite($fp, "static zend_class_entry *gtk_ce;\n");
		$this->register_classes .= "\n\tINIT_CLASS_ENTRY(ce, \"gdk\", php_gdk_functions);\n"
								. "\tgdk_ce = zend_register_internal_class_ex(&ce, NULL, NULL);\n";
		$this->register_classes .= "\n\tINIT_CLASS_ENTRY(ce, \"gtk\", php_gtk_functions);\n"
								. "\tgtk_ce = zend_register_internal_class_ex(&ce, NULL, NULL);\n";
		$function_entry["gdk"] = sprintf($this->function_entry, 'php_gdk');
		$function_entry["gtk"] = sprintf($this->function_entry, $this->prefix);

		foreach ($this->parser->functions as $function) {
			if ($this->overrides->is_overriden($function->c_name)) {
				list($function_name, $function_override) = $this->overrides->get_override($function->c_name);
				fwrite($fp, $function_override . "\n");
				if (!isset($function_name)) {
					if ($function->name == $function->c_name)
						$function_name = substr($function->name, 4);
					else
						$function_name = $function->name;
				}
				$function_entry[substr($function->c_name, 0, 3)] .= sprintf($function_entry_tpl, 
																			strtolower($function_name), 
																			strtolower($function->c_name), 
																			'NULL');
			}
			else if (!$this->overrides->is_ignored($function->c_name)) {
				if ($byref = $this->write_function($fp, $function)) {
					if ($function->name == $function->c_name)
						$function_name = substr($function->name, 4);
					else
						$function_name = $function->name;
					$function_entry[substr($function->c_name, 0, 3)] .= sprintf($function_entry_tpl, 
																				$function_name, 
																				strtolower($function->c_name), 
																				$byref);
				}
			}
		}
		$function_entry["gdk"] .= $this->function_entry_end;
		$function_entry["gtk"] .= $this->function_entry_end;
		fwrite($fp, $function_entry["gdk"]);
		fwrite($fp, $function_entry["gtk"]);
	}

	function write_no_constructor($fp)
	{
		fwrite($fp, "PHP_FUNCTION(wrap_no_constructor)\n" .
			   		"{\n" .
					" 	php_error(E_WARNING, \"%s: An abstract or unimplemented class\", get_active_function_name());\n" .
					"	php_gtk_invalidate(this_ptr);\n" .
					"}\n\n");
	}

	function create_source()
	{
		/* Rename to php_gtk_gen.c */
		if ($fp = fopen("../src/php_gtk_gen.c", "w")) {
			fwrite($fp, "\n#include \"php_gtk.h\"\n");
			fwrite($fp, "\n#if HAVE_PHP_GTK\n");
			fwrite($fp, "\n#include \"php_gtk_gen.h\"\n");
			$this->write_constants($fp);
			$this->write_static_vars($fp);
			$this->write_functions($fp);
			$this->write_no_constructor($fp);
			$this->write_objects($fp);
			fwrite($fp, $this->register_classes . "}\n");
			fwrite($fp, "\n#endif HAVE_PHP_GTK\n");
			fclose($fp);
		}

		if ($fp = fopen("../src/php_gtk_gen.h", "w")) {
			fwrite($fp, implode('', $this->byref_def));
			fclose($fp);
		}
	}
}

if ($argc < 2)
	die("usage: php -q generator.php defsfile [overridesfile [prefix]]\n");

if ($argc > 2) {
	$overrides = new Overrides($argv[2]);
	if ($argc > 3)
		$prefix = $arvv[3];
	else
		$prefix = 'php_gtk';
}
else
	$overrides = new Overrides();

$parser = new Defs_Parser($argv[1]);
$generator = new Generator($parser, $overrides, $prefix);
$parser->start_parsing();
$generator->register_types();
$generator->create_source();

?>
