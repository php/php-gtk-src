<?php
/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2004 Andrei Zmievski <andrei@php.net>
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

// override the default PHP 8Mb as this script tends to use a lot more
// and hopefully reduce the support questions a bit..
ini_set('memory_limit','64M');

if (strstr(PHP_OS, 'WIN')) {
	define('WIN_OS', true);
} else {
	define('WIN_OS', false);
}

require "Getopt.php";
require "override.php";
require "arg_types.php";
require "scheme.php";
require "templates.php";
require "array_printf.php";
require "lineoutput.php";

class Generator {
    var $parser             = null;
    var $overrides          = null;
    var $prefix             = null;
    var $lprefix            = null;
    var $function_class     = null;
    var $logfile            = null;
    var $diversions         = null;

    var $gtype_constants    = '';

    var $template_map       = array('object' => array('constructor' => Templates::constructor_body,
                                                      'static_constructor' => Templates::static_constructor_body,
                                                      'method' => Templates::method_body,
                                                      'prop' => Templates::prop_access),
                                    'boxed'  => array('constructor' => Templates::boxed_constructor_body,
                                                      'static_constructor' => Templates::boxed_static_constructor_body,
                                                      'method' => Templates::boxed_method_body,
                                                      'prop' => Templates::boxed_prop_access),
                                    'interface' => array('method' => Templates::method_body),
                                   );
    var $handlers           = array('read_property', 'write_property', 'get_properties');
    var $cover              = array();

    function Generator(&$parser, &$overrides, $prefix, $function_class)
    {
        $this->parser    = &$parser;
        $this->overrides = &$overrides;
        $this->prefix    = ucfirst($prefix);
        $this->lprefix   = strtolower($prefix);
        $this->function_class = $function_class;

        $this->cover["funcs"] = new Coverage("Functions");
        $this->cover["methods"] = new Coverage("Methods");
        $this->cover["ctors"] = new Coverage("Constructors");
        $this->cover["props"] = new Coverage("Property accessors");
    }

    function set_logfile($logfile) {
        $this->logfile = fopen($logfile, 'w');
    }
    
    function log_print()
    {
        $args = func_get_args();
        if (count($args) == 0) return;

        $format = array_shift($args);

        $output = vsprintf($format, $args);

        if (!WIN_OS) {
            echo $output;
        }

        fwrite($this->logfile, $output);
    }

    function log()
    {
        $args = func_get_args();
        if (count($args) == 0) return;

        $format = array_shift($args);

        fwrite($this->logfile, vsprintf($format, $args));
    }

    function divert()
    {
        $args = func_get_args();
        if (count($args) < 2) return;

        list ($divert_id, $format) = $args;

        @$this->diversions[$divert_id] .= vsprintf($format, array_slice($args, 2));
    }

    function register_types($parser = null)
    {
        global  $matcher;

        if (!$parser)
            $parser = $this->parser;

        foreach ($parser->objects as $object) {
            $matcher->register_object($object->c_name, $object->typecode);
        }

        foreach ($parser->interfaces as $interface) {
            $matcher->register_object($interface->c_name, $interface->typecode);
        }

        foreach ($parser->enums as $enum) {
            if ($enum->def_type == 'flags')
                $matcher->register_flag($enum->c_name, $enum->typecode);
            else
                $matcher->register_enum($enum->c_name, $enum->typecode);
        }

        foreach ($parser->boxed as $boxed) {
            $matcher->register_boxed($boxed->c_name, $boxed->typecode);
        }
    }


    function write_override($override, $id)
    {
        $args = array_slice(func_get_args(), 1);
        list($lineno, $file_name) = $this->overrides->get_line_info(join('.', $args)); 
        $this->fp->set_line($lineno, $file_name);
        $this->fp->write($override);
        $this->fp->reset_line();
        $this->fp->write("\n\n");
    }


    function write_callable($callable, $template, $handle_return = false, $is_method = false, $dict = array())
    {
        global $matcher;

        if ($callable->varargs) {
            throw new Exception('varargs methods not supported');
        }

        $info = new Wrapper_Info();
        /*
         * Slight hack that relies on both parameters being false when used to
         * write out a constructor.
         */
        if (!$handle_return && !$is_method) {
            $info->error_action = "PHPG_THROW_CONSTRUCT_EXCEPTION($dict[class])";
        }

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

        if (isset($callable->deprecated)) {
            $info->pre_code[] = sprintf(Templates::deprecation_msg, $callable->deprecated ? '"'.$callable->deprecated.'"' : 'NULL');
        }

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

        if (!empty($dict['ctor_with_props'])) {
            $dict['n_params'] = count((array)$callable->params);
            $dict['n_args'] = $dict['n_params'] + 1;
            $prop_names = '';
            foreach ($callable->properties as $prop) {
                $prop_names .= '"' . $prop[0] . '", ';
            }
            $dict['props'] = $prop_names;
            $n = 0;
            for ($i = 0; $i < count($info->parse_list); $i++) {
                if ($info->parse_list[$i] && $info->parse_list[$i]{0} == '&') {
                    $info->parse_list[$i] = "&php_args[$n]";
                    $n++;
                }
            }
            $dict['specs'] = preg_replace(',([^|!/]),', '^$1', $dict['specs']);
            $dict['parse_list'] = $info->get_parse_list();
            $dict['pre_code'] = '';
            $dict['post_code'] = '';
        }

        return aprintf($template, $dict);
    }


