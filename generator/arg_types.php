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

/*======================================================================*\
    Function: convert_typename
    Purpose:  Converts a typename to the uppercased and underscored
              version used for type conversion macros and enum/flag
              names.
\*======================================================================*/
function convert_typename($typename)
{
    preg_match_all('![A-Z]+[^A-Z]*!', $typename, $match);
    return '_' . strtoupper(implode('_', $match[0]));
}

/*======================================================================*\
    Function: enum_name
    Purpose:  Creates a GTK_TYPE_* name from the given typename
\*======================================================================*/
function enum_name($typename)
{
    $proper = convert_typename($typename);
    if (strlen($proper) > 4 && substr($proper, 0, 4) == '_GTK')
        return 'GTK_TYPE' . substr($proper, 4);
    else
        return 'GTK_TYPE' . $proper;
}

/*======================================================================*\
    Class:   Var_List
    Purpose: Format C variable list 
\*======================================================================*/
class Var_List {
    var $vars = array();

    function add($c_type, $name)
    {
        $this->vars[$c_type][] = $name;
    }

    function __tostring()
    {
        $result = array();
        foreach (array_keys($this->vars) as $c_type)
            $result[] = "\t$c_type " . implode(', ', $this->vars[$c_type]) .  ";\n";
        if (count($result)) {
            return implode('', $result);
        } else
            return '';
    }
}

class Wrapper_Info {
    var $var_list   = array();
    var $arg_list   = array();
    var $parse_list = array('');
    var $specifiers = '';
    var $pre_code   = array();
    var $post_code  = array();
    var $error_action  = 'return';

    function __construct()
    {
        $this->var_list = new Var_List;
    }

    function add_parse_list($specifiers, $parse_args = array())
    {
        $this->specifiers .= $specifiers;
        $this->parse_list = array_merge($this->parse_list, (array)$parse_args);
    }

    function get_var_list()
    {
        return $this->var_list->__tostring();
    }

    function get_arg_list()
    {
        return implode(', ', $this->arg_list);
    }
    
    function get_parse_list()
    {
        return implode(', ', $this->parse_list);
    }

    function get_pre_code()
    {
        return implode('', $this->pre_code);
    }

    function get_post_code()
    {
        return implode('', $this->post_code);
    }
}

/*======================================================================*\
    Class:    Arg_Type
    Purpose:  Base class for argument type handlers
\*======================================================================*/
class Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info, $in_constructor)
    {
        throw new Exception("write_param() not implemented for " . get_class($this));
    }
    
    function write_return($type, $owns_return, $info)
    {
        throw new Exception("write_return() not implemented for " . get_class($this));
    }

    /*
    function write_to_prop($obj, $name, $source)
    {
        trigger_error("This is an abstract class", E_USER_ERROR);
    }

    function write_from_prop($name, $type)
    {
        trigger_error("This is an abstract class", E_USER_ERROR);
    }
    */
}

/* {{{ None_Arg */
class None_Arg extends Arg_Type {
    function write_return($type, $owns_return, $info)
    {
        $this->post_code[] = "\tRETVAL_NULL();";
    }
}
/* }}} */

/* {{{ String_Arg */
class String_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default)) {
            if ($default != 'NULL')
                $default = '"' . $default . '"';
            $info->var_list->add('char', '*' . $name . ' = ' . $default);
        } else
            $info->var_list->add('char', '*' . $name);
        $info->var_list->add('zend_bool', 'free_' . $name . ' = FALSE');
        $info->add_parse_list('u', array("&$name", "&free_$name"));
        $info->arg_list[] = $name;
        $info->post_code[] = "\tif (free_$name) g_free($name);\n";
    }
    
    function write_return($type, $owns_return, $info)
    {
        if ($owns_return) {
            $info->var_list->add('gchar', '*php_retval');
            $info->var_list->add('gchar', '*cp_ret');
            $info->var_list->add('gsize', 'cp_len');
            $info->var_list->add('zend_bool', 'free_result');
            $info->post_code[] = 
                   "    if (php_retval) {\n"                        .
                   "        cp_ret = phpg_from_utf8(php_retval, strlen(php_retval), &cp_len, &free_result TSRMLS_CC);\n" .
                   "        if (cp_ret) {\n"                        .
                   "            RETVAL_STRINGL((char *)cp_ret, cp_len, 1);\n"    .
                   "        } else {\n"                             .
                   "            php_error(E_WARNING, \"%s::%s(): could not convert return value from UTF-8\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));\n" .
                   "        }\n"                                    .
                   "        g_free(php_retval);\n"                  .
                   "        if (free_result)\n"                     .
                   "            g_free(cp_ret);\n"                  .
                   "    } else\n"                                   .
                   "        RETVAL_NULL();";
        } else {
            $info->var_list->add('const gchar', '*php_retval');
            $info->var_list->add('gchar', '*cp_ret');
            $info->var_list->add('gsize', 'cp_len');
            $info->var_list->add('zend_bool', 'free_result');
            $info->post_code[] = 
                   "    if (php_retval) {\n"                           .
                   "        cp_ret = phpg_from_utf8(php_retval, strlen(php_retval), &cp_len, &free_result TSRMLS_CC);\n" .
                   "        if (cp_ret) {\n"                        .
                   "            RETVAL_STRINGL((char *)cp_ret, cp_len, 1);\n"    .
                   "        } else {\n"                             .
                   "            php_error(E_WARNING, \"%s::%s(): could not convert return value from UTF-8\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));\n" .
                   "        }\n"                                    .
                   "        if (free_result)\n"                        .
                   "            g_free(cp_ret);\n"                     .
                   "    } else {\n"                                    .
                   "        RETVAL_NULL();\n"                          .
                   "    }";
        }
    }
}
/* }}} */

