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

if (!class_exists('gtk')) {
	//required for signal generation. no problem if its not available; then they are just omitted
	if (!@dl('php_gtk2.' . PHP_SHLIB_SUFFIX)) {
		echo "Signals will NOT be generated as you don't have php-gtk2 installed\r\n";
		$GLOBALS['disable_signal_generation'] = true;
	}
}


set_time_limit(300);

require 'Getopt.php';
require 'arg_types.php';
require 'override.php';
require 'scheme.php';
require 'docmerger2.php';
require 'doc_templates.php';

/**
 * DocGenerator class. It generates docs.
 */
class DocGenerator {
    protected $parser     = null;
    protected $overrides  = null;
    protected $prefix     = null;
    protected $output_dir = null;
    protected $fp         = null;
    protected $docmerger  = null;

    /**
     * Constructor.
     *
     * @access public
     * @param  &object   $parser
     * @param  &resource $overrides  File pointer to the overrides file.
     * @param  &object   $docmerger
     * @param  string    $prefix     The prefix for the docs.
     * @param  string    $output_dir The directory to write to.
     * @return void
     */
    function DocGenerator(&$parser, &$overrides, &$docmerger, $prefix, $output_dir)
    {
        $this->parser       = &$parser;
        $this->overrides    = &$overrides;
        $this->docmerger    = &$docmerger;
        $this->prefix       = $prefix;

        // Check to see if an output_dir was passed.
        if ($output_dir) {
            $this->output_dir = $output_dir;
        } else {
            // If not write to stdout
            $this->fp = fopen('php://stdout', 'w');
        }
    }

    /**
     * Creates the docs. Workhorse for class.
     *
     * @access public
     * @param  array  $classes Associative array of classes to build docs for.
     * @return void
     */
    function create_docs($classes)
    {
        // Make each classname lowercase
        foreach ($classes as $key => $val) {
            $classes[$key] = strtolower($val);
        }

        /**
         * Object sorter. Puts objects in alphabetical order.
         */
        function sort_objects($a, $b)
        {
            return strcmp($a->c_name, $b->c_name);
        }
        
        // Get objects from parser.
        $parser_objects = $this->parser->objects;

        // Sort objects from parser.
        usort($parser_objects, 'sort_objects');

        // Create the docs for each object.
        foreach ($parser_objects as $object) {
            
            // Get object name as lowercase module + name
            $object_lcname = strtolower($object->in_module . $object->name);
            
            // Skip objects not in classes array.
            if (count($classes) && !in_array($object_lcname, $classes))
                continue;

            // If we have an output directory, write the docs there.
            if ($this->output_dir) {
                print "Generating $object_lcname.xml...";
                //$this->fp = fopen($this->output_dir.'/'.$object_lcname.'.xml', 'w');
                $this->write_class($object, $this->output_dir.'/'.$object_lcname.'.xml');
                //fclose($this->fp);
                print "\n";
            } else {
                // Otherwise write the docs to stdout.
                $this->write_class($object);
            }
        }

        // Check to see if we are writing miscellaneous functions.
        // ex: gtk::timeout_add()
        if (count($classes) == 0) {
            // Write to the output directory.
            if ($this->output_dir) {
                print "Generating ".$this->prefix."-functions.xml...";
                $this->fp = fopen($this->output_dir.'/'.$this->prefix.'-functions.xml', 'w');
                $this->write_functions();
                fclose($this->fp);
                print "\n";
            } else {
                // Write to stdout
                $this->write_functions();
            }
        }
    }
    
    /**
     * Registers object types?
     *
     * @access public
     * @param  object $parser
     * @return void
     */
    function register_types($parser = null)
    {
        global $matcher;

        // Use the classes parser if one wasn't given to 
        // the function.
        if (!$parser) {
            $parser = $this->parser;
        }

        // Register each object in the parser.
        foreach ($parser->objects as $object) {
            $matcher->register_object($object->c_name, $object->typecode);
        }
        
        // Register all of the enums.
        foreach ($parser->enums as $enum) {
            if ($enum->def_type == 'flags') {
                $matcher->register_flag($enum->c_name);
            } else {
                $matcher->register_enum($enum->c_name);
            }
        }
    }