    function write_methods($object)
    {
        $this->log_print("  %-20s ", "methods");

        $num_written = $num_skipped = 0;
        $method_entries = array();

        $methods = $this->parser->find_methods($object);

        $dict['class'] = $object->c_name;
        $dict['scope'] = $object->c_name;
        $dict['typecode'] = $object->typecode;

        switch ($object->def_type) {
            case 'object':
            case 'interface':
                $dict['cast'] = preg_replace('!_TYPE_!', '_', $object->typecode, 1);
                break;

            case 'boxed':
                $dict['cast'] = $object->c_name . ' *';
                break;

            default:
                throw new Exception("unhandled definition type");
                break;
        }

        $object->methods = array();
        $methodarginfos = '';

        foreach ($methods as $method) {
            $method_name = $method->c_name;
            
            /* skip ignored methods */
            if ($this->overrides->is_ignored($method_name)) continue;

            try {
                list($arginfo, $reflection_func) = $this->genReflectionArgInfo($method, $object);
                
                if (($overriden = $this->overrides->is_overriden($method_name))) {
                    list($method_name, $method_override, $flags) = $this->overrides->get_override($method_name);
                    if (!isset($method_name))
                        $method_name = $method->name;
                    $method_override = preg_replace('!^.*(PHP_METHOD).*$!m', "static $1($object->in_module$object->name, $method_name)", $method_override);
                    $this->write_override($method_override, $method->c_name);
                    $method_entries[$method_name] = array($object->in_module . $object->name,
                                                          $method_name, $reflection_func, $flags ?  $flags : 'ZEND_ACC_PUBLIC');
                } else {
                    if ($method->static) {
                        $code = $this->write_callable($method, Templates::function_body, true, false, $dict);
                        $flags = 'ZEND_ACC_PUBLIC|ZEND_ACC_STATIC';
                    } else {
                        $template = $this->template_map[$object->def_type]['method'];
                        $code = $this->write_callable($method, $template, true, true, $dict);
                        $flags = 'ZEND_ACC_PUBLIC';
                    }
                    $this->fp->write($code);
                    $method_entries[$method->name] = array($object->in_module . $object->name,
                                                           $method->name, $reflection_func, $flags);
                }
                
                $this->divert("gen", "%s  %-11s %s::%s\n", $overriden ? "%%":"  ", "method", $object->c_name, $method->name);
                $num_written++;
                $this->cover["methods"]->written();
                
                if ($arginfo === null) {
                    $object->methods[$method->name] = 1;
                } else {
                    $methodarginfos .= $arginfo;
                    $object->methods[$method->name] = 2;
                }
            } catch (Exception $e) {
                $this->divert("notgen", "  %-11s %s::%s: %s\n", "method", $object->c_name, $method->name, $e->getMessage());
                $num_skipped++;
                $this->cover["methods"]->skipped();
            }
        }

        $this->log_print("(%d written, %d skipped)\n", $num_written, $num_skipped);

        return array($method_entries, $methodarginfos);
    }

    function write_constructor($object)
    {
        $this->log_print("  %-20s ", "constructors");
        $num_written = $num_skipped = 0;

        $ctors = $this->parser->find_constructor($object, $this->overrides);

        $ctor_defs = array();
        $ctor_arginfos = '';

        if ($ctors) {
            $dict['class'] = $object->c_name;
            $dict['typecode'] = $object->typecode;;
            $first = 1;

            foreach ($ctors as $ctor) {
                $ctor_name = $ctor->c_name;
                if ($first) {
                    $ctor_fe_name = '__construct';
                    $flags = 'ZEND_ACC_PUBLIC';
                    $template_name = 'constructor';
                } else {
                    // remove class name from the constructor name, i.e. turn
                    // gtk_button_new_with_mnemonic into new_with_mnemonic
                    $ctor_fe_name = substr($ctor_name, strlen(convert_typename($ctor->is_constructor_of)));
                    $flags = 'ZEND_ACC_PUBLIC|ZEND_ACC_STATIC';
                    $template_name = 'static_constructor';
                }

                $dict['ctor_with_props'] = false;
                /* Use simple no-prop constructor only for default constructors
                   with no parameters. */
                if ($object->def_type == 'object' && !$ctor->params && $first) {
                    $template = Templates::constructor_without_props;
                    $dict['ctor_with_props'] = true;
                } else if ($object->def_type == 'object' && $ctor->properties) {
                    $template = Templates::constructor_with_props;
                    $dict['ctor_with_props'] = true;
                } else {
                    $template = $this->template_map[$object->def_type][$template_name];
                }

                try {
                    list($arginfo, $reflection_func) = $this->genReflectionArgInfo($ctor, $object, $ctor_fe_name);

                    if (($overriden = $this->overrides->is_overriden($ctor_name))) {
                        list(, $ctor_override, $ctor_flags) = $this->overrides->get_override($ctor_name);
                        if (!empty($ctor_flags))
                            $flags = $ctor_flags;
                        $ctor_override = preg_replace('!^.*(PHP_METHOD).*$!m', "static $1($ctor->is_constructor_of, $ctor_fe_name)", $ctor_override);
                        $this->write_override($ctor_override, $ctor->c_name);
                    } else {
                        $dict['name'] = $ctor_fe_name;
                        $code = $this->write_callable($ctor, $template, false, false, $dict);
                        $this->fp->write($code);
                    }

                    $ctor_defs[] = sprintf(Templates::method_entry,
                                           $ctor->is_constructor_of,
                                           $ctor_fe_name, $reflection_func, $flags);
                    $this->divert("gen", "%s  %-11s %s::%s\n", $overriden?"%%":"  ", "constructor", $object->c_name, $ctor_fe_name);
                    $num_written++;
                    $this->cover["ctors"]->written();
                    
                    if ($arginfo !== null) {
                        $ctor_arginfos .= $arginfo;
                    }
                } catch (Exception $e) {
                    $this->divert("notgen", "  %-11s %s::%s: %s\n", "constructor", $object->c_name, $ctor_fe_name, $e->getMessage());
                    $num_skipped++;
                    $this->cover["ctors"]->skipped();
                    // mark class as non-instantiable directly if we were trying
                    // to generate default constructor
                    if ($ctor_fe_name == '__construct') {
                        $ctor_defs[] = sprintf(Templates::function_entry, $ctor_fe_name, 'no_direct_constructor');
                    }
                }
                $first = 0;
            }
        } else {
            if ($this->overrides->have_extra_methods($object->c_name)) {
                $ctor_name = '__construct';
                $extras = $this->overrides->get_extra_methods($object->c_name);
                if (isset($extras[$ctor_name])) {
                    $ctor_body = $extras[$ctor_name];
                    $ctor_body = preg_replace('!^.*(PHP_METHOD).*$!m', "static $1($object->c_name, $ctor_name)", $ctor_body);
                    $this->write_override($ctor_body, $object->c_name, $ctor_name);
                    $ctor_defs[] = sprintf(Templates::method_entry,
                                           $object->c_name, $ctor_name,
                                           'NULL', 'ZEND_ACC_PUBLIC');
                    $this->divert("gen", "%%%%  %-11s %s::%s\n", "constructor", $object->c_name, $ctor_name);
                    $num_written++;
                }
            } else {
                // mark class as non-instantiable directly, only if it's not
                // GObject. For GObject's we let it chain up to GObject
                // constructor
                if ($object->def_type != 'object') {
                    $ctor_defs[] = sprintf(Templates::function_entry, '__construct', 'no_direct_constructor');
                }
            }
        }

        $this->log_print("(%d written, %d skipped)\n", $num_written, $num_skipped);
        return array($ctor_defs, $ctor_arginfos);
    }//function write_constructor($object)

