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

/*
 * Significant portions of this generator are based on the pygtk code generator
 * developed by James Henstridge <jamesh@daa.com.au>.
 *
 */

set_time_limit(300);

// override the default PHP 8Mb as this script tends to use alot more
// and hopefully reduce the support questions a bit..
ini_set('memory_limit','64M');

require "Getopt.php";
require "arg_types.php";
require "override.php";
require "scheme.php";
require "templates.php";

class Generator {
    var $parser             = null;
    var $overrides          = null;
    var $prefix             = null;
    var $lprefix            = null;
    var $function_class     = null;

    var $is_gtk_object      = array('GtkObject' => true);

    var $constants          = '';
    var $register_classes   = '';

    var $functions_decl_end = "\t{NULL, NULL, NULL}\n};\n\n";

    function Generator(&$parser, &$overrides, $prefix, $function_class)
    {
        $this->parser    = &$parser;
        $this->overrides = &$overrides;
        $this->prefix    = $prefix;
        $this->lprefix   = strtolower($prefix);
        $this->function_class = $function_class;
    }
    
    function register_types($parser = null)
    {
        global  $matcher;

        if (!$parser)
            $parser = $this->parser;

        foreach ($parser->structs as $struct)
            $matcher->register_struct($struct->c_name);

        foreach ($parser->objects as $object) {
            $gtk_obj_descendant = false;
            if (isset($object->parent) &&
                isset($this->is_gtk_object[$object->parent[1] . $object->parent[0]])) {
                $this->is_gtk_object[$object->in_module . $object->name] = $this->is_gtk_object[$object->parent[1] . $object->parent[0]];
                $gtk_obj_descendant = true;
            }

            $matcher->register_object($object->c_name, $gtk_obj_descendant);
        }

        foreach ($parser->enums as $enum) {
            if ($enum->def_type == 'flags')
                $matcher->register_flag($enum->c_name);
            else
                $matcher->register_enum($enum->c_name, $enum->simple);
        }
    }

    function write_constants($fp)
    {
        fwrite($fp, "\nvoid php_". $this->lprefix . "_register_constants(int module_number TSRMLS_DC)\n");
        fwrite($fp, "{\n");
        foreach ($this->parser->enums as $enum) {
            if ($this->overrides->is_ignored($enum->c_name)) continue;
            fwrite($fp, "\n /* " . $enum->def_type . " " . $enum->c_name . " */\n");
            foreach ($enum->values as $enum_value) {
                fwrite($fp, "   REGISTER_LONG_CONSTANT(\"$enum_value[1]\", $enum_value[1], CONST_CS | CONST_PERSISTENT);\n");
            }
        }
        fwrite($fp, $this->overrides->get_constants());
        fwrite($fp, "}\n\n");
    }

    function write_method($fp, $obj_name, $gtk_object_descendant, $method)
    {
        global  $matcher,
                $method1_call_tpl,
                $method2_call_tpl,
                $method_tpl;

        $specs      = '';
        $var_list   = new Var_List();
        $parse_list = array('');
        $arg_list   = array('');
        $extra_pre_code = array();
        $extra_post_code = array();

        if ($method->varargs)
            trigger_error("varargs functions not supported", E_USER_ERROR);

        foreach ($method->params as $params_array) {
            list($param_type, $param_name, $param_default, $param_null) = $params_array;

            if (isset($param_default) && strpos($specs, '|') === false)
                $specs .= '|';

            $handler = &$matcher->get($param_type);
            if ($handler === null) {
                error_log("Could not write method $obj_name::$method->name (parameter type $param_type)");
                return;
            }
            $specs .= $handler->write_param($param_type,
                                            $param_name,
                                            $param_default,
                                            $param_null,
                                            $var_list,
                                            $parse_list,
                                            $arg_list,
                                            $extra_pre_code,
                                            $extra_post_code,
                                            false);
        }

        $arg_list   = implode(', ', $arg_list);
        $parse_list = implode(', ', $parse_list);
        $extra_pre_code = implode('', $extra_pre_code);
        $extra_post_code = implode('', $extra_post_code);

        if ($gtk_object_descendant)
            $method_call = sprintf($method1_call_tpl,
                                   $method->c_name,
                                   substr(convert_typename($obj_name), 1),
                                   $arg_list);
        else
            $method_call = sprintf($method2_call_tpl,
                                   $method->c_name,
                                   substr(convert_typename($obj_name), 1),
                                   $arg_list);
        $ret_handler = &$matcher->get($method->return_type);
        if ($ret_handler === null) {
            error_log("Could not write method $obj_name::$method->name (return type $method->return_type)");
            return false;
        }
        $return_tpl = $ret_handler->write_return($method->return_type, $var_list, true);
        $return_code = sprintf($return_tpl,
                               $method_call,
                               $extra_post_code);
        
        $method_code = sprintf($method_tpl,
                               strtolower($method->c_name),
                               $var_list->to_string(),
                               $specs,
                               $parse_list,
                               $extra_pre_code,
                               $return_code);

        fwrite($fp, $method_code);

        return true;
    }