/* {{{ Char_Arg */
class Char_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default))
            $info->var_list->add('char', $name . ' = \'' . $default . '\'');
        else
            $info->var_list->add('char', $name);
        $info->arg_list[] = $name;
        $info->add_parse_list('c', '&' . $name);
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('gchar', 'php_retval');
        $info->post_code[] =  "\tRETVAL_STRINGL((char*)&ret, 1, 1);";
    }
}
/* }}} */

/* {{{ Int_Arg */
class Int_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default))
            $info->var_list->add('long', $name . ' = ' . $default);
        else
            $info->var_list->add('long', $name);
        $info->arg_list[] = "($type)$name";
        $info->add_parse_list('i', '&' . $name);
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('long', 'php_retval');
        $info->post_code[] = "\tRETVAL_LONG(php_retval);";
    }

    function write_to_prop($obj, $name, $source)
    {
        return "    add_property_long($obj, \"$name\", $source);\n";
    }

    function write_from_prop($name, $type)
    {
        return "    if (zend_hash_find(Z_OBJPROP_P(wrapper), \"$name\", sizeof(\"$name\"), (void **)&item) == SUCCESS && Z_TYPE_PP(item) == IS_LONG)\n" .
               "        obj->$name = ($type)Z_LVAL_PP(item);\n" .
               "    else\n" .
               "        return 0;\n\n";

    }
}
/* }}} */

/* {{{ Bool_Arg */
class Bool_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default))
            $info->var_list->add('zend_bool', $name . ' = ' . $default);
        else
            $info->var_list->add('zend_bool', $name);
        $info->arg_list[] = "($type)$name";
        $info->add_parse_list('b', '&' . $name);
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('gboolean', 'php_retval');
        $info->post_code[] = "\tRETVAL_BOOL(php_retval);";
    }

    function write_to_prop($obj, $name, $source)
    {
        return "    add_property_bool($obj, \"$name\", $source);\n";
    }

    function write_from_prop($name, $type)
    {
        return "    if (zend_hash_find(Z_OBJPROP_P(wrapper), \"$name\", sizeof(\"$name\"), (void **)&item) == SUCCESS && Z_TYPE_PP(item) == IS_BOOL)\n" .
               "        obj->$name = ($type)Z_BVAL_PP(item);\n" .
               "    else\n" .
               "        return 0;\n\n";

    }
}
/* }}} */

/* {{{ Double_Arg */
class Double_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default))
            $info->var_list->add('double', $name . ' = ' . $default);
        else
            $info->var_list->add('double', $name);

        $info->add_parse_list('d', '&' . $name);

        if($type == 'gfloat')
          $info->arg_list[] = "(float)$name";
        else
          $info->arg_list[] = $name;
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('double', 'php_retval');
        $info->post_code[] = "\tRETVAL_DOUBLE(php_retval);";
    }

    function write_to_prop($obj, $name, $source)
    {
        return "    add_property_double($obj, \"$name\", $source);\n";
    }

    function write_from_prop($name, $type)
    {
        return "    if (zend_hash_find(Z_OBJPROP_P(wrapper), \"$name\", sizeof(\"$name\"), (void **)&item) == SUCCESS && Z_TYPE_PP(item) == IS_DOUBLE)\n" .
               "        obj->$name = ($type)Z_DVAL_PP(item);\n" .
               "    else\n" .
               "        return 0;\n\n";

    }
}
/* }}} */

/* {{{ Enum_Arg */
class Enum_Arg extends Arg_Type {
    static $enum_tpl  = "\n\tif (php_%(name) && phpg_gvalue_get_enum(%(typecode), php_%(name), (gint *)&%(name)) == FAILURE) {\n\t\t%(on_error);\n\t}\n";
    var $enum_name = null;
    var $typecode = null;

    function __construct($enum_name, $typecode)
    {
        $this->enum_name = $enum_name;
        $this->typecode = $typecode;
    }

    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default))
            $info->var_list->add($this->enum_name, $name . ' = ' . $default);
        else
            $info->var_list->add($this->enum_name, $name);
        $info->var_list->add('zval', '*php_' . $name . ' = NULL');
        $info->arg_list[] = $name;
        $info->add_parse_list('V', '&php_' . $name);
        $info->pre_code[] = aprintf(self::$enum_tpl,
                                    array('typecode' => $this->typecode,
                                          'name' => $name,
                                          'on_error' => $info->error_action));
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('long', 'php_retval');
        $info->post_code[] = "\tRETVAL_LONG(php_retval);";
    }
}
/* }}} */

