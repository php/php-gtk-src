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

abstract class Definition {
    static $true_values = array('#t', 't');
    static $char_types  = array('char*', 'gchar*', 'string');

    /* return true if caller owns return value */
    function guess_return_value_ownership()
    {
        if (get_class($this) == 'Function_Def' && $this->is_constructor_of)
            $this->caller_owns_return = true;
        else if (in_array($this->return_type, self::$char_types))
            $this->caller_owns_return = true;
        else
            $this->caller_owns_return = false;
    }

    function merge($old, $merge_params=null)
    {
    }
}

class Enum_Def extends Definition {
    var $def_type   = 'enum';
    var $name       = null;
    var $in_module  = null;
    var $c_name     = null;
    var $values     = array();
    var $typecode   = null;

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
            else if ($arg[0] == 'gtype-id')
                $this->typecode = $arg[1];
            else if ($arg[0] == 'values')
                $this->values = array_slice($arg, 1);
        }
    }

    function write_defs($fp)
    {
        fwrite($fp, "(define-$this->def_type $this->name\n");
        if ($this->in_module)
            fwrite($fp, "  (in-module \"$this->in_module\")\n");
        if ($this->c_name)
            fwrite($fp, "  (c-name \"$this->c_name\")\n");
        if ($this->typecode)
            fwrite($fp, "  (gtype-id \"$this->typecode\")\n");
        if ($this->values) {
            fwrite($fp, "  (values\n");
            foreach ($this->values as $value) {
                fwrite($fp, "    '(\"$value[0]\" \"$value[1]\")\n");
            }
            fwrite($fp, "  )\n");
        }
        fwrite($fp, ")\n\n");
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
    var $typecode   = null;
    var $implements = array();
    var $ce_flags   = array();

    function Object_Def($args)
    {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'in-module')
                $this->in_module = $arg[1];
            else if ($arg[0] == 'parent') {
                $this->parent = $arg[1];
            }
            else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'gtype-id')
                $this->typecode = $arg[1];
            else if ($arg[0] == 'implements')
                $this->implements[] = $arg[1];
            else if ($arg[0] == 'fields')
                $this->fields = array_slice($arg, 1);
        }
        $this->ce = strtolower($this->c_name) . '_ce';
    }

    function merge($old, $merge_params=null)
    {
        $this->fields = $old->fields;
        $this->implements = $old->implements;
    }

    function write_defs($fp)
    {
        fwrite($fp, "(define-object $this->name\n");
        if ($this->in_module)
            fwrite($fp, "  (in-module \"$this->in_module\")\n");
        if ($this->parent)
            fwrite($fp, "  (parent \"$this->parent\")\n");
        if ($this->c_name)
            fwrite($fp, "  (c-name \"$this->c_name\")\n");
        if ($this->typecode)
            fwrite($fp, "  (gtype-id \"$this->typecode\")\n");
        foreach ($this->implements as $interface) {
            fwrite($fp, "  (implements \"$interface\")\n");
        }
        if ($this->fields) {
            fwrite($fp, "  (fields\n");
            foreach ($this->fields as $field) {
                fwrite($fp, "    '(\"$field[0]\" \"$field[1]\")\n");
            }
            fwrite($fp, "  )\n");
        }
        fwrite($fp, ")\n\n");
    }
}

class Method_Def extends Definition {
    var $def_type           = 'method';
    var $name               = null;
    var $of_object          = null;
    var $c_name             = null;
    var $return_type        = null;
    var $caller_owns_return = null;
    var $params             = array();
    var $varargs            = false;
    var $deprecated         = null;
    var $properties         = array();
    var $static             = false;

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
                $this->caller_owns_return = in_array($arg[1], self::$true_values);
            else if ($arg[0] == 'static')
                $this->static = true;
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
                $this->varargs = in_array($arg[1], self::$true_values);
            else if ($arg[0] == 'deprecated')
                $this->deprecated = $arg[1];
        }

        if ($this->caller_owns_return === null && $this->return_type !== null) {
            $this->guess_return_value_ownership();
        }
    }

    function merge($old, $merge_params=null)
    {
        $this->caller_owns_return = $old->caller_owns_return;
        $this->is_constructor_of = $old->is_constructor_of;
        $this->varargs = $old->varargs;
        $this->deprecated = $old->deprecated;
        $this->static = $old->static;

        if ($merge_params) {
            try {
                foreach ($this->params as &$param) {
                    foreach ($old->params as $old_param) {
                        if ($old_param[1] == $param[1]) {
                            if ($old_param[2] !== null)
                                $param[2] = $old_param[2];
                            if ($old_param[3] !== null)
                                $param[3] = $old_param[3];
                        } else {
                            throw new Exception;
                        }
                    }
                }
            } catch (Exception $e) {
                $this->params = $old->params;
            }
        } else {
            $this->params = $old->params;

        }
    }

    function write_defs($fp)
    {
        fwrite($fp, "(define-method $this->name\n");
        if ($this->of_object)
            fwrite($fp, "  (of-object \"$this->of_object\")\n");
        if ($this->c_name)
            fwrite($fp, "  (c-name \"$this->c_name\")\n");
        if ($this->caller_owns_return)
            fwrite($fp, "  (caller-owns-return #t)\n");
        if ($this->return_type)
            fwrite($fp, "  (return-type \"$this->return_type\")\n");
        if ($this->deprecated)
            fwrite($fp, "  (deprecated \"$this->deprecated\")\n");
        if ($this->params) {
            fwrite($fp, "  (parameters\n");
            foreach ($this->params as $param) {
                fwrite($fp, "    '(\"$param[0]\" \"$param[1]\"");
                if ($param[2])
                    fwrite($fp, " (default \"$param[2]\")");
                if ($param[3])
                    fwrite($fp, " (null-ok)");
                fwrite($fp, ")\n");
            }
            fwrite($fp, "  )\n");
        }
        if ($this->static)
            fwrite($fp, "  (static #t)\n");
        if ($this->varargs)
            fwrite($fp, "  (varargs #t)\n");
        fwrite($fp, ")\n\n");
    }
}