    function write_constructor($fp, $obj_name, $gtk_object_descendant, $constructor)
    {
        global  $matcher,
                $constructor_tpl,
                $gtk_object_init_tpl,
                $non_gtk_object_init_tpl;

        $specs      = '';
        $var_list   = new Var_List();
        $parse_list = array('');
        $arg_list   = array();
        $extra_pre_code = array();
        $extra_post_code = array();

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
                                            $extra_pre_code,
                                            $extra_post_code,
                                            true);
        }

        $arg_list   = implode(', ', $arg_list);
        $parse_list = implode(', ', $parse_list);
        $extra_pre_code = implode('', $extra_pre_code);
        $extra_post_code = implode('', $extra_post_code);

        if ($gtk_object_descendant) {
            $cast = 'GtkObject';
            $var_list->add('GtkObject', '*wrapped_obj');
            $obj_init_code = $gtk_object_init_tpl;
        } else {
            $cast = $obj_name;
            $var_list->add($obj_name, '*wrapped_obj');
            $obj_init_code = sprintf($non_gtk_object_init_tpl,
                                    strtolower(substr(convert_typename($obj_name), 1)));
        }

        $constructor_code = sprintf($constructor_tpl,
                                    strtolower($constructor->c_name),
                                    $var_list->to_string(),
                                    $specs,
                                    $parse_list,
                                    $extra_pre_code,
                                    $cast,
                                    $constructor->c_name,
                                    $arg_list,
                                    $obj_name,
                                    $obj_init_code,
                                    $extra_post_code);

        fwrite($fp, $constructor_code);
        
        return true;
    }

    function write_function($fp, $function)
    {
        global  $matcher,
                $function_call_tpl,
                $function_tpl;

        $specs      = '';
        $var_list   = new Var_List();
        $parse_list = array('');
        $arg_list   = array();
        $extra_pre_code = array();
        $extra_post_code = array();

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
                                            $extra_pre_code,
                                            $extra_post_code,
                                            false);
        }

        $arg_list   = implode(', ', $arg_list);
        $parse_list = implode(', ', $parse_list);
        $extra_pre_code = implode('', $extra_pre_code);
        $extra_post_code = implode('', $extra_post_code);

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
                               $function_call,
                               $extra_post_code);

        // hack to avoid macro conflict on win32
        $extra_name = "";
        if (strtolower($function->c_name) == "gdk_draw_pixmap")
            $extra_name = "_int";

        $function_code = sprintf($function_tpl,
                                 strtolower($function->c_name) . $extra_name,
                                 $var_list->to_string(),
                                 $specs,
                                 $parse_list,
                                 $extra_pre_code,
                                 $return_code);

        fwrite($fp, $function_code);

        return true;
    }

    function write_prop_getter($fp, $object)
    {
        global  $matcher,
                $prop_check_tpl,
                $prop_getter_tpl;

        $obj_cast = substr(convert_typename($object->c_name), 1);
        $prop_checks = '';
        $else_clause = "\t";

        foreach ($object->fields as $field_def) {
            list($field_type, $field_name) = $field_def;
            //
            // HACK: Dont generate properties not defined on Win32
            //
            if (strtoupper(substr(PHP_OS, 0, 3)) == 'WIN' && $field_name == "gutter_size") {
                error_log("gutter_size is not defined on Win32");
                continue;
            }
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

    function write_structs($fp)
    {
        global  $struct_class_tpl,
                $struct_init_tpl,
                $struct_construct_tpl,
                $struct_get_tpl,
                $register_class_tpl,
                $functions_decl_tpl,
                $function_entry_tpl,
                $matcher;

        foreach ($this->parser->structs as $struct) {
            $struct_module = strtolower($struct->in_module);
            $struct_lname = strtolower(substr(convert_typename($struct->name),1));
            fwrite($fp, "\n/* struct $struct->c_name */\n");

            /*
            $this->register_classes .= sprintf($struct_class_tpl,
                                               $struct->in_module . $struct->name,
                                               $struct_module . '_' . $struct_lname);
            */
            $this->register_classes .= sprintf($register_class_tpl,
                                               $struct->ce,
                                               $struct->in_module . $struct->name,
                                               $struct_module . '_' . $struct_lname,
                                               'NULL', 0, 'NULL');

            $init_prop_code = array();
            $construct_prop_code = array();
            $get_prop_code = array();
            $specs      = '';
            $var_list   = new Var_List();
            $parse_list = array('');
            $arg_list   = array();
            $extra_pre_code = array();
            $extra_post_code = array();

            foreach ($struct->fields as $field_def) {
                list($field_type, $field_name) = $field_def;
                $handler = &$matcher->get($field_type);
                if ($handler === null) {
                    error_log("Could not write struct $struct->c_name '$field_name' field (field type $field_type)");
                    return;
                }

                $init_prop_code[] = $handler->write_to_prop('result', $field_name, "obj->$field_name");
                $get_prop_code[] = $handler->write_from_prop($field_name, $field_type);
                $construct_prop_code[] = $handler->write_to_prop('this_ptr', $field_name, $field_name);

                $specs .= $handler->write_param($field_type,
                                                $field_name,
                                                null,
                                                false,
                                                $var_list,
                                                $parse_list,
                                                $arg_list,
                                                $extra_pre_code,
                                                $extra_post_code,
                                                true);
            }

            /* write the initializer */
            fwrite($fp, sprintf($struct_init_tpl,
                                $struct_module . '_' . $struct_lname,
                                $struct->in_module . $struct->name,
                                $struct->ce, implode('', $init_prop_code)));
            
            /* write getter */
            fwrite($fp, sprintf($struct_get_tpl,
                                $struct_module . '_' . $struct_lname,
                                $struct->in_module . $struct->name,
                                $struct->ce, implode('', $get_prop_code)));

            /* write the constructor */
            fwrite($fp, sprintf($struct_construct_tpl,
                                strtolower($struct->c_name),
                                $var_list->to_string(),
                                $specs,
                                implode(', ', $parse_list),
                                implode('', $construct_prop_code)));
            $functions_decl = sprintf($functions_decl_tpl, $struct_module . '_' . $struct_lname);
            $functions_decl .= sprintf($function_entry_tpl,
                                       $struct->in_module . $struct->name,
                                       strtolower($struct->c_name),
                                       'NULL');
            /*
            $functions_decl .= sprintf($function_entry_tpl,
                                       $struct_module . $struct_lname,
                                       strtolower($struct->c_name),
                                       'NULL');
            */
            $functions_decl .= $this->functions_decl_end;
            fwrite($fp, $functions_decl);
        }
    }

    function write_objects($fp)
    {
        global  $function_entry_tpl,
                $functions_decl_tpl,
                $register_getter_tpl,
                $init_class_tpl,
                $register_class_tpl,
                $get_type_tpl;
        
        foreach ($this->parser->objects as $object) {
            $object_module = strtolower($object->in_module);
            $object_lname = strtolower($object->name);

            fwrite($fp, "\n/* object $object->c_name  */\n");
            /*
            $this->register_classes .= sprintf($init_class_tpl,
                                               $object->in_module . $object->name,
                                               $object_module . '_' . $object_lname,
                                               count($object->fields) ? 'php_gtk_get_property' : 'NULL');
            */
            if ($object->parent === null)
                $this->register_classes .= sprintf($register_class_tpl,
                                                   $object->ce,
                                                   $object->in_module . $object->name,
                                                   $object_module . '_' . $object_lname,
                                                  'NULL',
                                                  count($object->fields) ? 1 : 0,
                                                  count($object->fields) ? 'php_' . $object_module . '_' . $object_lname . '_properties' : 'NULL');
            else {
                if ($object->parent[1] === null)
                    $parent_ce = strtolower($object->parent[0]) . '_ce';
                else
                    $parent_ce = strtolower($object->parent[1] . '_' . $object->parent[0]) . '_ce';
                $this->register_classes .= sprintf($register_class_tpl,
                                                   $object->ce,
                                                   $object->in_module . $object->name,
                                                   $object_module . '_' . $object_lname,
                                                   $parent_ce,
                                                   count($object->fields) ? '1' : '0',
                                                   count($object->fields) ? 'php_' . $object_module . '_' . $object_lname . '_properties' : 'NULL');
                $this->register_classes .= "\tg_hash_table_insert(php_gtk_class_hash, g_strdup(\"Gtk$object->name\"), $object->ce);\n";
            }

            if (count($object->fields)) {
                $this->register_classes .= sprintf($register_getter_tpl, $object->ce, $object_module . '_' . $object_lname);
            }

            $functions_decl = sprintf($functions_decl_tpl, $object_module . '_' . $object_lname);
            $constructor = $this->parser->find_constructor($object);
            if (!$constructor || $this->overrides->is_ignored($constructor->c_name)) {
                $functions_decl .= sprintf($function_entry_tpl,
                                           $object->in_module . $object->name,
                                           'no_constructor', 'NULL');
                /*
                $functions_decl .= sprintf($function_entry_tpl,
                                           $object_module . $object_lname,
                                           'no_constructor', 'NULL');
                */
            } else if ($this->overrides->is_overriden($constructor->c_name)) {
                list(, $constructor_override) = $this->overrides->get_override($constructor->c_name);
                fwrite($fp, $constructor_override . "\n");
                $functions_decl .= sprintf($function_entry_tpl,
                                           $constructor->is_constructor_of,
                                           strtolower($constructor->c_name),
                                           'NULL');
                /*
                $functions_decl .= sprintf($function_entry_tpl,
                                           strtolower($constructor->is_constructor_of),
                                           strtolower($constructor->c_name),
                                           'NULL');
                */
            } else if (!$this->overrides->is_ignored($constructor->c_name)) {
                if ($this->write_constructor($fp, $object->c_name, $this->is_gtk_object[$object->in_module . $object->name], $constructor)) {
                    $functions_decl .= sprintf($function_entry_tpl,
                                               $constructor->is_constructor_of,
                                               strtolower($constructor->c_name),
                                               'NULL');
                    /*
                    $functions_decl .= sprintf($function_entry_tpl,
                                               strtolower($constructor->is_constructor_of),
                                               strtolower($constructor->c_name),
                                               'NULL');
                    */
                } else {
                    $functions_decl .= sprintf($function_entry_tpl,
                                               $constructor->is_constructor_of,
                                               'no_constructor', 'NULL');
                    /*
                    $functions_decl .= sprintf($function_entry_tpl,
                                               strtolower($constructor->is_constructor_of),
                                               'no_constructor', 'NULL');
                    */
                }
            }

            /*
             * Insert get_type() as class method if the object is descended from
             * GtkObject.
             */
            if ($this->is_gtk_object[$object->in_module . $object->name]) {
                $lclass = strtolower(substr(convert_typename($object->c_name), 1));
                $functions_decl .= sprintf($function_entry_tpl,
                                           'get_type',
                                           $lclass .  '_get_type',
                                           'NULL');
                fwrite($fp, sprintf($get_type_tpl, $lclass, $lclass));
            }

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
                    if ($this->write_method($fp, $object->c_name,
                                            $this->is_gtk_object[$object->in_module . $object->name], $method)) {
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

            if ($object->c_name == $this->function_class) {
                $this->write_functions($fp, false, $functions_decl);
            }

            $functions_decl .= $this->functions_decl_end;
            fwrite($fp, $functions_decl);

            if (count($object->fields))
                $this->write_prop_getter($fp, $object);
        }

        $this->register_classes .= "\n" . implode("\n", $this->overrides->get_register_classes());
    }

    function write_functions($fp, $separate_class, &$functions_decl)
    {
        global  $function_entry_tpl,
                $functions_decl_tpl,
                $class_entry_tpl,
                $init_class_tpl,
                $register_class_tpl;
        
        $num_functions = 0;
        if ($separate_class)
            $functions_decl = sprintf($functions_decl_tpl, $this->lprefix);

        foreach ($this->parser->functions as $function) {
            if ($this->overrides->is_overriden($function->c_name)) {
                list($function_name, $function_override) = $this->overrides->get_override($function->c_name);
                fwrite($fp, $function_override . "\n");
                if ($function->name == $function->c_name)
                    $function_name = substr($function->name, strlen($this->lprefix) + 1);
                else
                    $function_name = $function->name;
                $functions_decl .= sprintf($function_entry_tpl, 
                                           strtolower($function_name), 
                                           strtolower($function->c_name), 
                                           'NULL');
                $num_functions++;
            }
            else if (!$this->overrides->is_ignored($function->c_name)) {
                if ($this->write_function($fp, $function)) {
                    if ($function->name == $function->c_name)
                        $function_name = substr($function->name, strlen($this->lprefix) + 1);
                    else
                        $function_name = $function->name;

                    // hack to avoid macro conflict on win32
                    $extra_name = "";
                    if (strtolower($function->c_name) == "gdk_draw_pixmap")
                        $extra_name = "_int";

                    $functions_decl .= sprintf($function_entry_tpl, 
                                               $function_name, 
                                               strtolower($function->c_name) . $extra_name, 
                                               'NULL');
                    $num_functions++;
                }
            }
        }
        if ($separate_class)
            $functions_decl .= $this->functions_decl_end;

        if ($num_functions > 0) {
            if ($separate_class) {
                /*
                $this->register_classes .= sprintf($init_class_tpl,
                                                   $this->prefix,
                                                   $this->lprefix,
                                                   'NULL');
                */
                $this->register_classes .= sprintf($register_class_tpl,
                                                   $this->lprefix . '_ce',
                                                   $this->prefix,
                                                   $this->lprefix,
                                                   'NULL', 0, 'NULL');
                fwrite($fp, sprintf($class_entry_tpl, $this->lprefix . '_ce'));
                fwrite($fp, $functions_decl);
            }
        }
    }

    function write_prop_lists($fp)
    {
        global  $class_prop_list_header,
                $class_prop_list_footer;
        
        fwrite($fp, "\n");
        foreach ($this->parser->objects as $object) {
            if (count($object->fields) == 0) continue;

            fwrite($fp, sprintf($class_prop_list_header,
                                strtolower($object->in_module) . '_' .
                                strtolower($object->name)));
            foreach ($object->fields as $field_def) {
                list(, $field_name) = $field_def;
                fwrite($fp, "\t\"$field_name\",\n");
            }
            fwrite($fp, $class_prop_list_footer);
        }
    }

    function write_class_entries($fp)
    {
        global  $class_entry_tpl;

        foreach ($this->parser->structs as $struct)
            fwrite($fp, sprintf($class_entry_tpl, $struct->ce));
        foreach ($this->parser->objects as $object)
            fwrite($fp, sprintf($class_entry_tpl, $object->ce));
        foreach ($this->overrides->get_register_classes() as $ce => $rest)
            fwrite($fp, sprintf($class_entry_tpl, $ce));
    }

    function create_source()
    {
        global  $register_classes_tpl;

        $fp = fopen('php://stdout', 'w');
        fwrite($fp, "#include \"php_gtk.h\"");
        fwrite($fp, "\n#if HAVE_PHP_GTK\n");
        fwrite($fp, $this->overrides->get_headers());
        $this->write_constants($fp);
        $this->write_class_entries($fp);
        $this->write_prop_lists($fp);
        if (!isset($this->parser->objects[$this->function_class]))
            $this->write_functions($fp, true, $dummy);
        $this->write_structs($fp);
        $this->write_objects($fp);
        fwrite($fp, sprintf($register_classes_tpl,
                            $this->lprefix,
                            $this->register_classes));
        fwrite($fp, "\n#endif /* HAVE_PHP_GTK */\n");
        fclose($fp);
    }
}

/* simple fatal_error function 
        - useful in 4.2.0-dev and later as it will actually halt exectution of make
    - needs to output to stderr as the default print would end up inside the code
    - TODO - the make file outputs an empty file to gen_XXX.c, there must be a
      better way  to delete it... - by using output buffering or somthing?
*/  
function fatal_error($message) {
    $fh = fopen("php://stderr", "w");
        fwrite($fh,"\n\n\n\n
========================================================================    
    There was a Serious error with the PHP-GTK generator script
========================================================================    
    $message
========================================================================    
    You should type this to ensure that the c source is correctly
    generated before attempting to make again.
    
    #find . | grep defs | xargs touch

    \n\n\n\n");
    fclose($fh);
    exit(1);
}   


$old_error_reporting = error_reporting(E_ALL & ~E_NOTICE);

/* For backwards compatibility. */
chdir(dirname(__FILE__));

if (!isset($_SERVER['argv'])) 
    fatal_error("
        Could not read command line arguments for generator.php 
        Please ensure that this option is set in your php.ini
        register_argc_argv = On     
    ");


if (isset($_SERVER['argc']) &&
    isset($_SERVER['argv'])) {
    $argc = $_SERVER['argc'];
    $argv = $_SERVER['argv'];
}

/* An ugly hack to counteract PHP's pernicious desire to treat + as an argument
   separator in command-line version. */
array_walk($argv, create_function('&$x', '$x = urldecode($x);'));

    
$result = Console_Getopt::getopt($argv, 'o:p:c:r:');
if (!$result || count($result[1]) < 2)
    fatal_error("usage: php -q generator.php [-o overridesfile] [-p prefix] [-c functionclass ] [-r typesfile] defsfile\n");

list($opts, $argv) = $result;

chdir('..');

$prefix = 'Gtk';
$function_class = null;
$overrides = new Overrides();
$register_defs = array();

foreach ($opts as $opt) {
    list($opt_spec, $opt_arg) = $opt;
    if ($opt_spec == 'o') {
        $overrides = new Overrides($opt_arg);
    } else if ($opt_spec == 'p') {
        $prefix = $opt_arg;
    } else if ($opt_spec == 'c') {
        $function_class = $opt_arg;
    } else if ($opt_spec == 'r') {
        $register_defs[] = $opt_arg;
    }
}

if (file_exists(dirname($argv[1]) . '/arg_types.php')) {
    include(dirname($argv[1]) . '/arg_types.php');
}

$parser = new Defs_Parser($argv[1]);
$generator = new Generator($parser, $overrides, $prefix, $function_class);
foreach ($register_defs as $defs) {
    $type_parser = new Defs_Parser($defs);
    $type_parser->start_parsing();
    $generator->register_types($type_parser);
}
$parser->start_parsing();
$generator->register_types();
$generator->create_source();

error_reporting($old_error_reporting);

/* vim: set et sts=4: */
?>
