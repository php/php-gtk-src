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

/*
 * Significant portions of this generator are based on the pygtk code generator
 * developed by James Henstridge <jamesh@daa.com.au>.
 *
 */

set_time_limit(300);

require "Getopt.php";
require "arg_types.php";
require "override.php";
require "scheme.php";
require "templates.php";

class Generator {
	var $parser 	= null;
	var $overrides  = null;
	var $prefix		= null;

	var $constants	= '';
	var $register_classes = '';

	var $functions_decl_end = "\t{NULL, NULL, NULL}\n};\n\n";

	function Generator(&$parser, &$overrides, $prefix)
	{
		$this->parser 	 = &$parser;
		$this->overrides = &$overrides;
		$this->prefix	 = $prefix;
	}
	
	function register_types($parser = null)
	{
		global	$matcher;

		if (!$parser)
			$parser = $this->parser;

		foreach ($parser->objects as $object)
			$matcher->register_object($object->c_name);

		foreach ($parser->enums as $enum) {
			if ($enum->def_type == 'flags')
				$matcher->register_flag($enum->c_name);
			else
				$matcher->register_enum($enum->c_name);
		}
	}

	function write_constants($fp)
	{
		fwrite($fp, "\nvoid php_". $this->prefix . "_register_constants(int module_number ELS_DC)\n");
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
			error_log("Could not write method $method->name (return type $method->return_type)");
			return false;
		}
		$return_tpl = $ret_handler->write_return($method->return_type, $var_list, true);
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

		return true;
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