/* {{{ Flags_Arg */
class Flags_Arg extends Arg_Type {
    static $flag_tpl = "\n\tif (php_%(name) && phpg_gvalue_get_flags(%(typecode), php_%(name), (gint *)&%(name)) == FAILURE) {\n\t\t%(on_error);\n\t}\n";
    var $flag_name = null;
    var $typecode = null;

    function __construct($flag_name, $typecode)
    {
        $this->flag_name = $flag_name;
        $this->typecode = $typecode;
    }

    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default))
            $info->var_list->add($this->flag_name, $name . ' = ' . $default);
        else
            $info->var_list->add($this->flag_name, $name);
        $info->var_list->add('zval', '*php_' . $name . ' = NULL');
        $info->arg_list[] = $name;
        $info->add_parse_list('V', '&php_' . $name);
        $info->pre_code[] = aprintf(self::$flag_tpl, array('typecode' => $this->typecode,
                                                           'name' => $name,
                                                           'on_error' => $info->error_action));
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('long', 'php_retval');
        $info->post_code[] = "\tRETVAL_LONG(php_retval);";
    }
}
/* }}} */

/* {{{ Struct_Arg */
class Struct_Arg extends Arg_Type {
    var $struct_name = null;

    function __construct($struct_name)
    {
        $this->struct_name = $struct_name;
        $this->struct_tpl   =  "    if (!php_%s_get(php_%s, &%s)) {\n" .
                               "        %sreturn;\n" .
                               "    }\n\n";
        $this->struct_def_tpl   =   "   if (php_%s && !php_%s_get(php_%s, &%s)) {\n" .
                                    "       %sreturn;\n" .
                                    "   }\n\n";
    }

    function write_param($type, $name, $default, $null_ok, &$var_list,
                         &$parse_list, &$arg_list, &$extra_pre_code, &$extra_post_code, $in_constructor)
    {
        $typename = strtolower(substr(convert_typename($this->struct_name), 1));
        $var_list->add($this->struct_name, $name);
        $parse_list[]   = '&php_' . $name . ', ' . $typename . '_ce';

        if (isset($default) && $default == 'NULL') {
            $var_list->add('zval', '*php_' . $name . ' = NULL');
            $arg_list[]     = '(php_' . $name . ' ? &'. $name . ' : NULL)';
            $extra_pre_code[] = sprintf($this->struct_def_tpl, $name, $typename, $name, $name,
                                        $in_constructor ?  "php_gtk_invalidate(this_ptr);\n\t\t" : "");
        } else {
            $var_list->add('zval', '*php_' . $name);
            $arg_list[]     = '&' . $name;
            $extra_pre_code[] = sprintf($this->struct_tpl, $typename, $name, $name,
                                        $in_constructor ?  "php_gtk_invalidate(this_ptr);\n\t\t" : "");
        }

        return 'O';
    }

    function write_return($type, &$var_list, $separate)
    {
        $typename = strtolower(substr(convert_typename($this->struct_name), 1));
        return  "   *return_value = *php_{$typename}_new(" . ((substr($type, -1) != '*') ? '&' : '') . "%s);\n" .
                "   return;";
    }
}
/* }}} */

/* {{{ Pointer_Arg */
class Pointer_Arg extends Arg_Type {

    const check_tpl = "
    if (phpg_gpointer_check(php_%(name), %(typecode), FALSE TSRMLS_CC)) {
        %(name) = (%(type) *) PHPG_GPOINTER(php_%(name));
    } else {
        php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a valid %(type) object\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        %(on_error);
    }\n";

    const check_null_tpl = "
    if (Z_TYPE_P(php_%(name)) != IS_NULL) {
        if (phpg_gpointer_check(php_%(name), %(typecode), FALSE TSRMLS_CC)) {
            %(name) = (%(type) *) PHPG_GPOINTER(php_%(name));
        } else {
            php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a valid %(type) object or null\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
            %(on_error);
        }
    }\n";

    var $type           = null;
    var $typecode       = null;

    function __construct($type, $typecode)
    {
        $this->type     = $type;
        $this->typecode = $typecode;
    }

    function write_param($type, $name, $default, $null_ok, $info)
    {
        if ($null_ok) {
            $info->var_list->add($this->type, '*' . $name . ' = NULL');
            $info->var_list->add('zval', '*php_' . $name . ' = NULL');
            $info->pre_code[] = aprintf(self::check_null_tpl, array('typecode' => $this->typecode,
                                                                    'type'     => $this->type,
                                                                    'on_error' => $info->error_action,
                                                                    'name'     => $name));
        } else {
            $info->var_list->add($this->type, '*' . $name . ' = NULL');
            $info->var_list->add('zval', '*php_' . $name);
            $info->pre_code[] = aprintf(self::check_tpl, array('typecode' => $this->typecode,
                                                               'type'     => $this->type,
                                                               'on_error' => $info->error_action,
                                                               'name'     => $name));
        }

        $info->add_parse_list('O', array('&php_' . $name, 'gpointer_ce'));
        $info->arg_list[] = $name;
    }

