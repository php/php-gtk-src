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
    Purpose:  Creates a GTK_TYPE_* name from the give typename
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
            $result[] = "\n";
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

    function Wrapper_Info()
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

class None_Arg extends Arg_Type {
    function write_return($type, $owns_return, $info)
    {
        return "\tRETURN_NULL();";
    }
}

class String_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default)) {
            if ($default != 'NULL')
                $default = '"' . $default . '"';
            $info->var_list->add('char', '*' . $name . ' = ' . $default);
        } else
            $info->var_list->add('char', '*' . $name);
        $info->add_parse_list('s', '&' . $name);

        if (strtoupper(substr(PHP_OS, 0,3) == "WIN")) {
            $info->var_list->add('gchar', '*utf8_' . $name . ' = NULL');
            $info->arg_list[]  = 'utf8_' . $name;
            $info->post_code[] = "\tif (utf8_$name) g_free(utf8_$name);\n";
            $info->pre_code[]  = "\tif ($name) utf8_$name = g_convert($name, strlen($name), \"UTF-8\", GTK_G(codepage), NULL, NULL, NULL);\n";
        } else
            $info->arg_list[] = $name;
    }
    
    function write_return($type, $owns_return, $info)
    {
        if (!$owns_return) {
            $info->var_list->add('const gchar', '*php_retval');
            if (strtoupper(substr(PHP_OS, 0,3) == "WIN")) {
                $info->var_list->add('gchar', '*cp_ret');
                $info->post_code[] = 
                       "    if (php_retval) {\n"                           .
                       "        cp_ret = g_convert(php_retval, strlen(php_retval), GTK_G(codepage), \"UTF-8\", NULL, NULL, NULL);\n" .
                       "        RETURN_STRING((char *)cp_ret, 1);\n"       .
                       "        g_free(cp_ret);\n"                         .
                       "    } else {\n"                                    .
                       "        RETURN_NULL();\n"                          .
                       "    }";
            } else {
                $info->post_code[] = 
                       "    if (php_retval) {\n"                           .
                       "        RETURN_STRING((char *)php_retval, 1);\n"   .
                       "    } else {\n"                                    .
                       "        RETURN_NULL();\n"                          .
                       "    }";
            }

        } else {
            $info->var_list->add('gchar', '*php_retval');
            if (strtoupper(substr(PHP_OS, 0,3) == "WIN")) {
                $info->var_list->add('gchar', '*cp_ret');
                $info->post_code[] = 
                       "    if (php_retval) {\n"                        .
                       "        cp_ret = g_convert(php_retval, strlen(php_retval), GTK_G(codepage), \"UTF-8\", NULL, NULL, NULL);\n" .
                       "        RETURN_STRING((char *)cp_ret, 1);\n"    .
                       "        g_free(cp_ret);\n"                      .
                       "        g_free(php_retval);\n"                  .
                       "    } else\n"                                   .
                       "        RETURN_NULL();";
            }
            else {
                $info->post_code[] = 
                       "    if (php_retval) {\n"                   .
                       "        RETURN_STRING(php_retval, 1);\n"   .
                       "        g_free(php_retval);\n"             .
                       "    } else\n"                              .
                       "        RETURN_NULL();";
            }
        }
    }
}

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
        $info->post_code[] =  "\tRETURN_STRINGL((char*)&ret, 1, 1);";
    }
}

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
        $info->post_code[] = "\tRETURN_LONG(php_retval);";
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
        $info->post_code[] = "\tRETURN_BOOL(php_retval);";
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