    /**
     * Writes the docs for the given class.
     *
     * @access public
     * @param  object $object The class to write docs for.
     * @return void
     */
    function write_class($object, $path = NULL)
    {
        global  $class_start_tpl,
                $class_end_tpl,
                $update_docs;

        // Create the name as module + name
        $object_name = $object->in_module . $object->name;

        // Prep-a-r-e the docs if we are just updating.
        // I was under the impression that this doesn't work.
        if($update_docs) {
            $this->docmerger->prepair($object_name);
        }

		// Open the file after the merge if we can.
		if (!empty($path)) {
			$this->fp = @fopen($path, 'w');
			if (!$this->fp) {
				echo "\r\nERROR: can't open file " . $path . " for writing\r\n";
				exit(2);
			}
		}

		// Check for the classmeta from file.
		$id = $this->prefix . '.' . strtolower($object_name);
		$merged = $this->docmerger->getSectionContentsById($id);
		if (empty($merged)) {
			// Write the opening of the doc file from a template.
			fwrite($this->fp,
				   sprintf($class_start_tpl,
						   $this->prefix,
						   strtolower($object_name),
						   $object_name,
						   $object->parent,
						   NULL, NULL
						   )
				   );
		} else {
			fwrite($this->fp, '<classentry id="' . $this->prefix . '.' . strtolower($object_name) . "\">\n");
			fwrite($this->fp, $merged);
		}

        // Write the constructor.
        $this->write_constructors($object);
        
        // Write the class methods.
        $this->write_methods($object);
        
        // Write the properties.
        $this->write_properties($object);
		
		$this->write_signals($object);

        // If updating, write something else. I have to come back to this.
        if($update_docs) {
            fwrite($this->fp,
                   $this->docmerger->getSectionContentsByName('signals')
				   );
        }

        // Close off the file with another template.
        fwrite($this->fp, $class_end_tpl);

		fclose($this->fp);
		
		$GLOBALS['docgenstats']['classes']++;
    }

    
    
    /**
     * Writes the class properties to the current file.
     *
     * @access public
     * @param  object $object The class to write docs for.
     * @return void.
     */
    function write_properties($object)
    {
        global  $props_start_tpl,
                $props_end_tpl;

        // Check to see if the class has any publicly
        // accessible properties.
        if (count($object->fields) == 0) {
            return;
        }

        // Write the opening for the properties section.
        fwrite($this->fp, $props_start_tpl);

        // Write each property.
        foreach ($object->fields as $field) {
            $this->write_property($object, $field);
        }
        
        // Write the closing of the properties section.
        fwrite($this->fp, $props_end_tpl);
    }//function write_properties($object)

    
    
    /**
     * Writes one property to the current file.
     *
     * @access public
     * @param  object $object The class to write docs for.
     * @param  array  $field  The property to create docs for.
     * @return void
     */
    function write_property($object, $field)
    {
        // The property templates.
        global  $prop_start_tpl,
                $prop_end_tpl;

        // Get the property type and name.
        list($field_type, $field_name) = $field;

        // If we can't determine what the real type is use XXX
        try {
			$doc_type = $this->get_type($field_type);
		} catch (Exception $ex) {
            $doc_type = 'XXX';
        }

        // Write the opening from a template.
        fwrite($this->fp, sprintf($prop_start_tpl, $this->prefix,
                                  strtolower($object->in_module . $object->name),
                                  $field_name, $field_name, $doc_type));
        
        // Write the closing from a template.
        fwrite($this->fp, sprintf($prop_end_tpl, 
                                    $this->gen_etter_links(
                                        $field_name,
                                        $this->parser->find_methods($object),
                                        null,
                                        true)
                                ));
		
		$GLOBALS['docgenstats']['properties']++;
    }//function write_property($object, $field)

    
    