    function write_classes()
    {
        $register_classes = '';

        if ($this->parser->functions || $this->parser->enums) {
            $this->log_print("\n%s\n%s\n", $this->prefix, str_repeat('~', 50));
            $func_defs = $this->write_functions();

            $register_classes .= aprintf(Templates::register_class,
                                         array('ce' => $this->lprefix . '_ce',
                                               'class' => $this->prefix,
                                               'methods' => $func_defs ? $this->lprefix . '_methods' : 'NULL',
                                               'parent' => 'NULL',
                                               'ce_flags' => 0,
                                               'propinfo' => 'NULL',
                                               'create_func' => 'NULL',
                                               'typecode' => 0));
        }

        /* GInterface's */
        $this->log_print("\n\n" . $this->make_header("Interfaces", 50, '-'));
        foreach ($this->parser->interfaces as $interface) {
            $reg_info = $this->write_class($interface);
            $register_classes .= aprintf(Templates::register_interface, $reg_info);
        }
        if (!$this->parser->interfaces) {
            $this->log_print("  -- none --  ");
        }

        /* GObject's */
        $this->log_print("\n\n" . $this->make_header("Objects", 50, '-'));
        foreach ($this->parser->objects as $object) {
            /* skip ignored objects */
            if ($this->overrides->is_ignored($object->c_name)) {
                $this->log_print("\n%s %s\n", $object->c_name, "is ignored in the overrides file");
                continue;
            }
            $reg_info = $this->write_class($object);
            $register_classes .= aprintf(Templates::register_class, $reg_info);
        }
        if (!$this->parser->objects) {
            $this->log_print("  -- none --  ");
        }

        /* GBoxed */
        $this->log_print("\n\n" . $this->make_header("Boxed Types", 50, '-'));
        foreach ($this->parser->boxed as $object) {
            $reg_info = $this->write_class($object);
            $register_classes .= aprintf(Templates::register_boxed, $reg_info);
        }
        if (!$this->parser->boxed) {
            $this->log_print("  -- none --  ");
        }

        $this->fp->write(sprintf(Templates::register_classes,
                                  $this->lprefix,
                                  $register_classes));
    }
    
    function write_prop_handlers($object)
    {
        global $matcher;

        if (!$object->fields) {
            return 'NULL';
        }

        $this->log_print("  %-20s ", "property accessors");
        $num_written = $num_skipped = 0;

        $class = strtolower($object->c_name);
        $prop_defs = array();
        $dict = array();

        switch ($object->def_type) {
            case 'object':
                $dict['cast'] = preg_replace('!_TYPE_!', '_', $object->typecode, 1);
                break;

            case 'boxed':
                $dict['cast'] = $object->c_name . ' *';
                break;

            default:
                throw new Exception("unhandled definition type");
                break;
        }

        foreach ($object->fields as $field) {
            list($field_type, $field_name) = $field;

            $read_func = "PHPG_PROP_READ_FN($object->c_name, $field_name)";
            $write_func = 'NULL';
            $info = new Wrapper_Info();

            try {
                if ($this->overrides->is_prop_overriden($object->c_name, $field_name)) {
                    $overrides = $this->overrides->get_prop_override($object->c_name, $field_name);
                    if (isset($overrides['read'])) {
                        $read_override = preg_replace('!^.*(PHPG_PROP_READER).*$!m', "$1($object->c_name, $field_name)", $overrides['read']);
                        $this->write_override($read_override, $object->c_name, $field_name, 'read');
                        $this->divert("gen", "%%%%  %-11s %s->%s\n", "reader for", $object->c_name, $field_name);
                    } else {
                        $read_func = 'NULL';
                    }
                    if (isset($overrides['write'])) {
                        $write_override = preg_replace('!^.*(PHPG_PROP_WRITER).*$!m', "static $1($object->c_name, $field_name)", $overrides['write']);
                        $this->write_override($overrides['read'], $object->c_name, $field_name, 'write');
                        $this->divert("gen ", "%%%%  %-11s %s->%s\n", "writer for", $object->c_name, $field_name);
                    }
                } else {
                    $handler = $matcher->get($field_type);
                    $handler->write_return($field_type, false, $info);

                    $dict['name'] = $field_name;

                    $this->fp->write(aprintf(Templates::prop_reader,
                                              array('class' => $object->c_name,
                                                    'name' => $field_name,
                                                    'var_list' => $info->get_var_list(),
                                                    'pre_code' => $info->get_pre_code(),
                                                    'post_code' => $info->get_post_code(),
                                                    'prop_access' => aprintf($this->template_map[$object->def_type]['prop'], $dict)
                                                   )));
                    $this->divert("gen", "    %-11s %s->%s\n", "reader for", $object->c_name, $field_name);
                }
                $prop_defs[] = sprintf(Templates::prop_info_entry, 
                                       $field_name, $read_func, $write_func);
                $num_written++;
                $this->cover["props"]->written();
            } catch (Exception $e) {
                $this->divert("notgen", "  %-11s %s->%s: %s\n", "reader for", $object->c_name, $field_name, $e->getMessage());
                $num_skipped++;
                $this->cover["props"]->written();
            }
        }

        $this->log_print("(%d written, %d skipped)\n", $num_written, $num_skipped);

        if ($prop_defs) {
            $this->fp->write(sprintf(Templates::prop_info_header, $class));
            $this->fp->write(join('', $prop_defs));
            $this->fp->write(Templates::prop_info_footer);
            return $class . '_prop_info';
        } else {
            return 'NULL';
        }
    }