class Function_Def extends Definition {
    var $def_type           = 'function';
    var $name               = null;
    var $in_module          = null;
    var $is_constructor_of  = null;
    var $c_name             = null;
    var $return_type        = null;
    var $caller_owns_return = null;
    var $params             = array();
    var $properties         = array();
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
                $this->caller_owns_return = in_array($arg[1], self::$true_values);
            else if ($arg[0] == 'parameters') {
                foreach (array_slice($arg, 1) as $param_arg) {
                    $param_type = $param_arg[0];
                    $param_name = $param_arg[1];
                    $param_default = null;
                    $param_null = false;
                    foreach (array_slice($param_arg, 2) as $opt_arg) {
                        if ($opt_arg[0] == 'default')
                            $param_default = $opt_arg[1];
                        else if ($opt_arg[0] == 'null-ok')
                            $param_null = true;
                    }
                    $this->params[] = array($param_type, $param_name,
                                            $param_default, $param_null);
                }
            } else if ($arg[0] == 'properties') {
                if (empty($this->is_constructor_of)) {
                    continue;
                }

                foreach (array_slice($arg, 1) as $prop) {
                    $prop_name = $prop[0];
                    $optional = false;
                    foreach (array_slice($prop, 1) as $opt_arg) {
                        if ($opt_arg[0] == 'optional') {
                            $optional = true;
                        }
                    }
                    $this->properties[] = array($prop_name, $optional);
                }
            } else if ($arg[0] == 'varargs')
                $this->varargs = in_array($arg[1], self::$true_values);
            else if ($arg[0] == 'deprecated')
                $this->deprecated = $arg[1];
        }

        if ($this->caller_owns_return === null && $this->return_type !== null) {
            $this->guess_return_value_ownership();
        }
    }

    function merge($old, $merge_params=null)
    {
        $this->caller_owns_return = $old->caller_owns_return;
        $this->varargs = $old->varargs;
        $this->deprecated = $old->deprecated;

        if ($merge_params && $this->params) {
            try {
                foreach ($this->params as &$param) {
                    foreach ($old->params as $old_param) {
                        if ($old_param[1] == $param[1]) {
                            if ($old_param[2] !== null)
                                $param[2] = $old_param[2];
                            if ($old_param[3] !== null)
                                $param[3] = $old_param[3];
                        } else {
                            throw new Exception;
                        }
                    }
                }
            } catch (Exception $e) {
                $this->params = $old->params;
            }
        } else {
            $this->params = $old->params;

        }

        $this->properties = $old->properties;

        if ($old->def_type == 'method') {
            $this->name = $old->name;
            $this->of_object = $old->of_object;
            $this->static = $old->static;
            $this->def_type = 'method';
        } else {
            if ($this->is_constructor_of === null &&
                $old->is_constructor_of !== null) {
                $this->is_constructor_of = $old->is_constructor_of;
            }
        }
    }

    function write_defs($fp)
    {
        if ($this->def_type == 'method') {
            Method_Def::write_defs($fp);
            return;
        }

        fwrite($fp, "(define-function $this->name\n");
        if ($this->in_module)
            fwrite($fp, "  (in-module \"$this->in_module\")\n");
        if ($this->is_constructor_of)
            fwrite($fp, "  (is-constructor-of \"$this->is_constructor_of\")\n");
        if ($this->c_name)
            fwrite($fp, "  (c-name \"$this->c_name\")\n");
        if ($this->caller_owns_return)
            fwrite($fp, "  (caller-owns-return #t)\n");
        if ($this->return_type)
            fwrite($fp, "  (return-type \"$this->return_type\")\n");
        if ($this->deprecated)
            fwrite($fp, "  (deprecated \"$this->deprecated\")\n");
        if ($this->params) {
            fwrite($fp, "  (parameters\n");
            foreach ($this->params as $param) {
                fwrite($fp, "    '(\"$param[0]\" \"$param[1]\"");
                if ($param[2])
                    fwrite($fp, " (default \"$param[2]\")");
                if ($param[3])
                    fwrite($fp, " (null-ok)");
                fwrite($fp, ")\n");
            }
            fwrite($fp, "  )\n");
        }
        if ($this->properties) {
            fwrite($fp, "  (properties\n");
            foreach ($this->properties as $prop) {
                fwrite($fp, "    '(\"$prop[0]\"");
                if ($prop[1])
                    fwrite($fp, " (optional)");
                fwrite($fp, ")\n");
            }
            fwrite($fp, "  )\n");
        }
        if ($this->varargs)
            fwrite($fp, "  (varargs #t)\n");
        fwrite($fp, ")\n\n");
    }
}