class Double_Arg extends Arg_Type {
    function write_param($type, $name, $default, $null_ok, $info)
    {
        if (isset($default))
            $info->var_list->add('double', $name . ' = ' . $default);
        else
            $info->var_list->add('double', $name);
        $info->arg_list[] = "(float)$name";
        $info->add_parse_list('d', '&' . $name);
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('double', 'php_retval');
        $info->post_code[] = "\tRETURN_DOUBLE(php_retval);";
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

class Enum_Arg extends Arg_Type {
    static $enum_tpl  = "\n\tif (php_%(name) && !phpg_gvalue_get_enum(%(typecode), php_%(name), (gint *)&%(name))) {\n\t\treturn;\n\t}\n";
    var $enum_name = null;
    var $typecode = null;
    //var $simple    = null;

    function Enum_Arg($enum_name, $typecode)
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
        $info->pre_code[] = aprintf(self::$enum_tpl, array('typecode' => $this->typecode,
                                                           'name' => $name));
        /*
        if ($this->simple) {
            $enum_tpl = "   if (php_%s && !php_gtk_get_simple_enum_value(php_%s, (gint *)&%s)) {\n" .
                        "       %sreturn;\n" .
                        "   }\n\n";
            $extra_pre_code[]   = sprintf($enum_tpl, $name, $name, $name,
                                          $in_constructor ?  "php_gtk_invalidate(this_ptr);\n\t\t" : "");
        } else {
            $enum_tpl = "   if (php_%s && !php_gtk_get_enum_value(%s, php_%s, (gint *)&%s)) {\n" .
                        "       %sreturn;\n" .
                        "   }\n\n";
            $extra_pre_code[]   = sprintf($enum_tpl, $name, $this->type_code, $name, $name,
                                          $in_constructor ?  "php_gtk_invalidate(this_ptr);\n\t\t" : "");
        }
        */
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('long', 'php_retval');
        $info->post_code[] = "RETURN_LONG(php_retval);";
    }
}

class Flags_Arg extends Arg_Type {
    static $flag_tpl = "\n\tif (php_%(name) && !phpg_gvalue_get_flags(%(typecode), php_%(name), (gint *)&%(name))) {\n\t\treturn;\n\t}\n";
    var $flag_name = null;
    var $typecode = null;

    function Flags_Arg($flag_name, $typecode)
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
                                                           'name' => $name));
    }
    
    function write_return($type, $owns_return, $info)
    {
        $info->var_list->add('long', 'php_retval');
        $info->post_code[] = "RETURN_LONG(php_retval);";
    }
}

class Struct_Arg extends Arg_Type {
    var $struct_name = null;

    function Struct_Arg($struct_name)
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

class Object_Arg extends Arg_Type {
    var $obj_name = null;
    var $cast     = null;
    //var $getter   = null;
    //var $gtk_object_descendant;

    function Object_Arg($obj_name, $typecode)
    {
        $this->obj_name = $obj_name;
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

    function write_param($type, $name, $default, $null_ok, &$var_list,
                         &$parse_list, &$arg_list, &$extra_pre_code, &$extra_post_code, $in_constructor)
    {
        if ($null_ok) {
            $getter = sprintf($this->getter, "php_$name");
            if (isset($default)) {
                $var_list->add($this->obj_name, '*' . $name . ' = ' . $default);
                $var_list->add('zval', '*php_' . $name . ' = NULL');
                $extra_pre_code[]   =   "   if (php_$name) {\n" .
                                    "       if (Z_TYPE_P(php_$name) == IS_NULL)\n" .
                                    "           $name = NULL;\n" .
                                    "       else\n" .
                                    "           $name = $getter;\n" .
                                    "   }\n";
            } else {
                $var_list->add($this->obj_name, '*' . $name . ' = NULL');
                $var_list->add('zval', '*php_' . $name);
                $extra_pre_code[]   =   "   if (Z_TYPE_P(php_$name) != IS_NULL)\n" .
                                    "       $name = $getter;\n";
            }

            $parse_list[]   = '&php_' . $name . ', ' . strtolower($this->typename) . '_ce';
            $arg_list[]     = $name;

            return 'N';
        } else {
            if (isset($default)) {
                $getter = sprintf($this->getter, "php_$name");
                $var_list->add($this->obj_name, '*' . $name . ' = ' . $default);
                $var_list->add('zval', '*php_' . $name . ' = NULL');
                $parse_list[]   = '&php_' . $name . ', ' . strtolower($this->typename) . '_ce';
                $arg_list[]     = $name;
                $extra_pre_code[]   =   "   if (php_$name)\n" .
                                    "       $name = $getter;\n";
            } else {
                $var_list->add('zval', '*' . $name);
                $parse_list[]   = '&' . $name . ', ' . strtolower($this->typename) . '_ce';
                $arg_list[]     = sprintf($this->getter, $name);
            }

            return 'O';
        }
    }
    
    function write_return($type, &$var_list, $separate)
    {
        if ($this->gtk_object_descendant) {
            $initializer = 'php_gtk_new((GtkObject *)%s)';
        } else {
            $initializer = 'php_' . strtolower($this->typename) . '_new(%s)';
        }

        if ($separate) {
            return  "   PHP_GTK_SEPARATE_RETURN(return_value, $initializer);\n" .
                    "%s return;";
        } else {
            return  "   *return_value = *$initializer;\n" .
                    "%s return;";
        }
    }
}

class Boxed_Arg extends Arg_Type {
    var $type        = null;
    var $php_type    = null;