    function write_return($type, $owns_return, $info)
    {
        if (substr($type, -1) == '*') {
            $info->var_list->add($this->type, '*php_retval');
            $ret = 'php_retval';
        } else {
            $info->var_list->add($this->type, 'php_retval');
            $ret = '&php_retval';
        }

        $info->post_code[] = sprintf("\tphpg_gpointer_new(&return_value, %s, %s TSRMLS_CC);\n",
                                     $this->typecode, $ret);
    }
}
/* }}} */

/* {{{ Object_Arg */
class Object_Arg extends Arg_Type {
    var $obj_name = null;
    var $cast     = null;
    var $obj_ce   = null;
    //var $getter   = null;
    //var $gtk_object_descendant;

    function __construct($obj_name, $typecode)
    {
        $this->obj_name = $obj_name;
        $this->obj_ce = strtolower($obj_name) . '_ce';
        $this->cast = preg_replace('!_TYPE_!', '_', $typecode, 1);
        /*
        if ($gtk_object_descendant) {
            $this->getter = $this->typename . "(PHP_GTK_GET(%s))";
        } else {
            $this->getter = "PHP_" . $this->typename . "_GET(%s)";
        }
        */
        //$this->gtk_object_descendant = $gtk_object_descendant;
    }

    function write_param($type, $name, $default, $null_ok, $info)
    {
        if ($null_ok) {
            if (isset($default)) {
                $info->var_list->add($this->obj_name, '*' . $name . ' = ' . $default);
                $info->var_list->add('zval', '*php_' . $name . ' = NULL');
                $info->pre_code[] = "    if (php_$name) {\n" .
                                    "        if (Z_TYPE_P(php_$name) == IS_NULL)\n" .
                                    "            $name = NULL;\n" .
                                    "        else\n" .
                                    "            $name = $this->cast(PHPG_GOBJECT(php_$name));\n" .
                                    "    }\n";
            } else {
                $info->var_list->add($this->obj_name, '*' . $name . ' = NULL');
                $info->var_list->add('zval', '*php_' . $name);
                $info->pre_code[] = "    if (Z_TYPE_P(php_$name) != IS_NULL)\n" .
                                    "        $name = $this->cast(PHPG_GOBJECT(php_$name));\n";
            }

            $info->add_parse_list('N', array('&php_' . $name, $this->obj_ce));
            $info->arg_list[] = $name;
        } else {
            if (isset($default)) {
                $info->var_list->add($this->obj_name, '*' . $name . ' = ' . $default);
                $info->var_list->add('zval', '*php_' . $name . ' = NULL');
                $info->add_parse_list('O', array('&php_' . $name, $this->obj_ce));
                $info->arg_list[] = $name;
                $info->pre_code[] = "    if (php_$name)\n" .
                                    "        $name = $this->cast(PHPG_GOBJECT(php_$name));\n";
            } else {
                $info->var_list->add('zval', '*' . $name);
                $info->add_parse_list('O', array('&' . $name, $this->obj_ce));
                $info->arg_list[] = "$this->cast(PHPG_GOBJECT($name))";
            }
        }
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add($type, 'php_retval');
        if ($owns_return) {
            $info->post_code[] = "    phpg_gobject_new(&return_value, (GObject *)php_retval TSRMLS_CC);\n" .
                                 "    if (php_retval != NULL) {\n" .
                                 "        g_object_unref(php_retval);\n" .
                                 "    }";
        } else {
            $info->post_code[] = "    phpg_gobject_new(&return_value, (GObject *)php_retval TSRMLS_CC);";
        }
    }
}
/* }}} */

/* {{{ Boxed_Arg */
class Boxed_Arg extends Arg_Type {
    const check_tpl = "
    if (phpg_gboxed_check(php_%(name), %(typecode), FALSE TSRMLS_CC)) {
        %(name) = (%(typename) *) PHPG_GBOXED(php_%(name));
    } else {
        php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a valid %(typename) object\",
                  get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        %(on_error);
    }\n";
    const check_default_tpl = "
    if (php_%(name)) {
        if (phpg_gboxed_check(php_%(name), %(typecode), FALSE TSRMLS_CC)) {
            %(name) = (%(typename) *) PHPG_GBOXED(php_%(name));
        } else {
            php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a valid %(typename) object\",
                      get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
            %(on_error);
        }
    }\n";
    const check_null_tpl = "
    if (Z_TYPE_P(php_%(name)) != IS_NULL) {
        if (phpg_gboxed_check(php_%(name), %(typecode), FALSE TSRMLS_CC)) {
            %(name) = (%(typename) *) PHPG_GBOXED(php_%(name));
        } else {
            php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a valid %(typename) object or null\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
            %(on_error);
        }
    }\n";
    const check_null_default_tpl = "
    if (php_%(name)) {
        if (Z_TYPE_P(php_%(name)) == IS_NULL) {
            %(name) = NULL;
        } else {
            if (phpg_gboxed_check(php_%(name), %(typecode), FALSE TSRMLS_CC)) {
                %(name) = (%(typename) *) PHPG_GBOXED(php_%(name));
            } else {
                php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a valid %(typename) object or null\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
                %(on_error);
            }
        }
    }\n";
    var $boxed_type  = null;
    var $typecode    = null;