    /**
     * Write the class methods to the current file.
     *
     * @access public
     * @param  object $object The class to write docs for.
     * @return void
     */
    function write_methods($object)
    {
        // The methods section templates.
        global  $methods_start_tpl,
                $methods_end_tpl;
        
        // Get methods from the object.
        $methods = $this->parser->find_methods($object);
        
        // Only write the template if there are methods
        // for this class.
        if (count($methods)) {
            fwrite($this->fp, $methods_start_tpl);
        }

        // Write each method to the current file.
        foreach ($methods as $method) {
            if ($this->overrides->is_overriden($method->c_name)) {
                // The default method was overriden.
                $this->write_method($method, true, $methods, $object->fields);
            } else if (!$this->overrides->is_ignored($method->c_name)) {
                // Don't write a method that is ignored.
                $this->write_method($method, false, $methods, $object->fields);
            }
        }

        // Write the closing template if there were methods
        // to be written.
        if (count($methods)) {
            fwrite($this->fp, $methods_end_tpl);
        }
    }

    /**
     * Writes functions that don't belong to a particular class.
     *
     * @access public
     * @param  none
     * @return void
     */
    function write_functions()
    {
        // Write each function.
        foreach ($this->parser->functions as $function) {
            if ($this->overrides->is_overriden($function->c_name)) {
                // Function was overriden.
                $this->write_method($function, true);
            } else if (!$this->overrides->is_ignored($function->c_name)) {
                // Don't write functions that are ignored.
                $this->write_method($function, false);
            }
        }
    }

    /**
     * Writes the class constructor.
     *
     * @access public
     * @param  object $object The class to write docs for.
     * @return void
     */
    function write_constructors($object)
    {
        // The constructor templates.
        global  $constructors_start_tpl,
				$constructors_end_tpl,
				$constructor_start_tpl,
				$constructor_alt_start_tpl,
                $constructor_end_tpl,
                $funcproto_tpl,
                $no_parameter_tpl,
                $update_docs;

        // Get the constructor method from the class.
        $constructed = false;
        $constructors = $this->parser->find_constructor($object, $this->overrides);
		if (count($constructors) > 0) {

			foreach ($constructors as $constr_number => $constructor) {
        
				// If a constructor was found, check to see if it was
				// overriden or ignore.
				if ($constructor) {
					if ($this->overrides->is_overriden($constructor->c_name)) {
						$overriden = true;
					} else if (!$this->overrides->is_ignored($constructor->c_name)) {
						$overriden = false;
					} else {
						break;
					}
				} else {
					// No constructor found.
					break;
				}
		
				// If it hasn't been overriden, get the parameters.
				if (!$overriden) {
					if (($paramdef = $this->get_paramdef($constructor)) === false) {
						// No parameters found!
						break;
					}
				}
		
				// Get the class that the constructor instantiates.
				$object_name = $constructor->is_constructor_of;
				$is_main_constructor = $constr_number == 0;
                
                if ($is_main_constructor) {
                    $constructed = true;
                    fwrite($this->fp, $constructors_start_tpl);
                }
				
				if (!$is_main_constructor) {
					//get the real function name
					$function_name = substr($constructor->c_name, strpos($constructor->c_name, "_new_") + 1);
				}
		
				$id = $this->prefix . '.' . strtolower($object_name) . '.constructor';
				$merged = $this->docmerger->getSectionContentsById($id);
				if (empty($merged)) {
					// Write the constructor opening template.
					if ($is_main_constructor) {
						//first constructor
						fwrite($this->fp,
							sprintf($constructor_start_tpl,
									$this->prefix,
									strtolower($object_name),
									$constr_number));
					} else {
						//not the first constructor -> static factory method
						fwrite($this->fp,
							sprintf($constructor_alt_start_tpl,
									$this->prefix,
									strtolower($object_name),
									$function_name));
					}
					
					// Create the function prototype docs from a template.
					$function_fullname = $is_main_constructor ? $object_name : $object_name . '::' . $function_name;
					$funcdef = sprintf($funcproto_tpl,
									'',
									$function_fullname,
									$overriden ? sprintf($no_parameter_tpl, 'XXX') : $paramdef);
					
					// Write the function prototype docs to the current file.
					fwrite($this->fp, preg_replace('!^ !m', '', $funcdef));
					
					// Write the constructor closing from a template.
					fwrite($this->fp, 
						sprintf($constructor_end_tpl,
								NULL,
								NULL
								)
						);    
					$GLOBALS['docgenstats']['constructors']++;
				} else {
					fwrite($this->fp, $merged);
				}
			}//foreach constructor
			
            if ($constructed) {
                fwrite($this->fp, $constructors_end_tpl);
            }
		}//there is at least oneconstructor
	}//function write_constructors($object)
	
	

