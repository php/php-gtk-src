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
require "override.php";
require "arg_types.php";
require "scheme.php";
require "templates.php";
require "array_printf.php";

class Generator {
    var $parser             = null;
    var $overrides          = null;
    var $prefix             = null;
    var $lprefix            = null;
    var $function_class     = null;

    var $is_gtk_object      = array('GtkObject' => true);

    var $constants          = '';

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

        /*
        foreach ($parser->structs as $struct)
            $matcher->register_struct($struct->c_name);
            */

        foreach ($parser->objects as $object) {
            /*
            $gtk_obj_descendant = false;
            if (isset($object->parent) &&
                isset($this->is_gtk_object[$object->parent[1] . $object->parent[0]])) {
                $this->is_gtk_object[$object->in_module . $object->name] = $this->is_gtk_object[$object->parent[1] . $object->parent[0]];
                $gtk_obj_descendant = true;
            }
            */

            //TODO $matcher->register_object($object->c_name, $object->typecode);
        }

        foreach ($parser->enums as $enum) {
            if ($enum->def_type == 'flags')
                $matcher->register_flag($enum->c_name, $enum->typecode);
            else
                $matcher->register_enum($enum->c_name, $enum->typecode);
        }
    }

    function write_constants()
    {
        fwrite($this->fp, "\nvoid php_". $this->lprefix . "_register_constants(int module_number TSRMLS_DC)\n");
        fwrite($this->fp, "{\n");
        foreach ($this->parser->enums as $enum) {
            if ($this->overrides->is_ignored($enum->c_name)) continue;
            fwrite($this->fp, "\n /* " . $enum->def_type . " " . $enum->c_name . " */\n");
            foreach ($enum->values as $enum_value) {
                fwrite($this->fp, "   REGISTER_LONG_CONSTANT(\"$enum_value[1]\", $enum_value[1], CONST_CS | CONST_PERSISTENT);\n");
            }
        }
        fwrite($this->fp, $this->overrides->get_constants());
        fwrite($this->fp, "}\n\n");
    }

    function write_method($obj_name, $gtk_object_descendant, $method)
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

        fwrite($this->fp, $method_code);

        return true;
    }

    function write_constructor($obj_name, $gtk_object_descendant, $constructor)
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

        fwrite($this->fp, $constructor_code);
        
        return true;
    }

    function write_callable($callable, $template, $handle_return = false, $is_method = false, $dict = array())
    {
        global $matcher;

        if ($callable->varargs) {
            throw new Exception('varargs methods not supported');
        }

        $info = new Wrapper_Info();

        /* need the extra comma for methods */
        if ($is_method) {
            $info->arg_list[] = '';
        }

        foreach ($callable->params as $params_array) {
            list($param_type, $param_name, $param_default, $param_null) = $params_array;

            if (isset($param_default) && strpos($info->specifiers, '|') === false) {
                $info->add_parse_list('|');
            }

            $handler = $matcher->get($param_type);
            $handler->write_param($param_type, $param_name, $param_default, $param_null, $info);
        }

        $dict['return'] = '';
        if ($handle_return) {
            if ($callable->return_type !== null &&
                $callable->return_type != 'none') {
                $dict['return'] = 'php_retval = ';
            }
            $handler = $matcher->get($callable->return_type);
            $handler->write_return($callable->return_type, $callable->caller_owns_return, $info);
        }

        /* TODO handle deprecation */

        if (!isset($dict['name'])) {
            $dict['name'] = $callable->name;
        }
        $dict['cname'] = $callable->c_name;
        $dict['var_list'] = $info->get_var_list();
        $dict['specs'] = $info->specifiers;
        $dict['parse_list'] = $info->get_parse_list();
        $dict['arg_list'] = $info->get_arg_list();
        $dict['pre_code'] = $info->get_pre_code();
        $dict['post_code'] = $info->get_post_code();

        return aprintf($template, $dict);
    }

    function write_function($function)
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

        fwrite($this->fp, $function_code);

        return true;
    }

    function write_prop_getter($object)
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

        fwrite($this->fp, sprintf($prop_getter_tpl,
                            strtolower($object->in_module . '_' .  $object->name),
                            $prop_checks));
    }

    function write_structs()
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
            fwrite($this->fp, "\n/* struct $struct->c_name */\n");

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
            fwrite($this->fp, sprintf($struct_init_tpl,
                                $struct_module . '_' . $struct_lname,
                                $struct->in_module . $struct->name,
                                $struct->ce, implode('', $init_prop_code)));
            
            /* write getter */
            fwrite($this->fp, sprintf($struct_get_tpl,
                                $struct_module . '_' . $struct_lname,
                                $struct->in_module . $struct->name,
                                $struct->ce, implode('', $get_prop_code)));

            /* write the constructor */
            fwrite($this->fp, sprintf($struct_construct_tpl,
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
            fwrite($this->fp, $functions_decl);
        }
    }

    function write_methods($object)
    {
        $method_defs = array();

        $methods = $this->parser->find_methods($object);
        if ($methods) {
            fprintf(STDERR, "Writing methods for $object->c_name...\n");
        }

        $dict['class'] = $object->c_name;
        $dict['cast'] = preg_replace('!_TYPE_!', '_', $object->typecode, 1);

        foreach ($methods as $method) {
            $method_name = $method->c_name;
            
            /* skip ignored methods */
            if ($this->overrides->is_ignored($method_name)) continue;

            try {
                if ($this->overrides->is_overriden($method_name)) {
                /* XXX fix
                    list($method_name, $method_override) = $this->overrides->get_override($method_name);
                    fwrite($this->fp, $method_override . "\n");
                    if (!isset($method_name))
                        $method_name = $method->name;
                    $functions_decl .= sprintf($function_entry_tpl,
                                               strtolower($method->name),
                                               strtolower($method->c_name),
                                               'NULL');
                                               */
                } else {
                    $code = $this->write_callable($method, Templates::method_body, true, true, $dict);
                    fwrite($this->fp, $code);
                    $method_defs[] = sprintf(Templates::function_entry,
                                             $object->in_module . $object->name,
                                             $method->name, 'NULL', 0);
                }
            } catch (Exception $e) {
                fprintf(STDERR, "\tnot generating method %s::%s: %s\n", $object->c_name, $method->name, $e->getMessage());
            }
        }

        if ($method_defs) {
            fwrite($this->fp, sprintf(Templates::functions_decl, strtolower($object->c_name)));
            fwrite($this->fp, join('', $method_defs));
            fwrite($this->fp, Templates::functions_decl_end);
        }

        return $method_defs;
    }

    function write_objects()
    {
        global  $function_entry_tpl,
                $functions_decl_tpl,
                $register_getter_tpl,
                $init_class_tpl,
                $register_class_tpl,
                $get_type_tpl;
        
        foreach ($this->parser->objects as $object) {
            fprintf(STDERR, $object->c_name. "\n");
            $object_module = strtolower($object->in_module);
            $object_lname = strtolower($object->name);

            fwrite($this->fp, "\n/* object $object->c_name  */\n");
            /*
            $this->register_classes .= sprintf($init_class_tpl,
                                               $object->in_module . $object->name,
                                               $object_module . '_' . $object_lname,
                                               count($object->fields) ? 'php_gtk_get_property' : 'NULL');
            */
            $this->register_classes .= sprintf($register_class_tpl,
                                               $object->ce,
                                               $object->in_module . $object->name,
                                               strtolower($object->c_name),
                                               $object->parent ? strtolower($object->parent) . '_ce' : 'NULL',
                                               $object->typecode);

            /* XXX fix
            if (count($object->fields)) {
                $this->register_classes .= sprintf($register_getter_tpl, $object->ce, $object_module . '_' . $object_lname);
            }
            */

            $functions_decl = sprintf($functions_decl_tpl, strtolower($object->c_name));
            $constructor = $this->parser->find_constructor($object);
            if (!$constructor || $this->overrides->is_ignored($constructor->c_name)) {
                /* XXX fix
                $functions_decl .= sprintf($function_entry_tpl,
                                           $object->in_module . $object->name,
                                           'no_constructor', 'NULL');
                                           */
            } else if ($this->overrides->is_overriden($constructor->c_name)) {
                /*
                list(, $constructor_override) = $this->overrides->get_override($constructor->c_name);
                fwrite($this->fp, $constructor_override . "\n");
                $functions_decl .= sprintf($function_entry_tpl,
                                           $constructor->is_constructor_of,
                                           strtolower($constructor->c_name),
                                           'NULL');
                                           */
            } else if (!$this->overrides->is_ignored($constructor->c_name)) {
                /* XXX fix */
                /*
                if ($this->write_constructor($this->fp, $object->c_name, $this->is_gtk_object[$object->in_module . $object->name], $constructor)) {
                    $functions_decl .= sprintf($function_entry_tpl,
                                               $constructor->is_constructor_of,
                                               strtolower($constructor->c_name),
                                               'NULL');
                } else {
                    $functions_decl .= sprintf($function_entry_tpl,
                                               $constructor->is_constructor_of,
                                               'no_constructor', 'NULL');
                }
                */
            }

            /*
             * Insert get_type() as class method if the object is descended from
             * GtkObject.
             */
            /* XXX replace with an overloaded property?
            if ($this->is_gtk_object[$object->in_module . $object->name]) {
                $lclass = strtolower(substr(convert_typename($object->c_name), 1));
                $functions_decl .= sprintf($function_entry_tpl,
                                           'get_type',
                                           $lclass .  '_get_type',
                                           'NULL');
                fwrite($this->fp, sprintf($get_type_tpl, $lclass, $lclass));
            }
            */

            $this->write_methods($object);

            /* XXX fix
            if (isset($this->overrides->extra_methods[$object->c_name])) {
                foreach ($this->overrides->extra_methods[$object->c_name] as $method_cname => $method_data) {
                    list($method_name, $method_override) = $method_data;
                    fwrite($this->fp, $method_override . "\n");
                    if (!isset($method_name))
                        $method_name = $method_cname;
                    $functions_decl .= sprintf($function_entry_tpl,
                                               strtolower($method_name),
                                               strtolower($method_cname),
                                               'NULL');
                }
            }
            */

            /* XXX fix 
            if (count($object->fields))
                $this->write_prop_getter($this->fp, $object);
            */
        }

        $this->register_classes .= "\n" . implode("\n", $this->overrides->get_register_classes());
    }

    function write_classes()
    {
        fprintf(STDERR, "Writing classes...\n");

        $register_classes = '';

        if ($this->parser->enums || $this->parser->functions) {
            //TODO $func_defs = $this->write_functions();

            /*
            $register_classes .= sprintf(Templates::register_class,
                                         $this->lprefix . '_ce',
                                         $this->lprefix,
                                         $func_defs ? $this->lprefix . '_methods' : 'NULL',
                                         'NULL', 0);
            */
        }

        /* Objects */
        foreach ($this->parser->objects as $object) {
            $object_module = strtolower($object->in_module);
            $object_lname = strtolower($object->name);

            $method_defs = $this->write_methods($object);

            $register_classes .= sprintf(Templates::register_class,
                                         $object->ce,
                                         $object->in_module . $object->name,
                                         $method_defs ? strtolower($object->c_name) . '_methods' : 'NULL',
                                         $object->parent ? strtolower($object->parent) . '_ce' : 'NULL',
                                         $object->typecode);
        }
        $register_classes .= "\n" . implode("\n", $this->overrides->get_register_classes());

        fwrite($this->fp, sprintf(Templates::register_classes,
                                  $this->lprefix,
                                  $register_classes));
    }

    function write_enums()
    {
        if (!$this->parser->enums) return;

        fwrite($this->fp, "\n/* Enums and Flags */\n");

        $enums_code = '';

        foreach ($this->parser->enums as $enum) {
            if ($enum->typecode === null) {
                throw new Exception("unhandled enum type");
                foreach ($enum->values as $nick => $value) {
                }
            } else {
                $enums_code .= sprintf(Templates::register_enum, $enum->def_type,
                                       $enum->typecode, $this->lprefix . '_ce');
            }
        }

        fwrite($this->fp, sprintf(Templates::register_constants, $this->lprefix, $enums_code));
    }

    function write_functions()
    {
        $func_defs = array();

        fprintf(STDERR, "Writing functions for $this->prefix...\n");

        foreach ($this->parser->functions as $function) {
            $func_name = $function->name;
            if ($function->name == $function->c_name) {
                $func_name = substr($function->name, strlen($this->lprefix) + 1);
            }

            /* skip ignored methods */
            if ($this->overrides->is_ignored($function->c_name)) continue;

            try {
                if ($this->overrides->is_overriden($function->c_name)) {
                } else {
                    //$code = $this->write_callable($function, Templates::function_body, true, false);
                    $func_defs[] = sprintf(Templates::function_entry,
                                             $this->lprefix,
                                             $func_name, 'NULL', 0);
                }
            } catch (Exception $e) {
                fprintf(STDERR, 'not generating function %s::%s: %s', $this->lprefix, $function->name, $e->getMessage());
            }
        }

        if ($func_defs) {
            fwrite($this->fp, sprintf(Templates::functions_decl, strtolower($this->lprefix)));
            fwrite($this->fp, join('', $func_defs));
            fwrite($this->fp, Templates::functions_decl_end);
        }

        return $func_defs;

        /* XXX fix
        foreach ($this->parser->functions as $function) {
            if ($this->overrides->is_overriden($function->c_name)) {
                list($function_name, $function_override) = $this->overrides->get_override($function->c_name);
                fwrite($this->fp, $function_override . "\n");
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
                if ($this->write_function($this->fp, $function)) {
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
        */
        if ($separate_class)
            $functions_decl .= $this->functions_decl_end;

        // XXX if ($num_functions > 0) {
            if ($separate_class) {
                /*
                $this->register_classes .= sprintf($init_class_tpl,
                                                   $this->prefix,
                                                   $this->lprefix,
                                                   'NULL');
                */
                $this->register_classes .= sprintf($register_class_tpl,
                                                   $this->lprefix . '_ce',
                                                   $this->lprefix, $this->lprefix,
                                                   'NULL', 0);
                fwrite($this->fp, $functions_decl);
            }
        //}
    }

    function write_prop_lists()
    {
        global  $class_prop_list_header,
                $class_prop_list_footer;
        
        fwrite($this->fp, "\n");
        foreach ($this->parser->objects as $object) {
            if (count($object->fields) == 0) continue;

            fwrite($this->fp, sprintf($class_prop_list_header,
                                strtolower($object->in_module) . '_' .
                                strtolower($object->name)));
            foreach ($object->fields as $field_def) {
                list(, $field_name) = $field_def;
                fwrite($this->fp, "\t\"$field_name\",\n");
            }
            fwrite($this->fp, $class_prop_list_footer);
        }
    }

    function write_class_entries()
    {
        /* XXX
        foreach ($this->parser->structs as $struct)
            fwrite($this->fp, sprintf($class_entry_tpl, $struct->ce));
         */
        fwrite($this->fp, "\n");
        foreach ($this->parser->objects as $object) {
            fwrite($this->fp, sprintf(Templates::class_entry, $object->ce));
        }
        foreach ($this->overrides->get_register_classes() as $ce => $rest)
            fwrite($this->fp, sprintf(Templates::class_entry, $ce));

        if ($this->parser->enums || $this->parser->functions) {
            fwrite($this->fp, sprintf(Templates::class_entry, $this->lprefix . '_ce'));
        }
    }

    function write_source($savefile)
    {
        $this->fp = fopen($savefile, 'w');
        fwrite($this->fp, "#include \"php_gtk.h\"");
        fwrite($this->fp, "\n#if HAVE_PHP_GTK\n");
        fwrite($this->fp, $this->overrides->get_headers());
        //$this->write_constants();
        $this->write_class_entries();
        //$this->write_prop_lists();
        //if (!isset($this->parser->objects[$this->function_class]))
        //    $this->write_functions(true, $dummy);
        //$this->write_structs();
        $this->write_enums();
        $this->write_classes();
        fwrite($this->fp, "\n#endif /* HAVE_PHP_GTK */\n");
        fclose($this->fp);
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


$old_error_reporting = error_reporting(E_ALL);

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

    
$result = Console_Getopt::getopt($argv, 'o:p:c:r:f:');
if (!$result || count($result[1]) < 2)
    fatal_error("usage: php -q generator.php [-o overridesfile] [-p prefix] [-c functionclass ] [-r typesfile] [-f savefile] defsfile\n");

list($opts, $argv) = $result;

$prefix = 'Gtk';
$function_class = null;
$overrides = new Overrides();
$register_defs = array();
$savefile = 'php://stdout';

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
    } else if ($opt_spec == 'f') {
        $savefile = $opt_arg;
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
$generator->write_source($savefile);

error_reporting($old_error_reporting);

/* vim: set et sts=4: */
?>