    function Boxed_Arg($boxed_type, $typecode)
    {
        $this->boxed_type   = $boxed_type;
        $this->typecode     = $typecode;
    }

    function write_param($type, $name, $default, $null_ok, $info)
    {
        if ($null_ok) {
            if (isset($default)) {
                $info->var_list->add($this->boxed_type, '*' . $name . ' = ' . $default);
                $info->var_list->add('zval', '*php_' . $name . ' = NULL');
                $info->pre_code[] = aprintf(self::check_null_default_tpl, array('typecode' => $this->typecode,
                                                                                'typename' => $this->boxed_type,
                                                                                'on_error' => $info->error_action,
                                                                                'name' => $name));
            } else {
                $info->var_list->add($this->boxed_type, '*' . $name . ' = NULL');
                $info->var_list->add('zval', '*php_' . $name);
                $info->pre_code[] = aprintf(self::check_null_tpl, array('typecode' => $this->typecode,
                                                                        'typename' => $this->boxed_type,
                                                                        'on_error' => $info->error_action,
                                                                        'name' => $name));
            }

            $info->add_parse_list('N', array('&php_' . $name, 'gboxed_ce'));
        } else {
            if (isset($default)) {
                $info->var_list->add($this->boxed_type, '*' . $name . ' = ' . $default);
                $info->var_list->add('zval', '*php_' . $name . ' = NULL');
                $info->pre_code[] = aprintf(self::check_default_tpl, array('typecode' => $this->typecode,
                                                                           'typename' => $this->boxed_type,
                                                                           'on_error' => $info->error_action,
                                                                           'name' => $name));
            } else {
                $info->var_list->add($this->boxed_type, '*' . $name . ' = NULL');
                $info->var_list->add('zval', '*php_' . $name);
                $info->pre_code[] = aprintf(self::check_tpl, array('typecode' => $this->typecode,
                                                                   'typename' => $this->boxed_type,
                                                                   'on_error' => $info->error_action,
                                                                   'name' => $name));
            }

            $info->add_parse_list('O', array('&php_' . $name, 'gboxed_ce'));
        }

        $typename = preg_replace('!^(const-)?([^*]+)(\*)?$!', '$2', $type);
        if ($typename != $this->boxed_type) {
            $info->arg_list[] = '(' . substr($type, 0, -1) . ' *)' . $name;
        } else {
            $info->arg_list[] = $name;
        }
    }

    function write_return($type, $owns_return, $info)
    {
        if (substr($type, -1) == '*') {
            $info->var_list->add($this->boxed_type, '*php_retval');
            $ret = 'php_retval';
        } else {
            $info->var_list->add($this->boxed_type, 'php_retval');
            $ret = '&php_retval';
            /* can't own reference to a local var */
            $owns_return = false;
        }

        $info->post_code[] = sprintf("\tphpg_gboxed_new(&return_value, %s, %s, %s, TRUE TSRMLS_CC);\n",
                                     $this->typecode, $ret, $owns_return ?  'FALSE' : 'TRUE');
    }
}
/* }}} */

/* {{{ Atom_Arg */
class Atom_Arg extends Int_Arg {
    const atom_tpl = "
    %(name) = phpg_gdkatom_from_zval(php_%(name) TSRMLS_CC);
    if (%(name) == NULL) {
        php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a valid GdkAtom object\",
                  get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        %(on_error);
    }\n";

    const atom_default_tpl = "
    if (php_%(name)) {
        %(name) = phpg_gdkatom_from_zval(php_%(name) TSRMLS_CC);
        if (%(name) == NULL) {
            php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a valid GdkAtom object\",
                      get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
            %(on_error);
        }
    }\n";

    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default)) {
            $info->var_list->add('GdkAtom', $name . ' = ' . $default);
            $info->var_list->add('zval', '*php_' . $name . ' = NULL');
            $info->pre_code[] = aprintf(self::atom_default_tpl, array('name' => $name,
                                                                      'on_error' => $info->error_action));
        } else {
            $info->var_list->add('GdkAtom', $name);
            $info->var_list->add('zval', '*php_' . $name . ' = NULL');
            $info->pre_code[] = aprintf(self::atom_tpl, array('name' => $name,
                                                              'on_error' => $info->error_action));
        }
        $info->arg_list[] = $name;
        $info->add_parse_list('V', '&php_' . $name);
    }

    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('GdkAtom', 'php_retval');
        $info->post_code[] = "\tphpg_gdkatom_new(&return_value, php_retval TSRMLS_CC);";
    }
}
/* }}} */

/* {{{ Drawable_Arg */
/* This is a hack -- the $default handling doesn't work, neither does
   write_return(). */ 
class Drawable_Arg extends Arg_Type {
    var $type = 'GdkDrawable';