    /**
     * Writes a single method to the current file.
     *
     * @access public
     * @param  object $method    The method to write docs for.
     * @param  bool   $overriden Whether or not the method was overriden.
     * @param  array  $methods   Array with all methods for this object. Used to cross-link getter and setter
     * @param  array  $properties Array with all properties for this object. Used to cross-link them with the method
     * @return void
     */
    function write_method($method, $overriden, $methods = null, $properties = null)
    {
        // The method templates.
        global  $method_start_tpl,
                $method_func_start_tpl,
                $method_end_tpl,
                $funcproto_tpl,
                $no_parameter_tpl,
                $update_docs;

        // Get the parameters if the method was not overriden.
        if (!$overriden) {
			//TODO: check if alright
			try {
				$paramdef = $this->get_paramdef($method);
                if ($paramdef == '') {
                    $paramdef = sprintf($no_parameter_tpl, 'void');
                }
				$return = $this->get_type($method->return_type);
			} catch (Exception $ex) {
                return;
            }
        }

        // Write the method to the current file with the object name.
        if (isset($method->of_object)) {
            $object_name = $method->of_object;

			$id = $this->prefix . '.' . strtolower($object_name) . '.method.' . $method->name;
			$merged = $this->docmerger->getSectionContentsById($id);
			if (empty($merged)) {
				fwrite($this->fp,
					   sprintf($method_start_tpl,
							   $this->prefix,
							   strtolower($object_name),
							   $method->name));
			} else {
				fwrite($this->fp, $merged);
			}
        } else {
            // The object name is not available.
            fwrite($this->fp,
                   sprintf($method_func_start_tpl,
                           $this->prefix,
                           $method->name));
        }
		if (empty($merged)) {
			// Write the method docs to the current file.
			fwrite($this->fp,
				   sprintf($funcproto_tpl,
						   $overriden ? 'XXX' : $return,
						   $method->name,
						   $overriden ? sprintf($no_parameter_tpl, 'XXX') : $paramdef));
			
			// Close off the method docs.
			fwrite($this->fp, 
				   sprintf($method_end_tpl,
						   NULL,
						   $this->gen_etter_links($method->name, $methods, $properties)
						   )
				   );
		}
		$GLOBALS['docgenstats']['functions']++;
	}//function write_method($method, $overriden, $methods = null, $properties = null)
	
	
	
	/**
	*	write the signals for the given class object.
	*	works only if php-gtk2 module is installed and working
	*	
	*	@param $object	The class object which is passes to write_class
	*/
	function write_signals($object)
	{
		global	$signals_start_tpl,
				$signal_start_tpl,
				$cbfuncproto_tpl,
				$no_parameter_tpl,
				$signal_end_tpl,
				$signals_end_tpl;
				
		if (isset($GLOBALS['disable_signal_generation']) && $GLOBALS['disable_signal_generation']) {
			//is the second warning necessary?
			echo "signals are NOT generated as you don't have php-gtk2 installed\r\n";
			return;
		}
		
		$classname = $object->c_name;
		//why can't I do "$classname::gtype"?
		eval("\$gtype = $classname::gtype;");
		$signals = GObject::signal_list_names($gtype);

		if (count($signals) == 0) {
			return;
		}
		
		fwrite($this->fp, $signals_start_tpl);
		
		foreach ($signals as $signalname) {
			$GLOBALS['docgenstats']['signals']++;
			fwrite($this->fp,
				sprintf($signal_start_tpl,
					$this->prefix,
					strtolower($classname),
					$signalname,
					$signalname));
			
			$signal_info = GObject::signal_query($signalname, $gtype);
			
			$return_type = $this->get_type($signal_info[4]->name);
			if (!$return_type && $signal_info[4]->name == 'void') {
				$return_type = $signal_info[4]->name;
			}
			
			fwrite($this->fp,
				sprintf($cbfuncproto_tpl,
					$return_type ? $return_type : '<!-- was: ' . $signal_info[4]->name . ' -->XXX',
					$this->get_signal_paramdef($signal_info[5])
					));

			fwrite($this->fp, $signal_end_tpl);
		}
		
		fwrite($this->fp, $signals_end_tpl);
	}//function write_signals($object)
    
    