		foreach ($constructor->params as $params_array) {
			list($param_type, $param_name, $param_default, $param_null) = $params_array;

			if (isset($param_default) && strpos($specs, '|') === false)
				$specs .= '|';

			$handler = &$matcher->get($param_type);
			if ($handler === null) {
				error_log("Could not write constructor $constructor->name (parameter type $param_type)");
				return false;
			}
			$specs .= $handler->write_param($param_type,
											$param_name,
											$param_default,
											$param_null,
											$var_list,
											$parse_list,
											$arg_list,
											$extra_code,
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
		
		return true;
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

		if ($function->varargs)
			trigger_error("varargs functions not supported", E_USER_ERROR);

		foreach ($function->params as $params_array) {
			list($param_type, $param_name, $param_default, $param_null) = $params_array;

			if (isset($param_default) && strpos($specs, '|') === false)
				$specs .= '|';

			$handler = &$matcher->get($param_type);
			if ($handler === null) {
				error_log("Could not write function $function->name (parameter type $param_type)");
				return false;
			}
			$specs .= $handler->write_param($param_type,
											$param_name,
											$param_default,
											$param_null,
											$var_list,
											$parse_list,
											$arg_list,
											$extra_code,
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
		$return_tpl = $ret_handler->write_return($function->return_type, $var_list, true);
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

		return true;
	}

	function write_prop_getter($fp, $object)
	{
		global	$matcher,
				$prop_check_tpl,
				$prop_getter_tpl;

		$obj_cast = substr(convert_typename($object->c_name), 1);
		$prop_checks = '';
		$else_clause = "\t";

		foreach ($object->fields as $field_def) {
			list($field_type, $field_name) = $field_def;
			if ($this->overrides->have_get_prop($object->c_name, $field_name)) {
				$prop_get_code = $this->overrides->get_prop($object->c_name, $field_name);
				$prop_get_code .= "\n\n\treturn;";
				$prop_get_code = str_replace("\n", "\n\t", $prop_get_code);
			} else {
				$var_list = new Var_List();
				$handler = &$matcher->get($field_type);
				if ($handler === null) {
					error_log("Could not write getter for $object->c_name '$field_name' field (field type $field_type)");
					continue;
				}

				$prop_tpl = $handler->write_return($field_type, $var_list, false);
				$prop_code = sprintf($prop_tpl,
									 $obj_cast . '(PHP_GTK_GET(object))->' . $field_name, "");
				$prop_code = str_replace("\n", "\n\t", $prop_code);
				$var_list_code = $var_list->to_string();
				$prop_get_code = ($var_list_code ? $var_list_code . "\t" : '') .  ' ' . $prop_code;
			}
			$prop_checks .= sprintf($prop_check_tpl,
									$else_clause,
									$field_name,
									$prop_get_code);
			$else_clause = ' else ';
		}

		fwrite($fp, sprintf($prop_getter_tpl,
							strtolower($object->in_module . '_' .  $object->name),
							$prop_checks));
	}

	function write_objects($fp)
	{
		global	$function_entry_tpl,
				$functions_decl_tpl,
				$register_getter_tpl,
				$init_class_tpl,
				$register_class_tpl,
				$get_type_tpl;
		
		foreach ($this->parser->objects as $object) {
			$object_module = strtolower($object->in_module);
			$object_lname = strtolower($object->name);

			fwrite($fp, "\n/* object $object->c_name  */\n");
			$this->register_classes .= sprintf($init_class_tpl,
											   $object_module . $object_lname,
											   $object_module . '_' . $object_lname,
											   count($object->fields) ? 'php_gtk_get_property' : 'NULL');
			if ($object->parent === null)
				$this->register_classes .= sprintf($register_class_tpl,
												   $object->ce,
												  'NULL');
			else {
				if ($object->parent[1] === null)
					$parent_ce = strtolower($object->parent[0]) . '_ce';
				else
					$parent_ce = strtolower($object->parent[1] . '_' . $object->parent[0]) . '_ce';
				$this->register_classes .= sprintf($register_class_tpl,
												   $object->ce,
												   $parent_ce);
				$this->register_classes .= "\tg_hash_table_insert(php_gtk_class_hash, g_strdup(\"Gtk$object->name\"), $object->ce);\n";
			}

			if (count($object->fields)) {
				$this->register_classes .= sprintf($register_getter_tpl, $object->ce, $object_module . '_' . $object_lname);
			}

			$functions_decl = sprintf($functions_decl_tpl, $object_module . '_' . $object_lname);
			$constructor = $this->parser->find_constructor($object);
			if (!$constructor || $this->overrides->is_ignored($constructor->c_name)) {
				$functions_decl .= sprintf($function_entry_tpl,
										   strtolower($object->c_name),
										   'no_constructor', 'NULL');
			} else if ($this->overrides->is_overriden($constructor->c_name)) {
				list(, $constructor_override) = $this->overrides->get_override($constructor->c_name);
				fwrite($fp, $constructor_override . "\n");
				$functions_decl .= sprintf($function_entry_tpl,
										   strtolower($constructor->is_constructor_of),
										   strtolower($constructor->c_name),
										   'NULL');
			} else if (!$this->overrides->is_ignored($constructor->c_name)) {
				if ($this->write_constructor($fp, $object->c_name, $constructor)) {
					$functions_decl .= sprintf($function_entry_tpl,
											   strtolower($constructor->is_constructor_of),
											   strtolower($constructor->c_name),
											   'NULL');
				} else {
					$functions_decl .= sprintf($function_entry_tpl,
											   strtolower($constructor->is_constructor_of),
											   'no_constructor', 'NULL');
				}
			}

			/* insert get_type() as class method */
			$lclass = strtolower(substr(convert_typename($object->c_name), 1));
			$functions_decl .= sprintf($function_entry_tpl,
									   'get_type',
									   $lclass .  '_get_type',
									   'NULL');
			fwrite($fp, sprintf($get_type_tpl, $lclass, $lclass));

			foreach ($this->parser->find_methods($object) as $method) {
				if ($this->overrides->is_overriden($method->c_name)) {
					list($method_name, $method_override) = $this->overrides->get_override($method->c_name);
					fwrite($fp, $method_override . "\n");
					if (!isset($method_name))
						$method_name = $method->name;
					$functions_decl .= sprintf($function_entry_tpl,
											   strtolower($method->name),
											   strtolower($method->c_name),
											   'NULL');
				}
				else if (!$this->overrides->is_ignored($method->c_name)) {
					if ($this->write_method($fp, $object->c_name, $method)) {
						$functions_decl .= sprintf($function_entry_tpl,
												   strtolower($method->name),
												   strtolower($method->c_name),
												   'NULL');
					}
				}

			}

			if (isset($this->overrides->extra_methods[$object->c_name])) {
				foreach ($this->overrides->extra_methods[$object->c_name] as $method_cname => $method_data) {
					list($method_name, $method_override) = $method_data;
					fwrite($fp, $method_override . "\n");
					if (!isset($method_name))
						$method_name = $method_cname;
					$functions_decl .= sprintf($function_entry_tpl,
											   strtolower($method_name),
											   strtolower($method_cname),
											   'NULL');
				}
			}

			$functions_decl .= $this->functions_decl_end;
			fwrite($fp, $functions_decl);

			if (count($object->fields))
				$this->write_prop_getter($fp, $object);
		}

		$this->register_classes .= "\n" . implode("\n", $this->overrides->get_register_classes());
	}

	function write_functions($fp)
	{
		global	$function_entry_tpl,
				$functions_decl_tpl,
				$init_class_tpl,
				$register_class_tpl;
		
		$this->register_classes .= sprintf($init_class_tpl,
										   $this->prefix,
										   $this->prefix,
										   'NULL');
		$this->register_classes .= sprintf($register_class_tpl,
										   $this->prefix . '_ce',
										   'NULL');
		$functions_decl = sprintf($functions_decl_tpl, $this->prefix);

		foreach ($this->parser->functions as $function) {
			if ($this->overrides->is_overriden($function->c_name)) {
				list($function_name, $function_override) = $this->overrides->get_override($function->c_name);
				fwrite($fp, $function_override . "\n");
				if ($function->name == $function->c_name)
					$function_name = substr($function->name, strlen($this->prefix) + 1);
				else
					$function_name = $function->name;
				$functions_decl .= sprintf($function_entry_tpl, 
										   strtolower($function_name), 
										   strtolower($function->c_name), 
										   'NULL');
			}
			else if (!$this->overrides->is_ignored($function->c_name)) {
				if ($this->write_function($fp, $function)) {
					if ($function->name == $function->c_name)
						$function_name = substr($function->name, strlen($this->prefix) + 1);
					else
						$function_name = $function->name;
					$functions_decl .= sprintf($function_entry_tpl, 
											   $function_name, 
											   strtolower($function->c_name), 
											   'NULL');
				}
			}
		}
		$functions_decl .= $this->functions_decl_end;
		fwrite($fp, $functions_decl);
	}

	function write_class_entries($fp)
	{
		global	$class_entry_tpl;

		fwrite($fp, sprintf($class_entry_tpl, $this->prefix . '_ce'));
		foreach ($this->parser->objects as $object)
			fwrite($fp, sprintf($class_entry_tpl, $object->ce));
		foreach ($this->overrides->get_register_classes() as $ce => $rest)
			fwrite($fp, sprintf($class_entry_tpl, $ce));
	}

	function create_source()
	{
		global	$register_classes_tpl;

		$fp = fopen('../src/php_gtk_gen_' . $this->prefix . '.c', 'w');
		fwrite($fp, "#include \"php_gtk.h\"");
		fwrite($fp, "\n#if HAVE_PHP_GTK\n");
		fwrite($fp, $this->overrides->get_headers());
		$this->write_constants($fp);
		$this->write_class_entries($fp);
		$this->write_functions($fp);
		$this->write_objects($fp);
		fwrite($fp, sprintf($register_classes_tpl,
							$this->prefix,
							$this->register_classes));
		fwrite($fp, "\n#endif /* HAVE_PHP_GTK */\n");
		fclose($fp);
	}
}

$argc = $HTTP_SERVER_VARS['argc'];
$argv = $HTTP_SERVER_VARS['argv'];

$result = Console_Getopt::getopt($argv, 'o:p:r:');
if (!$result || count($result[1]) < 2)
	die("usage: php -q generator.php [-o overridesfile] [-p prefix] [-r typesfile] defsfile\n");

list($opts, $argv) = $result;

$prefix = 'gtk';
$overrides = new Overrides();

foreach ($opts as $opt) {
	list($opt_spec, $opt_arg) = $opt;
	if ($opt_spec == 'o') {
		$overrides = new Overrides($opt_arg);
	} else if ($opt_spec == 'p') {
		$prefix = $opt_arg;
	} else if ($opt_spec == 'r') {
		$type_parser = new Defs_Parser($opt_arg);
		$type_parser->start_parsing();
		Generator::register_types($type_parser);
	}
}

$parser = new Defs_Parser($argv[1]);
$generator = new Generator($parser, $overrides, $prefix);
$parser->start_parsing();
$generator->register_types();
$generator->create_source();

?>
