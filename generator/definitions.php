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

abstract class Definition {
    static $true_values = array('#t', 't');
    static $char_types  = array('char*', 'gchar*', 'string');

    /* return true if caller owns return value */
    function guess_return_value_ownership()
    {
        if ($this->is_constructor_of)
            $this->caller_owns_return = true;
        else if (in_array($this->return_type, $char_types))
            $this->caller_owns_return = true;
        else
            $this->caller_owns_return = false;
    }
}

class Enum_Def extends Definition {
    var $def_type   = 'enum';
    var $name       = null;
    var $in_module  = null;
    var $c_name     = null;
    var $values     = array();
    var $simple     = true;

    function Enum_Def($args)
    {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'in-module')
                $this->in_module = $arg[1];
            else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'value') {
                $value_name = null;
                $value_c_name = null;
                foreach (array_slice($arg, 1) as $value_arg) {
                    if ($value_arg[0] == 'name')
                        $value_name = $value_arg[1];
                    else if ($value_arg[0] == 'c-name')
                        $value_c_name = $value_arg[1];
                }
                $this->values[] = array($value_name, $value_c_name);
            }
        }
    }
}

class Flag_Def extends Enum_Def {
    var $def_type = 'flags';

    function Flag_Def($args) {
        parent::Enum_Def($args);
    }
}

class Object_Def extends Definition {
    var $def_type   = 'object';
    var $name       = null;
    var $in_module  = null;
    var $parent     = null;
    var $c_name     = null;
    var $ce         = null;
    var $fields     = array();
    var $methods    = array();

    function Object_Def($args)
    {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'in-module')
                $this->in_module = $arg[1];
            else if ($arg[0] == 'parent') {
                if (count($arg) > 2)
                    $this->parent = array($arg[1], $arg[2][0]);
                else
                    $this->parent = array($arg[1], null);
            }
            else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'field') {
                $field_type = null;
                $field_name = null;
                foreach (array_slice($arg, 1) as $field_arg) {
                    if ($field_arg[0] == 'type-and-name') {
                        $field_type = $field_arg[1];
                        $field_name = $field_arg[2];
                    }
                }
                $this->fields[] = array($field_type, $field_name);
            }
        }
        $this->ce = strtolower($this->in_module . '_' . $this->name . '_ce');
    }
}

class Method_Def extends Definition {
    var $name               = null;
    var $of_object          = null;
    var $c_name             = null;
    var $return_type        = null;
    var $caller_owns_return = null;
    var $params             = array();
    var $varargs            = false;
    var $deprecated         = null;

    function Method_Def($args)
    {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'of-object') {
                $this->of_object = $arg[1];
            } else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'return-type')
                $this->return_type = $arg[1];
            else if ($arg[0] == 'caller-owns-return')
                $this->caller_owns_return = in_array($arg[1], Definition::$true_values);
            else if ($arg[0] == 'parameters') {
                $param_type = null;
                $param_name = null;
                $param_default = null;
                $param_null = false;
                foreach (array_slice($arg, 1) as $param_arg) {
                    $param_type = $param_arg[0];
                    $param_name = $param_arg[1];
                    foreach (array_slice($param_arg, 2) as $opt_arg) {
                        if ($opt_arg[0] == 'default')
                            $param_default = $opt_arg[1];
                        else if ($opt_arg[0] == 'null-ok')
                            $param_null = true;
                    }
                    $this->params[] = array($param_type, $param_name,
                                            $param_default, $param_null);
                }
            } else if ($arg[0] == 'varargs')
                $this->varargs = in_array($arg[1], Definition::$true_values);
            else if ($arg[0] == 'deprecated')
                $this->deprecated = $arg[1];
        }

        if ($this->caller_owns_return === null && $this->return_type !== null) {
            $this->guess_return_value_ownership();
        }
    }
}

class Function_Def extends Definition {
    var $name               = null;
    var $in_module          = null;
    var $is_constructor_of  = null;
    var $c_name             = null;
    var $return_type        = null;
    var $caller_owns_return = null;
    var $params             = array();
    var $varargs            = false;
    var $deprecated         = null;

    function Function_Def($args) {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'in-module')
                $this->in_module = $arg[1];
            else if ($arg[0] == 'is-constructor-of')
                $this->is_constructor_of = $arg[1];
            else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'return-type')
                $this->return_type = $arg[1];
            else if ($arg[0] == 'caller-owns-return')
                $this->caller_owns_return = in_array($arg[1], Definition::$true_values);
            else if ($arg[0] == 'parameters') {
                $param_type = null;
                $param_name = null;
                $param_default = null;
                $param_null = false;
                foreach (array_slice($arg, 1) as $param_arg) {
                    $param_type = $param_arg[0];
                    $param_name = $param_arg[1];
                    foreach (array_slice($param_arg, 2) as $opt_arg) {
                        if ($opt_arg[0] == 'default')
                            $param_default = $opt_arg[1];
                        else if ($opt_arg[0] == 'null-ok')
                            $param_null = true;
                    }
                    $this->params[] = array($param_type, $param_name,
                                            $param_default, $param_null);
                }
            } else if ($arg[0] == 'varargs')
                $this->varargs = in_array($arg[1], Definition::$true_values);
            else if ($arg[0] == 'deprecated')
                $this->deprecated = $arg[1];
        }

        if ($this->caller_owns_return === null && $this->return_type !== null) {
            $this->guess_return_value_ownership();
        }
    }
}

class Struct_Def extends Definition {
    var $def_type   = 'struct';
    var $name       = null;
    var $in_module  = null;
    var $c_name     = null;
    var $ce         = null;
    var $fields     = array();

    function Struct_Def($args)
    {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'in-module')
                $this->in_module = $arg[1];
            else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'field') {
                $field_type = null;
                $field_name = null;
                foreach (array_slice($arg, 1) as $field_arg) {
                    if ($field_arg[0] == 'type-and-name') {
                        $field_type = $field_arg[1];
                        $field_name = $field_arg[2];
                    }
                }
                $this->fields[] = array($field_type, $field_name);
            }
        }
        $this->ce = strtolower($this->in_module . convert_typename($this->name) . '_ce');
    }
}

/* vim: set et: */
?>