    function write_object_handlers($object)
    {
        $handlers = array();

        foreach ($this->handlers as $handler) {
            if ($this->overrides->is_handler_overriden($object->c_name, $handler)) {
                $override = $this->overrides->get_handler_override($object->c_name, $handler);
                $this->write_override($override, $object->c_name, $handler);
                $handlers[] = $handler;
            }
        }

        if (!$handlers)
            return array('NULL', '');

        $dict['class'] = strtolower($object->in_module . $object->name);
        switch ($object->def_type) {
            case 'object':
                $dict['create_func'] = 'phpg_create_gobject';
                break;

            case 'boxed':
                $dict['create_func'] = 'phpg_create_gboxed';
                break;

            default:
                throw new Exception("unhandled definition type");
                break;
        }

        $this->fp->write(aprintf(Templates::custom_create_func, $dict));
        $create_func = 'phpg_create_' . $dict['class'];

        $extra_reg_info = aprintf(Templates::custom_handlers_init, $dict);
        foreach ($handlers as $handler) {
            $dict['handler'] = $handler;
            $extra_reg_info .= aprintf(Templates::custom_handler_set, $dict);
        }

        return array($create_func, $extra_reg_info);
    }

    function make_method_defs($class, $method_entries)
    {
        $method_defs = array();

        if ($class->def_type == 'object') {
            foreach ($method_entries as $me) {
                $method_defs[] = vsprintf(Templates::method_entry, $me);
            }
            if ($class->implements) {
                foreach ($class->implements as $interface_name) {
                    $interface = $this->parser->interfaces[$interface_name];
                    $iface_methods = array_diff_key($interface->methods, $method_entries);
                    $method_defs[] = "\n\t/***   $interface_name interface implementations   ***/\n\n";
                    
                    foreach ($iface_methods as $iface_method=>$dummy) {
                        if ($dummy == 1) {
                            $reflection_func = 'NULL';
                        } else {
                            $reflection_func = 'arginfo_' . strtolower($interface->in_module) . '_' . strtolower($interface->c_name) . '_'. $iface_method;
                        }
                        $method_defs[] = sprintf(Templates::alias_entry,
                                                 $interface_name, $iface_method,
                                                 $iface_method, $reflection_func, 'ZEND_ACC_PUBLIC');
                    }
                }
            }
        } else if ($class->def_type == 'boxed') {
            foreach ($method_entries as $me) {
                $method_defs[] = vsprintf(Templates::method_entry, $me);
            }
        } else if ($class->def_type == 'interface') {
            foreach ($method_entries as $me) {
                $method_defs[] = vsprintf(Templates::abs_method_entry, $me);
            }
        }

        return $method_defs;
    }

    function write_class($class)
    {
        $this->log_print("\n%s\n%s\n", $class->c_name, str_repeat('~', 50));

        $class_module = strtolower($class->in_module);
        $class_lname = strtolower($class->name);

        $create_func = 'NULL';
        $extra_reg_info = '';
        $ctor_defs = null;
        $prop_info = 'NULL';
        $ctor_arginfo = '';

        if ($class->def_type != 'interface') {
            /* interfaces don't have these */
            list($ctor_defs, $ctor_arginfo) = $this->write_constructor($class);
            $prop_info = $this->write_prop_handlers($class);
            list($create_func, $extra_reg_info) = $this->write_object_handlers($class);
        }

        list($method_entries, $method_arginfo) = $this->write_methods($class);
        ksort($method_entries);
        $method_defs           = $this->make_method_defs($class, $method_entries);
        
        $arginfo = $ctor_arginfo . $method_arginfo;

        if ($ctor_defs) {
            $method_defs           = array_merge($ctor_defs, $method_defs);
        }

        if ($method_defs) {
            $this->fp->write($arginfo);
            $this->fp->write(sprintf(Templates::functions_decl, strtolower($class->c_name)));
            $this->fp->write(join('', $method_defs));
            $this->fp->write(Templates::functions_decl_end);
        }

        if ($class->def_type == 'object' && $class->implements) {
            $iface_entries = array();
            foreach ($class->implements as $interface) {
                $iface_entries[] = strtolower($interface) . '_ce';
            }
            $extra_reg_info .= aprintf(Templates::implement_interface,
                                       array('ce' => $class->ce,
                                             'ifaces' => join(', ', $iface_entries),
                                             'num_ifaces' => count($iface_entries)));
        }
        $this->gtype_constants .= sprintf(Templates::gtype_constant, $class->ce, $class->typecode);

        return array('ce' => $class->ce,
                     'class' => $class->in_module . $class->name,
                     'methods' => $method_defs ? strtolower($class->c_name) . '_methods' : 'NULL',
                     'parent' => ($class->def_type == 'object' && $class->parent) ? strtolower($class->parent) . '_ce' : 'NULL',
                     'ce_flags' => $class->ce_flags ? implode('|', $class->ce_flags) : 0,
                     'typecode' => $class->typecode,
                     'create_func' => $create_func,
                     'extra_reg_info' => $extra_reg_info,
                     'propinfo' => $prop_info);
    }