    function Boxed_Arg($type, $php_type)
    {
        $this->type         = $type;
        $this->php_type     = $php_type;
    }

    function write_param($type, $name, $default, $null_ok, &$var_list,
                         &$parse_list, &$arg_list, &$extra_pre_code, &$extra_post_code, $in_constructor)
    {
        if ($null_ok) {
            if (isset($default)) {
                $var_list->add($this->type, '*' . $name . ' = ' . $default);
                $var_list->add('zval', '*php_' . $name . ' = NULL');
                $extra_pre_code[]   =   "   if (php_$name) {\n" .
                                    "       if (Z_TYPE_P(php_$name) == IS_NULL)\n" .
                                    "           $name = NULL;\n" .
                                    "       else\n" .
                                    "           $name = PHP_" . strtoupper($this->php_type). "_GET(php_" . $name . ");\n" .
                                    "   }\n";
            } else {
                $var_list->add($this->type, '*' . $name . ' = NULL');
                $var_list->add('zval', '*php_' . $name);
                $extra_pre_code[]   =   "   if (Z_TYPE_P(php_$name) != IS_NULL)\n" .
                                    "       $name = PHP_" . strtoupper($this->php_type). "_GET(php_" . $name . ");\n";
            }

            $parse_list[]   = '&php_' . $name;
            $parse_list[]   = $this->php_type . '_ce';
            $arg_list[]     = $name;

            return 'N';
        } else {
            if (isset($default)) {
                $var_list->add($this->type, '*' . $name . ' = ' . $default);
                $var_list->add('zval', '*php_' . $name . ' = NULL');

                $parse_list[]   = '&php_' . $name;
                $parse_list[]   = $this->php_type . '_ce';
                $arg_list[]     = $name;
                $extra_pre_code[]   =   "   if (php_$name)\n" .
                                    "       $name = PHP_" .  strtoupper($this->php_type) . "_GET(" . $name . ");\n";
            } else {
                $var_list->add('zval', '*' . $name);
                $parse_list[]   = '&' . $name;
                $parse_list[]   = $this->php_type . '_ce';
                $arg_list[]     = 'PHP_' . strtoupper($this->php_type) . '_GET(' . $name . ')';
            }

            return 'O';
        }
    }

    function write_return($type, &$var_list, $separate)
    {
        if ($separate) {
            return  "   PHP_GTK_SEPARATE_RETURN(return_value, php_" . $this->php_type . "_new(%s));\n" .
                    "%s return;";
        } else {
            return  "   *return_value = *php_" . $this->php_type . "_new(%s);\n" .
                    "%s return;";
        }
    }
}

class Atom_Arg extends Int_Arg {
    function write_return($type, &$var_list, $separate)
    {
        return "    *return_value = *php_gdk_atom_new(%s);\n" .
               "    return;";
    }
}

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
        }
    }

    function register_boxed($type, $php_type)
    {
        $handler = new Boxed_Arg($type, $php_type);
        $this->register($type . '*', $handler);
        $this->register('const-' . $type . '*', $handler);
    }

    function get($type)
    {
        /* TODO check for GdEvent */
        if (isset($this->arg_types[$type])) {
            return $this->arg_types[$type];
        } else {
            throw new Exception("unknown type '$type'");
        }
    }
}

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

#$arg = new Atom_Arg();
#$matcher->register('GdkAtom', $arg);
#
#$arg = new Drawable_Arg();
#$matcher->register('GdkDrawable*', $arg);
#
#$matcher->register_boxed('GdkEvent', 'gdk_event');
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

/* vim: set et sts=4: */
?>