class Boxed_Def extends Definition {
    var $def_type   = 'boxed';
    var $name       = null;
    var $in_module  = null;
    var $c_name     = null;
    var $ce         = null;
    var $copy       = null;
    var $release    = null;
    var $fields     = array();
    var $typecode   = null;
    var $ce_flags   = array();

    function Boxed_Def($args)
    {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'in-module')
                $this->in_module = $arg[1];
            else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'gtype-id')
                $this->typecode = $arg[1];
            else if ($arg[0] == 'copy-func')
                $this->copy = $arg[1];
            else if ($arg[0] == 'release-func')
                $this->release = $arg[1];
            else if ($arg[0] == 'fields')
                $this->fields = array_slice($arg, 1);
        }

        $this->ce = strtolower($this->c_name) . '_ce';
    }

    function merge($old, $merge_params=null)
    {
        $this->fields = $old->fields;
    }

    function write_defs($fp)
    {
        fwrite($fp, "(define-boxed $this->name\n");
        if ($this->in_module)
            fwrite($fp, "  (in-module \"$this->in_module\")\n");
        if ($this->c_name)
            fwrite($fp, "  (c-name \"$this->c_name\")\n");
        if ($this->typecode)
            fwrite($fp, "  (gtype-id \"$this->typecode\")\n");
        if ($this->copy)
            fwrite($fp, "  (copy-func \"$this->copy\")\n");
        if ($this->release)
            fwrite($fp, "  (release-func \"$this->release\")\n");
        if ($this->fields) {
            fwrite($fp, "  (fields\n");
            foreach ($this->fields as $field) {
                fwrite($fp, "    '(\"$field[0]\" \"$field[1]\")\n");
            }
            fwrite($fp, "  )\n");
        }
        fwrite($fp, ")\n\n");
    }
}

class Interface_Def extends Definition {
    var $def_type   = 'interface';
    var $name       = null;
    var $in_module  = null;
    var $c_name     = null;
    var $ce         = null;
    var $typecode   = null;
    var $ce_flags   = array();

    function Interface_Def($args)
    {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'in-module')
                $this->in_module = $arg[1];
            else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'gtype-id')
                $this->typecode = $arg[1];
        }

        $this->ce = strtolower($this->c_name) . '_ce';
    }

    function write_defs($fp)
    {
        fwrite($fp, "(define-interface $this->name\n");
        if ($this->in_module)
            fwrite($fp, "  (in-module \"$this->in_module\")\n");
        if ($this->c_name)
            fwrite($fp, "  (c-name \"$this->c_name\")\n");
        if ($this->typecode)
            fwrite($fp, "  (gtype-id \"$this->typecode\")\n");
        fwrite($fp, ")\n\n");
    }
}

class Pointer_Def extends Definition {
    var $def_type   = 'pointer';
    var $name       = null;
    var $in_module  = null;
    var $c_name     = null;
    var $ce         = null;
    var $typecode   = null;
    var $ce_flags   = array();
    var $fields     = array();

    function Pointer_Def($args)
    {
        $this->name = array_shift($args);

        foreach ($args as $arg) {
            if (!is_array($arg) || count($arg) < 2)
                continue;

            if ($arg[0] == 'in-module')
                $this->in_module = $arg[1];
            else if ($arg[0] == 'c-name')
                $this->c_name = $arg[1];
            else if ($arg[0] == 'gtype-id')
                $this->typecode = $arg[1];
            else if ($arg[0] == 'fields')
                $this->fields = array_slice($arg, 1);
        }

        $this->ce = strtolower($this->c_name) . '_ce';
    }

    function write_defs($fp)
    {
        fwrite($fp, "(define-pointer $this->name\n");
        if ($this->in_module)
            fwrite($fp, "  (in-module \"$this->in_module\")\n");
        if ($this->c_name)
            fwrite($fp, "  (c-name \"$this->c_name\")\n");
        if ($this->typecode)
            fwrite($fp, "  (gtype-id \"$this->typecode\")\n");
        if ($this->fields) {
            fwrite($fp, "  (fields\n");
            foreach ($this->fields as $field) {
                fwrite($fp, "    '(\"$field[0]\" \"$field[1]\")\n");
            }
            fwrite($fp, "  )\n");
        }
        fwrite($fp, ")\n\n");
    }
}

/* vim: set et sts=4: */
?>