    function write_constants()
    {
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

        $this->fp->write(sprintf(Templates::register_constants, $this->lprefix,
                                 $enums_code . "\n" . $this->overrides->get_constants(),
                                 $this->gtype_constants));
    }

    function write_functions()
    {
        $func_defs = array();

        $this->log_print("  %-20s", "functions");
        $num_written = $num_skipped = 0;

        $dict['scope'] = $this->prefix;
        
        $function_arginfos = '';
        
        $object = null;
        $object->in_module = $this->prefix;
        $object->c_name = $this->prefix;

        foreach ($this->parser->functions as $function) {
            $func_name = $function->name;
            if ($function->name == $function->c_name) {
                $func_name = substr($function->name, strlen($this->lprefix) + 1);
            }
            $dict['name'] = $func_name;

            /* skip ignored methods */
            if ($this->overrides->is_ignored($function->c_name)) continue;

            try {
                list($arginfo, $reflection_func) = $this->genReflectionArgInfo($function, $object);
                
                if (($overriden = $this->overrides->is_overriden($function->c_name))) {
                    list($func_name, $function_override, $flags) = $this->overrides->get_override($function->c_name);
                    if (empty($func_name) || $func_name == $function->c_name)
                        $func_name = $function->name;
                    $function_override = preg_replace('!^.*(PHP_METHOD).*$!m', "static $1($this->prefix, $func_name)", $function_override);
                    $this->write_override($function_override, $function->c_name);
                    $func_defs[] = sprintf(Templates::method_entry,
                                           $this->prefix,
                                           $func_name, $reflection_func, $flags ? $flags : 'ZEND_ACC_PUBLIC|ZEND_ACC_STATIC');
                } else {
                    $code = $this->write_callable($function, Templates::function_body, true, false, $dict);
                    $this->fp->write($code);
                    $func_defs[] = sprintf(Templates::method_entry,
                                           $this->prefix,
                                           $func_name, $reflection_func, 'ZEND_ACC_PUBLIC|ZEND_ACC_STATIC');
                }
                $this->divert("gen", "%s  %-11s %s::%s\n", $overriden?"%%":"  ", "function", $this->prefix, $function->name);
                $num_written++;
                $this->cover["funcs"]->written();

                if ($arginfo !== null) {
                    $function_arginfos .= $arginfo;
                }
            } catch (Exception $e) {
                $this->divert("notgen", "  %-11s %s::%s: %s\n", "function", $this->prefix, $function->name, $e->getMessage());
                $num_skipped++;
                $this->cover["funcs"]->skipped();
            }
        }

        if ($this->overrides->have_extra_methods($this->prefix)) {
            foreach ($this->overrides->get_extra_methods($this->prefix) as $func_name => $func_body) {
                list($arginfo, $reflection_func) = $this->genReflectionArgInfo(null, $object, $func_name);

                $func_body = preg_replace('!^.*(PHP_METHOD).*$!m', "static $1($this->prefix, $func_name)", $func_body);
                $this->write_override($func_body, $this->prefix, $func_name);
                $func_defs[] = sprintf(Templates::method_entry,
                                       $this->prefix,
                                       $func_name, $reflection_func, 'ZEND_ACC_PUBLIC|ZEND_ACC_STATIC');
                $this->divert("gen", "%%%%  %-11s %s::%s\n", "function", $this->prefix, $func_name);
                $num_written++;

                if ($arginfo !== null) {
                    $function_arginfos .= $arginfo;
                }
            }
        }

        sort($func_defs);
        if ($func_defs) {
            $this->fp->write($function_arginfos);
            $this->fp->write(sprintf(Templates::functions_decl, strtolower($this->lprefix)));
            $this->fp->write(join('', $func_defs));
            $this->fp->write(Templates::functions_decl_end);
        }

        $this->log_print("(%d written, %d skipped)\n", $num_written, $num_skipped);
        return $func_defs;
    }

    function write_prop_lists()
    {
        global  $class_prop_list_header,
                $class_prop_list_footer;
        
        $this->fp->write("\n");
        foreach ($this->parser->objects as $object) {
            if (count($object->fields) == 0) continue;

            $this->fp->write(sprintf($class_prop_list_header,
                                strtolower($object->in_module) . '_' .
                                strtolower($object->name)));
            foreach ($object->fields as $field_def) {
                list(, $field_name) = $field_def;
                $this->fp->write("\t\"$field_name\",\n");
            }
            $this->fp->write($class_prop_list_footer);
        }
    }