    function write_param($type, $name, $default, $null_ok, &$var_list,
                         &$parse_list, &$arg_list, &$extra_code, $in_constructor)
    {
        if ($null_ok) {
            if (isset($default)) {
                $var_list->add($this->type, '*' . $name . ' = ' . $default);
                $var_list->add('zval', '*php_' . $name . ' = NULL');
                $extra_code[]   =   "   if (php_$name) {\n" .
                                    "       if (Z_TYPE_P(php_$name) == IS_NULL)\n" .
                                    "           $name = NULL;\n" .
                                    "       else if (php_gtk_check_class(php_$name, gdk_window_ce))\n" .
                                    "           $name = (GdkDrawable *)PHP_GDK_WINDOW_GET(php_$name);\n" .
                                    "       else if(php_gtk_check_class(php_$name, gdk_pixmap_ce))\n" .
                                    "           $name = (GdkDrawable *)PHP_GDK_PIXMAP_GET(php_$name);\n" .
                                    "       else if(php_gtk_check_class(php_$name, gdk_bitmap_ce))\n" .
                                    "           $name = (GdkDrawable *)PHP_GDK_BITMAP_GET(php_$name);\n" .
                                    "   }\n";
            } else {
                $var_list->add($this->type, '*' . $name);
                $var_list->add('zval', '*php_' . $name);
                $extra_code[]   =
                    "   if (Z_TYPE_P(php_$name) == IS_NULL)
                            $name = NULL;
                        if (php_gtk_check_class(php_$name, gdk_window_ce))
                            $name = (GdkDrawable *)PHP_GDK_WINDOW_GET(php_$name);
                        else if(php_gtk_check_class(php_$name, gdk_pixmap_ce))
                            $name = (GdkDrawable *)PHP_GDK_PIXMAP_GET(php_$name);
                        else if(php_gtk_check_class(php_$name, gdk_bitmap_ce))
                            $name = (GdkDrawable *)PHP_GDK_BITMAP_GET(php_$name);
                        else {
                                php_error(E_WARNING, \"%s() expects the drawable to be GdkWindow, GdkPixmap, GdkBitmap, or null\", get_active_function_name(TSRMLS_C));
                                return;
                        }
                ";
            }

            $parse_list[]   = '&php_' . $name;
            $arg_list[]     = $name;

            return 'V';
        } else {
            if (isset($default)) {
                $var_list->add($this->type, '*' . $name . ' = ' . $default);
                $var_list->add('zval', '*php_' . $name . ' = NULL');

                $parse_list[]   = '&php_' . $name;
                $parse_list[]   = $this->php_type . '_ce';
                $arg_list[]     = $name;
                $extra_code[]   =   "   if (php_$name)\n" .
                                    "       $name = PHP_" .  strtoupper($this->php_type) . "_GET(" . $name . ");\n";
            } else {
                $var_list->add($this->type, '*' . $name);
                $var_list->add('zval', '*php_' . $name);
                $parse_list[]   = '&php_' . $name;
                $arg_list[]     = $name;
                $extra_code[]   =
                    "   if (php_gtk_check_class(php_$name, gdk_window_ce))
                            $name = (GdkDrawable *)PHP_GDK_WINDOW_GET(php_$name);
                        else if(php_gtk_check_class(php_$name, gdk_pixmap_ce))
                            $name = (GdkDrawable *)PHP_GDK_PIXMAP_GET(php_$name);
                        else if(php_gtk_check_class(php_$name, gdk_bitmap_ce))
                            $name = (GdkDrawable *)PHP_GDK_BITMAP_GET(php_$name);
                        else {
                                php_error(E_WARNING, \"%s() expects the drawable to be GdkWindow, GdkPixmap, or GdkBitmap\", get_active_function_name(TSRMLS_C));
                                return;
                        }
                ";
            }

            return 'V';
        }
    }

    function write_return($type, &$var_list, $separate)
    {
        $var_list->add('zval', '*ret');
        return "    ret = php_" . $this->php_type . "_new(%s);\n" .
               ($separate ? "   SEPARATE_ZVAL(&ret);\n" : "") .
               "    *return_value = *ret;\n" .
               "    return;";
    }
}
/* }}} */

/* {{{ GdkRectangle_Arg */
class GdkRectangle_Arg extends Arg_Type {
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('GdkRectangle', 'php_retval');
        $info->post_code[] = "\tphpg_gboxed_new(&return_value, GDK_TYPE_RECTANGLE, &php_retval, TRUE, TRUE TSRMLS_CC);\n";
    }
}
/* }}} */

/* {{{ GdkRectanglePtr_Arg */
class GdkRectanglePtr_Arg extends Arg_Type {
    const check_tpl = "
    if (phpg_rectangle_from_zval(php_%(name), &%(name) TSRMLS_CC) == FAILURE) {
        php_error(E_WARNING, \"%s::%s() expects %(name) argument to be either a 4-element array or a GdkRectangle object\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        %(on_error);
    }";
    const check_null_tpl = "
    if (Z_TYPE_P(php_%(name)) == IS_NULL) {
        %(name) = NULL;
    } else if (phpg_rectangle_from_zval(php_%(name), &%(name)_arg TSRMLS_CC) == SUCCESS) {
        %(name) = &%(name)_arg;
    } else {
        php_error(E_WARNING, \"%s::%s() expects %(name) argument to be a 4-element array, a GdkRectangle object, or null\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        %(on_error);
    }";
    function write_param($type, $name, $default, $null_ok, $info)
    {
        if ($null_ok) {
            $info->var_list->add('GdkRectangle', $name . '_arg = { 0, 0, 0, 0 }');
            $info->var_list->add('GdkRectangle', '*' . $name);
            $info->var_list->add('zval', '*php_' . $name . ' = NULL');
            $info->add_parse_list('V', '&php_' . $name);
            $info->arg_list[] = $name;
            $info->pre_code[] = aprintf(self::check_null_tpl, array('name' => $name,
                                                                    'on_error' => $info->error_action));
        } else {
            $info->var_list->add('GdkRectangle', $name . ' = { 0, 0, 0, 0 }');
            $info->var_list->add('zval', '*php_' . $name);
            $info->add_parse_list('V', '&php_' . $name);
            $info->arg_list[] = '&' . $name;
            $info->pre_code[] = aprintf(self::check_tpl, array('name' => $name,
                                                               'on_error' => $info->error_action));
        }
    }
}
/* }}} */

/* {{{ GType_Arg */
class GType_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        $info->var_list->add('GType', $name);
        $info->var_list->add('zval', '*php_' . $name . ' = NULL');
        $info->arg_list[] = $name;
        $info->add_parse_list('V', '&php_' . $name);
        $info->pre_code[] = "    if (($name = phpg_gtype_from_zval(php_$name)) == 0) {\n" .
                            "        $info->error_action;\n" .
                            "    }\n";
    }

    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('GType', 'php_retval');
        $info->post_code[] = "    phpg_gtype_new(return_value, php_retval TSRMLS_CC);\n";
    }
}
/* }}} */