    /**
     * Gets the parameters for the given function.
     *
     * @access public
     * @param  object $function The function to get params for.
     * @return void
     */
    function get_paramdef($function)
    {
        global  $matcher,
                $parameter_tpl,
                $no_parameter_tpl,
                $opt_parameter_tpl;

        // Check to see if the function has variable arguments.
        if ($function->varargs) {
            return false;
        }
		
		$paramdef = '';

        if (count($function->params) > 0) {
            // Get the info about each parameter.
            foreach ($function->params as $params_array) {
                list($param_type, $param_name, $param_default, $param_null) = $params_array;

                // Get the php type from the c type.
                if (($doc_type = $this->get_type($param_type)) === false) {
                    return false;
                }

                // Check for a default value.
                if (isset($param_default)) {
                    $paramdef .= sprintf($opt_parameter_tpl, $doc_type, $param_name, $param_default);
                } else {
                    $paramdef .= sprintf($parameter_tpl, $doc_type, $param_name);
                }
            }
        }

        // Return either the parameter string or a no param string
        return $paramdef != null ? $paramdef : sprintf($no_parameter_tpl, 'void');
    }//function get_paramdef($function)
	
	
	
	/**
	*	Returns the parameters for the signal callback
	*	The normal get_paramdef can't be used as it requires 
	*	 parameters which signal_query doesn't support
	*
	*	@param array	$signalparams element 5 of the GObject::signal_query return
	*	@return string	The signal parameters as string
	*/
	function get_signal_paramdef($signalparams)
	{
		global	$parameter_tpl,
				$no_parameter_tpl;
		
		$params = '';
		
		if (count($signalparams) > 0) {
			foreach ($signalparams as $param) {
				$type = $this->get_type($param->name);
				
				$params .= sprintf($parameter_tpl,
							$type ? $type : '<!-- was: ' . $param->name . ' -->XXX',
							'UNKNOWN');
			}
		} else {
			$params .= sprintf($no_parameter_tpl, 'void');
		}
		
		return $params;
	}//function get_signal_paramdef($signalparams)
    
    
    
    /**
    * generates links to corresponding getter- or setter functions
    * for inclusion in the <desc> section of the function method
    *
    * does the same for add_*, remove_* and so
    *
    * @return string The content to be written in the desc section
    */
    function gen_etter_links($methodname, $methods, $properties, $is_property = false)
    {
        global  $etter_start_tpl,
                $etter_end_tpl,
                $etter_link,
                $prop_link;
                
        if (!$is_property) {
            $base = substr($methodname, strpos($methodname, '_') + 1);
        } else {
            $base = $methodname;
        }
        
        if ($methods === null || count($methods) == 0 || $base == substr($methodname, 1)) {
            return '';
        }
        
        //other ?etter functions
        $foundone = false;
        $all = '';
        $funcs = array();
        foreach ($methods as $method) {
            if (substr($method->name, strpos($method->name, '_') + 1) == $base && $method->name != $methodname) {
                $funcs[] = sprintf($etter_link,
                                $method->of_object,
                                $method->name
                                );
            }
        }//foreach method
        
        //now: check if a property with the same name exists
        if ($properties != null && !$is_property && count($properties) > 0) {
            foreach ($properties as $property) {
                if ($property[1] == $base) {
                    $funcs[] = sprintf($prop_link,
                                        $methods[0]->of_object,
                                        $property[1]
                                        );
                }
            }
        }
        
        if (count($funcs) > 0) {
            $all .= $etter_start_tpl;
            $all .= implode($funcs, ', ');
            $all .= $etter_end_tpl;
        }
        
        return $all;
    }//function gen_etter_links($methodname, $methods, $properties) 

  

    /**
     * Gets a PHP type from a C type.
     *
     * @access public
     * @param  string $in_type The C type.
     * @return string The PHP type or false if no match
     */
    function get_type($in_type)
    {
        global $matcher;
        
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
                                 'GtkType'        => 'int',
                                 
                                 'gboolean'       => 'bool',
                                 
                                 'double'         => 'double',
                                 'gdouble'        => 'double',
                                 'float'          => 'double',
                                 'gfloat'         => 'double',
                                 
                                 'GdkDrawable*'   => 'GdkWindow');
        