    function write_coverage_info()
    {
        $this->log_print("\n\n");
        $this->log_print($this->make_header("$this->prefix Coverage"));

        foreach ($this->cover as $coverage) {
            list($name, $written, $total, $percent) = $coverage->get_stats();
            if ($total) {
                $this->log_print("%-20s: %5.1f%% written (%d/%d)\n", $name, $percent, $written, $total);
            } else {
                $this->log_print("%-20s: none found\n", $name);
            }
        }
        $this->log_print("\n\n");
    }

    function write_class_entries()
    {
        $this->fp->write("\n");
        foreach ($this->parser->interfaces as $interface) {
            $this->fp->write(sprintf(Templates::class_entry, $interface->ce));
        }

        foreach ($this->parser->objects as $object) {
            if (!$this->overrides->is_ignored($object->c_name)) {
                $this->fp->write(sprintf(Templates::class_entry, $object->ce));
            }
        }

        foreach ($this->parser->boxed as $object) {
            $this->fp->write(sprintf(Templates::class_entry, $object->ce));
        }

        if ($this->parser->functions || $this->parser->enums) {
            $this->fp->write(sprintf(Templates::class_entry, $this->lprefix . '_ce'));
        }
    }

    function make_header($string, $width = 72, $char = '=')
    {
        $res = "\n" . str_repeat($char, $width) . "\n";
        $res .= str_pad($string, $width, " ", STR_PAD_BOTH) . "\n";
        $res .= str_repeat($char, $width) . "\n";

        return $res;
    }

    function write_source($savefile)
    {
        $this->not_generated_list = array();;
        $this->log_print($this->make_header("$this->prefix Summary"));

        $this->fp = new LineOutput(fopen($savefile, 'w'), $savefile);
        $this->fp->write("#include \"php_gtk.h\"\n");
        $this->fp->write("#include \"php_gtk_api.h\"\n");
        $this->fp->write("\n#if HAVE_PHP_GTK\n");
        $this->fp->write($this->overrides->get_headers());
        $this->write_class_entries();
        $this->write_classes();
        $this->write_constants();
        $this->fp->write("\n#endif /* HAVE_PHP_GTK */\n");

        $this->log("\n\n");
        $this->log($this->make_header("$this->prefix Generated Items"));
        $this->log("%%%% - overriden\n\n");
        $this->log("%s", $this->diversions["gen"]);

        $this->log("\n\n");
        $this->log($this->make_header("$this->prefix Not Generated Items"));
        $this->log("%s", $this->diversions["notgen"]);

        $this->write_coverage_info();
        $this->write_unused_info();
    }
    
    
    
    /**
    *   generates an ZEND_ARGINFO entry for this method
    *   based on the parameters and returns it
    *
    *   @param Method $method           The method (or constructor) to generate the arginfo for (if NULL, then we check for overridden only)
    *   @param Class  $class            The class of the method
    *   @param string $det_method_name  Special determined method name if it can't be calculated from the method object
    *   @return array The arginfo (string) and the name of the reflection function
    */
    function genReflectionArgInfo($method, $class, $det_method_name = null)
    {
        if ($det_method_name !== null) {
            $len = 20 - strlen($det_method_name);
        } else {
            $len = 20 - strlen($method->name);
        }
        if ($len < 0) { $len = 0; }

        if ($method === null || ($overriden = $this->overrides->is_overriden($method->c_name))) {
            //overridden function - extra arginfo in override file?
            $class_name  = $class->c_name;
            if ($det_method_name !== null) {
                $method_name = $det_method_name;
            } else {
                $overrideinfo = $this->overrides->get_override($method->c_name);
                $method_name = $overrideinfo[0];
                if (empty($method_name) || $method_name == $method->c_name) {
                    $method_name = $method->name;
                }
            }

            if ($this->overrides->has_extra_arginfo($class_name, $method_name)) {
                $reflection_funcname = Generator::getReflectionFuncName($method, $class, $det_method_name);
                $reflection_func = str_repeat(' ', $len) . $reflection_funcname;

                list($line, $filename) = $this->overrides->get_line_info("$class_name.$method_name.arginfo");

                if (WIN_OS) {
                    $arginfo  = sprintf("//line %d \"%s\"\n", $line, $filename);
                } else {
                    $arginfo  = sprintf("#line %d \"%s\"\n", $line, $filename);
                }

                $arginfo .= str_replace('ARGINFO_NAME', $reflection_funcname, $this->overrides->get_extra_arginfo($class_name, $method_name));
            } else {
                //no arginfo
                $reflection_func = str_repeat(' ', $len) . 'NULL';
                $arginfo = null;
                //var_dump('No arginfo for overridden ' . $reflection_funcname);
            }

        } else if (count($method->params) == 0) {
            $reflection_func = str_repeat(' ', $len) . 'NULL';
            $arginfo = null;
        } else {
            $reflection_funcname = Generator::getReflectionFuncName($method, $class);
            $reflection_func = str_repeat(' ', $len) . $reflection_funcname;

            $param_count = 0;
            $optparam_count = 0;
            $argparams = '';
            foreach ($method->params as $paraminfo) {
                $param_count++;
                if ($paraminfo[2] !== null) {
                    //if this is set, we've got a default value -> optional parameter
                    $optparam_count++;
                }

                $paramtype = str_replace('const-', '', str_replace('*', '', $paraminfo[0]));
                if (Generator::is_php_type($paramtype)) {
                    $argparams .= sprintf(Templates::reflection_arg, $paraminfo[1]);
                } else {
                    $argparams .= sprintf(Templates::reflection_objarg, $paraminfo[1], $paramtype);
                }
            }
            if ($optparam_count > 0) {
                //with optional count
                $arginfo = sprintf(Templates::reflection_arginfoex_begin, $reflection_funcname, $param_count - $optparam_count);
            } else {
                //simple one
                $arginfo = sprintf(Templates::reflection_arginfo_begin, $reflection_funcname);
            }

            $arginfo .= $argparams;
            $arginfo .= Templates::reflection_arginfo_end;
        }
        return array($arginfo, $reflection_func);
    }//function genReflectionArgInfo($method, $class)