/* {{{ GError_Arg */
class GError_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        $info->var_list->add('GError', '*' . $name . ' = NULL');
        $info->arg_list[] = '&' . $name;
        $info->post_code[] = "    if (phpg_handle_gerror(&$name TSRMLS_CC)) {\n" .
                             "        return;\n" .
                             "    }\n";
    }
}
/* }}} */

/* {{{ GtkTreePath_Arg */
class GtkTreePath_Arg extends Arg_Type {
    const normal_tpl = "
    if (phpg_tree_path_from_zval(php_%(name), &%(name) TSRMLS_CC) == FAILURE) {
        php_error(E_WARNING, \"%s::%s() expects %(name) to be a valid tree path specification\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
        %(on_error);
    }\n";
    const null_tpl = "
    if (php_%(name) && Z_TYPE_P(php_%(name)) != IS_NULL) {
        if (phpg_tree_path_from_zval(php_%(name), &%(name) TSRMLS_CC) == FAILURE) {
            php_error(E_WARNING, \"%s::%s() expects %(name) to be a valid tree path specification\", get_active_class_name(NULL TSRMLS_CC), get_active_function_name(TSRMLS_C));
            %(on_error);
        }
    }\n";
    const free_path_tpl = "
    if (%(name))
        gtk_tree_path_free(%(name));\n";

    function write_param($type, $name, $default, $null_ok, $info)
    {
        if ($null_ok) {
            $info->var_list->add('GtkTreePath', '*' . $name . ' = NULL');
            $info->var_list->add('zval', '*php_' . $name . ' = NULL');
            $info->arg_list[] = $name;
            $info->add_parse_list('V', '&php_' . $name);
            $info->pre_code[] = aprintf(self::null_tpl, array('name' => $name,
                                                              'on_error' => $info->error_action));
            $info->post_code[] = aprintf(self::free_path_tpl, array('name' => $name));
        } else {
            $info->var_list->add('GtkTreePath', '*' . $name);
            $info->var_list->add('zval', '*php_' . $name);
            $info->arg_list[] = $name;
            $info->add_parse_list('V', '&php_' . $name);
            $info->pre_code[] = aprintf(self::normal_tpl, array('name' => $name,
                                                                'on_error' => $info->error_action));
            $info->post_code[] = aprintf(self::free_path_tpl, array('name' => $name));
        }
    }

    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('GtkTreePath', '*php_retval');
        if ($owns_return) {
            $info->post_code[] = "
    if (php_retval) {
        phpg_tree_path_to_zval(php_retval, &return_value TSRMLS_CC);
        gtk_tree_path_free(php_retval);
    }\n";
        } else {
            $info->post_code[] = "
    if (php_retval) {
        phpg_tree_path_to_zval(php_retval, &return_value TSRMLS_CC);
    }\n";
        }
    }
}
/* }}} */

/* {{{ Arg_Matcher */
class Arg_Matcher {
    var $arg_types = array();

    function register($type, $handler)
    {
        $this->arg_types[$type] = $handler;
    }

    function register_enum($type, $typecode = null)
    {
        if ($typecode === null) {
            $typecode = 'G_TYPE_NONE';
        }
        $this->register($type, new Enum_Arg($type, $typecode));
    }

    function register_flag($type, $typecode = null)
    {
        if ($typecode === null) {
            $typecode = 'G_TYPE_NONE';
        }
        $this->register($type, new Flags_Arg($type, $typecode));
    }