        // Check for a type handler.
		try {
	        $type_handler = &$matcher->get($in_type);
        } catch (Exception $ex) {
            return false;
        }

        // If we have an exact match, return it.
        if (isset($type_map[$in_type])) {
            return $type_map[$in_type];
        } else {
            // Otherwise try to figure it out.
            $in_type = str_replace('*', '', $in_type);

            // See if the type is an object.
            $type_handler_class = get_class($type_handler);
            if ($type_handler_class == 'object_arg' ||
                $type_handler_class == 'boxed_arg') {
                return "<classname>$in_type</classname>";
            } else if ($type_handler_class == 'enum_arg' ||
                       $type_handler_class == 'flags_arg') {
                // Type is an enum or a flag
                return "<enumname>$in_type</enumname>";
            } else {
                // Can't figure it out. Just give it back.
                return $in_type;
            }
        }
    }
}

/* For backwards compatibility. */
chdir(dirname(__FILE__));

// Use $_SERVER if available.
if (isset($_SERVER)) {
    $argc = $_SERVER['argc'];
    $argv = $_SERVER['argv'];
} else {
    $argc = $HTTP_SERVER_VARS['argc'];
    $argv = $HTTP_SERVER_VARS['argv'];
}

$GLOBALS['docgenstats'] = array(
	'classes'		=> 0,
	'constructors'	=> 0,
	'functions'		=> 0,
	'properties'	=> 0,
	'signals'		=> 0
);

/* An ugly hack to counteract PHP's pernicious desire to treat + as an argument
   separator in command-line version. */
// I don't need this. It causes trouble.
// array_walk($argv, create_function('&$x', '$x = urldecode($x);'));

// Use PEAR::Console_Getopt to get the arguments from the 
// command line.
$result = Console_Getopt::getopt($argv, 'o:p:r:d:s:l:u');
if (!$result || count($result[1]) < 2) {
    // Set up the help message.
    die(
        "Usage: php -q generator.php [OPTION] defsfile [class ...]\n\n" . 
        "  -o <file>         use overrides in <file>\n" . 
        "  -p <prefix>       use <prefix> for docs\n" . 
        "  -r <file>         register types from <file>\n" . 
        "  -d <path>         output files to this directory\n" . 
        "  -s <path>         documentation dir\n" . 
        "  -l <lang>         Language\n" . 
        "  -u                Update existing docs\n"
        );
}
list($opts, $argv) = $result;

// Set the default options.
$prefix = 'gtk';
$overrides = new Overrides();
$lang = 'en';
$update_docs = FALSE;

// Override the defaults with the command line options.
foreach ($opts as $opt) {
    list($opt_spec, $opt_arg) = $opt;
    if ($opt_spec == 'o') {
        $overrides = new Overrides($opt_arg);
    } else if ($opt_spec == 'p') {
        $prefix = $opt_arg;
    } else if ($opt_spec == 'r') {
        $type_parser = new Defs_Parser($opt_arg);
        $type_parser->start_parsing();
        DocGenerator::register_types($type_parser);
    } else if ($opt_spec == 'd') {
        $output_dir = $opt_arg;
    } else if ($opt_spec == 's') {
        $docs_dir = $opt_arg;
    } else if ($opt_spec ==    'l') {
        $lang = $opt_arg;
    } else if ($opt_spec ==    'u') {
        $update_docs = TRUE;
    }
}

// Create a new defs parser.
$parser = new Defs_Parser($argv[1]);

// Create a DocMerger if updating.
//if($update_docs) {
    $docmerger = new DocMerger($docs_dir, $lang, $prefix);
//}

// Create the generator.
$generator = new DocGenerator($parser, $overrides, $docmerger, $prefix, $output_dir);

// Get to work.
$parser->start_parsing();
$generator->register_types();
$generator->create_docs(array_slice($argv, 2));

//stats
echo 'Some statistics:' . "\r\n";
foreach ($GLOBALS['docgenstats'] as $name => $value) {
	echo ' ' . str_pad($name, 12) . ' ' . str_pad($value, 4, ' ', STR_PAD_LEFT) . "\r\n";
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
?>