    function getReflectionFuncName($method, $class, $det_method_name = null)
    {
        if ($method === null) {
            $method->name = $det_method_name;
        }
        return 'arginfo_' . strtolower($class->in_module) . '_' . strtolower($class->c_name) . '_'. $method->name;
    }//function getReflectionFuncName($method, $class, $det_method_name = null)



    function write_unused_info()
    {
        $unused = $this->overrides->get_unused_arginfo();
        
        $unused_count = 0;
        $output = '';
        foreach ($unused as $class => $methods) {
            foreach ($methods as $method => $true) {
                $unused_count++;
                list($line, $filename) = $this->overrides->get_line_info("$class.$method.arginfo");
                $output .= ' ' . str_pad($class . '::' . $method, 40) . ' ' . $filename . '#' . $line . "\n";
            }
        }
        if ($unused_count > 0) {
            $this->log_print('Unused arginfo overrides:' . "\n");
            $this->log_print($output);
            $this->log_print("\n\n");
        }
    }



    /**
     * Checks if the given type is a simple php type
     *
     * All unsupported classes have to be here as php5 functions weird if
     *  reflection is used with this ones...
     * Use generator/reflection_class_checker.php to missing classes, and
     *  add them here
     *
     * @access public
     * @param  string $in_type The C type.
     * @return boolean true if its a simple type
     */
    function is_php_type($in_type)
    {
        // Key = C Type, Val = PHP type
        static $type_map = array('none'           => 'void',

                                 'char*'          => 'string',
                                 'gchar*'         => 'string',
                                 'const-char*'    => 'string',
                                 'const-gchar*'   => 'string',
                                 'string'         => 'string',
                                 'static_string'  => 'string',
                                 'unsigned-char*' => 'string',
                                 'guchar*'        => 'string',
                                 
                                 'char'           => 'char',
                                 'gchar'          => 'char',
                                 'guchar'         => 'char',
                                 
                                 'int'            => 'int',
                                 'gint'           => 'int',
                                 'guint'          => 'int',
                                 'short'          => 'int',
                                 'gshort'         => 'int',
                                 'gushort'        => 'int',
                                 'long'           => 'int',
                                 'glong'          => 'int',
                                 'gulong'         => 'int',
                                 
                                 'guint8'         => 'int',
                                 'gint8'          => 'int',
                                 'guint16'        => 'int',
                                 'gint16'         => 'int',
                                 'guint32'        => 'int',
                                 'gint32'         => 'int',
                                 
                                 //all enumeration classes here
                                 //gtk enums
                                 'GtkAccelFlags'        => 'int',
                                 'GtkAnchorType'        => 'int',
                                 'GtkArrowType'         => 'int',
                                 'GtkAttachOptions'     => 'int',
                                 'GtkButtonBoxStyle'    => 'int',
                                 'GtkButtonsType'       => 'int',
                                 'GtkCalendarDisplayOptions' => 'int',
                                 'GtkCellRendererState' => 'int',
                                 'GtkCornerType'        => 'int',
                                 'GtkCTreeExpanderStyle'=> 'int',
                                 'GtkCTreeLineStyle'    => 'int',
                                 'GtkCurveType'         => 'int',
                                 'GtkDeleteType'        => 'int',
                                 'GtkDestroyNotify'     => 'int',
                                 'GtkDialogFlags'       => 'int',
                                 'GtkDirectionType'     => 'int',
                                 'GtkExpanderStyle'     => 'int',
                                 'GtkFileChooserAction' => 'int',
                                 'GtkFunction'          => 'int',
                                 'GtkIconLookupFlags'   => 'int',
                                 'GtkIconSize'          => 'int',
                                 'GtkIMPreeditStyle'    => 'int',
                                 'GtkIMStatusStyle'     => 'int',
                                 'GtkJustification'     => 'int',
                                 'GtkMatchType'         => 'int',
                                 'GtkMessageType'       => 'int',
                                 'GtkMetricType'        => 'int',
                                 'GtkMovementStep'      => 'int',
                                 'GtkOrientation'       => 'int',
                                 'GtkPackType'          => 'int',
                                 'GtkPathPriorityType'  => 'int',
                                 'GtkPathType'          => 'int',
                                 'GtkPolicyType'        => 'int',
                                 'GtkPositionType'      => 'int',
                                 'GtkPreviewType'       => 'int',
                                 'GtkProgressBarOrientation' => 'int',
                                 'GtkProgressBarStyle'  => 'int',
                                 'GtkReliefStyle'       => 'int',
                                 'GtkResizeMode'        => 'int',
                                 'GtkScrollStep'        => 'int',
                                 'GtkScrollType'        => 'int',
                                 'GtkSelectionMode'     => 'int',
                                 'GtkShadowType'        => 'int',
                                 'GtkSideType'          => 'int',
                                 'GtkSizeGroupMode'     => 'int',
                                 'GtkSpinButtonUpdatePolicy' => 'int',
                                 'GtkSpinType'          => 'int',
                                 'GtkStateType'         => 'int',
                                 'GtkStockItem'         => 'int',
                                 'GtkSubmenuDirection'  => 'int',
                                 'GtkSubmenuPlacement'  => 'int',
                                 'GtkTextDirection'     => 'int',
                                 'GtkTextSearchFlags'   => 'int',
                                 'GtkTextWindowType'    => 'int',
                                 'GtkToolbarStyle'      => 'int',
                                 'GtkTreeCellDataFunc'  => 'int',
                                 'GtkTreeModel'         => 'int',
                                 'GtkTreePath'          => 'int',
                                 'GtkTreeViewColumnSizing' => 'int',
                                 'GtkTreeViewDropPosition' => 'int',
                                 'GtkType'              => 'int',
                                 'GtkUIManagerItemType' => 'int',
                                 'GtkUpdateType'        => 'int',
                                 'GtkVisibility'        => 'int',
                                 'GtkWindowPosition'    => 'int',
                                 'GtkWindowType'        => 'int',
                                 'GtkWrapMode'          => 'int',
                                 'GtkSortType'          => 'int',
                                 
                                 //gdk enums
                                 'GdkAxisUse'           => 'int',
                                 'GdkBitmap'            => 'int',
                                 'GdkByteOrder'         => 'int',
                                 'GdkCapStyle'          => 'int',
                                 'GdkColorspace'        => 'int',
                                 'GdkCursorType'        => 'int',
                                 'GdkCrossingMode'      => 'int',
                                 'GdkDragAction'        => 'int',
                                 'GdkDragProtocol'      => 'int',
                                 'GdkEventExpose'       => 'int',
                                 'GdkEventKey'          => 'int',
                                 'GdkEventSelection'    => 'int',
                                 'GdkEventType'         => 'int',
                                 'GdkEventMask'         => 'int',
                                 'GdkExtensionMode'     => 'int',
                                 'GdkGravity'           => 'int',
                                 'GdkFill'              => 'int',
                                 'GdkFillRule'          => 'int',
                                 'GdkFilterReturn'      => 'int',
                                 'GdkFontType'          => 'int',
                                 'GdkFunction'          => 'int',
                                 'GdkGCValuesMask'      => 'int',
                                 'GdkGrabStatus'        => 'int',
                                 'GdkImageType'         => 'int',
                                 'GdkInputCondition'    => 'int',
                                 'GdkInputSource'       => 'int',
                                 'GdkInputMode'         => 'int',
                                 'GdkInterpType'        => 'int',
                                 'GdkJoinStyle'         => 'int',
                                 'GdkLineStyle'         => 'int',
                                 'GdkModifierType'      => 'int',
                                 'GdkNotifyType'        => 'int',
                                 'GdkOverlapType'       => 'int',
                                 'GdkOwnerChange'       => 'int',
                                 'GdkPixbufAlphaMode'   => 'int',
                                 'GdkPropertyState'     => 'int',
                                 'GdkPropMode'          => 'int',
                                 'GdkRgbDither'         => 'int',
                                 'GdkScrollDirection'   => 'int',
                                 'GdkSettingAction'     => 'int',
                                 'GdkSubwindowMode'     => 'int',
                                 'GdkVisibilityState'   => 'int',
                                 'GdkVisualType'        => 'int',
                                 'GdkWindowAttributesType'  => 'int',
                                 'GdkWindowClass'       => 'int',
                                 'GdkWindowEdge'        => 'int',
                                 'GdkWindowHints'       => 'int',
                                 'GdkWindowState'       => 'int',
                                 'GdkWindowTypeHint'    => 'int',
                                 'GdkWMDecoration'      => 'int',
                                 'GdkWMFunction'        => 'int',
                                 
                                 //Pango enums
                                 'PangoAlignment'       => 'int',
                                 'PangoDirection'       => 'int',
                                 'PangoEllipsizeMode'   => 'int',
                                 'PangoFontMask'        => 'int',
                                 'PangoRectangle'       => 'int',
                                 'PangoStretch'         => 'int',
                                 'PangoStyle'           => 'int',
                                 'PangoTabAlign'        => 'int',
                                 'PangoVariant'         => 'int',
                                 'PangoWeight'          => 'int',
                                 'PangoWrapMode'        => 'int',
                                 
                                 //Atk enums
                                 'AtkRole'              => 'int',
                                 'AtkRelationType'      => 'int',
                                 'AtkStateType'         => 'int',
                                 'AtkTextAttribute'     => 'int',
                                                                  
                                 //some more
                                 'GDestroyNotify'       => 'int',
                                 'GError'               => 'int',
                                 'GSList'               => 'int',
                                 'GValue'               => 'int',
                                 
                                 'gpointer'             => 'int',
                                 
                                 'gboolean'       => 'bool',
                                 
                                 'double'         => 'double',
                                 'gdouble'        => 'double',
                                 'float'          => 'double',
                                 'gfloat'         => 'double',
                                 
                                 'GdkDrawable*'   => 'GdkWindow');
        
        return isset($type_map[$in_type]);
    }//function is_php_type($in_type)
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

 
$result = Console_Getopt::getopt($argv, 'l:o:p:c:r:f:');
if (!$result || count($result[1]) < 2)
    die("usage: php generator.php [-l logfile] [-o overridesfile] [-p prefix] [-c functionclass ] [-r typesfile] [-f savefile] defsfile\n");

list($opts, $argv) = $result;

$prefix = 'Gtk';
$function_class = null;
$overrides = new Overrides();
$register_defs = array();
$savefile = 'php://stdout';
$logfile = 'php://stderr';

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
    } else if ($opt_spec == 'l') {
        $logfile = $opt_arg;
    }
}

if (file_exists(dirname($argv[1]) . '/arg_types.php')) {
    include(dirname($argv[1]) . '/arg_types.php');
}

$parser = new Defs_Parser($argv[1]);
$generator = new Generator($parser, $overrides, $prefix, $function_class);
$generator->set_logfile($logfile);
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