    function register_struct($type)
    {
        $struct_arg = new Struct_Arg($type);
        $this->register($type, $struct_arg);
        $this->register($type . '*', $struct_arg);
        $this->register('const-' . $type . '*', $struct_arg);
    }

    function register_object($type, $typecode)
    {
        $obj_arg = new Object_Arg($type, $typecode);
        $this->register($type, $obj_arg);
        $this->register($type . '*', $obj_arg);
        if ($type == 'GdkPixmap') {
            $this->register('GdkBitmap', $obj_arg);
            $this->register('GdkBitmap*', $obj_arg);
        }
    }

    function register_boxed($type, $typecode)
    {
        if (isset($this->arg_types[$type])) return;
        $boxed_arg = new Boxed_Arg($type, $typecode);
        $this->register($type, $boxed_arg);
        $this->register($type . '*', $boxed_arg);
        $this->register('const-' . $type . '*', $boxed_arg);
    }

    function register_pointer($type, $typecode)
    {
        if (isset($this->arg_types[$type])) return;
        $pointer_arg = new Pointer_Arg($type, $typecode);
        $this->register($type, $pointer_arg);
        $this->register($type.'*', $pointer_arg);
        $this->register('const-'.$type.'*', $pointer_arg);
    }

    function get($type)
    {
        if (isset($this->arg_types[$type])) {
            return $this->arg_types[$type];
        } else {
            if (substr($type, 0, 8) == 'GdkEvent' && substr($type, -1) == '*') {
                return $this->arg_types['GdkEvent*'];
            }
            throw new Exception("unknown type '$type'");
        }
    }
}
/* }}} */

/* {{{ type registration */
$matcher = new Arg_Matcher();

$arg = new None_Arg();
$matcher->register(null, $arg);
$matcher->register('none', $arg);

$arg = new String_Arg();
$matcher->register('char*', $arg);
$matcher->register('gchar*', $arg);
$matcher->register('const-char*', $arg);
$matcher->register('const-gchar*', $arg);
$matcher->register('string', $arg);
$matcher->register('static_string', $arg);
$matcher->register('unsigned-char*', $arg);
$matcher->register('guchar*', $arg);
$matcher->register('const-guchar*', $arg);

$arg = new Char_Arg();
$matcher->register('char', $arg);
$matcher->register('gchar', $arg);
$matcher->register('guchar', $arg);

$arg = new Int_Arg();
$matcher->register('int', $arg);
$matcher->register('gint', $arg);
$matcher->register('guint', $arg);
$matcher->register('short', $arg);
$matcher->register('gshort', $arg);
$matcher->register('gushort', $arg);
$matcher->register('long', $arg);
$matcher->register('glong', $arg);
$matcher->register('gulong', $arg);
$matcher->register('GdkNativeWindow', $arg);

$matcher->register('guint8', $arg);
$matcher->register('gint8', $arg);
$matcher->register('guint16', $arg);
$matcher->register('gint16', $arg);
$matcher->register('guint32', $arg);
$matcher->register('gint32', $arg);
$matcher->register('GtkType', $arg);

$arg = new Bool_Arg();
$matcher->register('gboolean', $arg);

$arg = new Double_Arg();
$matcher->register('double', $arg);
$matcher->register('gdouble', $arg);
$matcher->register('float', $arg);
$matcher->register('gfloat', $arg);

$arg = new GdkRectanglePtr_Arg();
$matcher->register('GdkRectangle*', $arg);
$matcher->register('GtkAllocation*', $arg);

$matcher->register('GdkRectangle', new GdkRectangle_Arg);


/* TODO
 * GdkRectangle(Ptr) args and others
 */

$matcher->register('GdkAtom', new Atom_Arg());

#$arg = new Drawable_Arg();
#$matcher->register('GdkDrawable*', $arg);
#
#$matcher->register_boxed('GdkWindow', 'gdk_window');
#$matcher->register_boxed('GdkPixmap', 'gdk_pixmap');
#$matcher->register_boxed('GdkBitmap', 'gdk_bitmap');
#$matcher->register_boxed('GdkColor', 'gdk_color');
#$matcher->register_boxed('GdkColormap', 'gdk_colormap');
#$matcher->register_boxed('GdkCursor', 'gdk_cursor');
#$matcher->register_boxed('GdkVisual', 'gdk_visual');
#$matcher->register_boxed('GdkFont', 'gdk_font');
#$matcher->register_boxed('GdkGC', 'gdk_gc');
#$matcher->register_boxed('GdkDragContext', 'gdk_drag_context');
#$matcher->register_boxed('GtkSelectionData', 'gtk_selection_data');
#$matcher->register_boxed('GtkCTreeNode', 'gtk_ctree_node');
#$matcher->register_boxed('GtkAccelGroup', 'gtk_accel_group');
#$matcher->register_boxed('GtkStyle', 'gtk_style');

$matcher->register_object('GObject', 'G_TYPE_OBJECT');
$matcher->register('GType', new GType_Arg);
$matcher->register('GError**', new GError_Arg);
$matcher->register('GtkTreePath*', new GtkTreePath_Arg);

/* }}} */

/* vim: set et sts=4 fdm=marker: */
?>
