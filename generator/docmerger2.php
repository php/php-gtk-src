<?php
/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001,2002 James Moore   <jmoore@php.net>
 * Copyright (C) 2005      Scott Mattoks <scottmattocks@php.net>
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
/**
 * Because I don't really understand what is going on
 * in docmerger.php, I have to write my own docmerger.
 * The docmerger really just parses the XML file and
 * returns portions when asked. My strategy will be to
 * build element objects which consist of the properties
 * of each XML element, the character data for the 
 * element and the children of the element.
 *
 * The docmerger itself will contain one element and 
 * will be able to search all elements for the given
 * element and section.
 */
class XMLElement {
    
    /**
     * The XML element name.
     * @var string
     */
    var $name;
    /**
     * The XML element attributes.
     * @var array
     */
    var $attributes;
    /**
     * The child elements.
     * @var array
     */
    var $children = array();
    /**
     * The elements parent.
     * @var object
     */
    var $parent;
    /**
     * Any character data that may be present.
     * @var string
     */
    var $data = '';

    /**
     * Constructor. Sets the element name and attributes.
     *
     * @access public
     * @param  string $name
     * @param  array  $attributes
     * @retrun void
     */
    function XMLELement($name, $attributes)
    {
        $this->name       =  $name;
        $this->attributes =  $attributes;
    }
    
    /**
     * Adds a child to the array of children elements and
     * sets the child's parent to this object.
     *
     * @access public
     * @param  &object $child An XMLElement object.
     * @return void
     */
    function addChild(&$child)
    {
        // Add the child.
        $this->children[] =& $child;
        
        // Set this instance as the parent.
        $child->setParent($this);
    }
    
    /**
     * Sets this instance's parent to the given XMLElement.
     *
     * @access public
     * @param  &object $parent An XMLElement
     * @return void
     */
    function setParent(&$parent)
    {
        $this->parent =& $parent;
    }

    /**
     * Returns the parent element for this instance.
     *
     * @access public
     * @param  none
     * @return &object An XMLElement instance.
     */
    function &getParent()
    {
        return $this->parent;
    }
    
    /**
     * Returns the XML name for this element.
     *
     * @access public
     * @param  none
     * @return string
     */
    function getName()
    {
        return $this->name;
    }

    /**
     * Sets the character data for this element.
     * 
     * @access public
     * @param  string $data
     * @return void
     */
    function setData($data)
    {
        $this->data.= $data;
    }
    
    /**
     * Finds an element by name.
     *
     * @access public
     * @param  string $name
     * @return void
     */
    function &search($name)
    {
		$name = strtoupper($name);

        // If this is right element, return it.
        if (strtoupper($this->name) == $name) {
            return $this;
        } elseif (count($this->children)) {
            // Look through all of the children.
            foreach ($this->children as $child) {
                $result = $child->search($name);
                if (!empty($result)) {
                    return $result;
                }
            }
        }
        
        // Nothing was found.
        return NULL;      
    }

    /**
     * Finds an element by the ID attribute.
     *
     * @access public
     * @param  string  $id Of the form: prefix.classname.[metod|signal|...].<sectionName>
     * @return &object     The element with the matching id attribute.
     */
    function &searchById($id)
    {
        // If this is the right element return it.
        if (isset($this->attributes['ID']) && $id == $this->attributes['ID']) {
            return $this;
        } else {
            // Check all of the children.
            foreach ($this->children as $child) {
                $childId = $child->searchById($id);
                if (!empty($childId)) {
                    return $childId;
                }
            }
        }

        // Id could not be found.
        return NULL;
    }

    /**
     * Gets the attributes, data, and children for this element
     * and all children of this element.
     *
     * @access public
     * @param  integer $depth An indent tracker.
     * @return string         The formatted XML.
     */
    function getData($depth = 1)
    {
        // Create an XML tag for this element and then
        // for each child element. If the element name is 
        // a number, then this is cdata. Just add it to the
        // string.
        $end    = '';
        $output = '';

        // If this element simply represents character data,
        // output it.
        if ($this->name == 'CDATA') {
            $output = trim($this->data);

            // Add a new line for the parent closing tag.
            if (!in_array(strtolower($this->parent->getName()), DocMerger::noNewLine())) {
                $end.="\n" . str_pad(' ', $depth - 1, ' ', STR_PAD_LEFT);
            }
        } else {
            // This is more than CDATA. Some extra formatting
            // will need to be done.

            // If the parent is character data, add a space before
            // starting this element.
            if ($this->parent->getName() == 'CDATA') {
                $output = ' ';
            }

            // Add the element name and attributes.
            $output.= '<' . strtolower($this->name);
            $output.= DocMerger::AttrsToString($this->attributes) . '>';

            // Some elements should always be followed by a new line.
            // We should pad the start of the newline to align the
            // next element properly.
            if (!in_array(strtolower($this->name), DocMerger::noNewLine())) {
                $output.="\n" . str_pad(' ', count($this->children) ? $depth + 1 : $depth, ' ', STR_PAD_LEFT);
            } 

			// Check to see if we need <![CDATA[ ]]>
			if (strtolower($this->name) == 'programlisting' ||
				strtolower($this->name) == 'command') {
				$output.= "<![CDATA[\n";
				$end = ']]>' . $end . "\n" . str_pad(' ', $depth, ' ', STR_PAD_LEFT);
			}

            // Set up the closing element tag.
            $end.= '</' . strtolower($this->name) . '>';
            
            // Some elements closing tags should always be followed by 
            // a new line. We should pad the new line again.
            if (!in_array(strtolower($this->name), DocMerger::noClosingNewLine())) {
                $end.= "\n" . str_pad(' ', $depth - 1, ' ', STR_PAD_LEFT);
            }
        }
        
        // Go through the children and get their output.
        foreach ($this->children as $count => $child) {
            if ($child->getName() != 'CDATA' && $count > 0) {
                $output.= ' ';
            }
            $output.= $child->getData($depth + 1);
        }

        // Return the formatted XML string.
        return $output . $end;
    }
}

class DocMerger {
    
    /**
     * The directory to find the docs in.
     * @var string
     */
    var $docs_dir;
    /**
     * The language the docs are in.
     * @var string
     */
    var $lang;
    /**
     * The prefix for the docs that are being generated (gtk|gdk|...)
     * @var string
     */
    var $prefix;
    /**
     * The XML parser object.
     * @var object
     */
    var $parser;
    /**
     * The first XMLElement.
     * @var object
     */
    var $startElement;
    /**
     * The current XMLElement.
     * @var object
     */
    var $currentElement;

    /**
     * Constructor. Sets the doc directory, the language and
     * the prefix.
     *
     * @access public
     * @param  string $docs_dir
     * @param  string $lang
     * @param  string $prefix
     * @return void
     */
    function DocMerger($docs_dir, $lang, $prefix)
    {
        $this->docs_dir = $docs_dir;
        $this->lang = $lang;
        $this->prefix = $prefix;
    }

    /**
     * Gets the contents of the section with the given id tag.
     *
     * @access public
     * @param  string $id <prefix>.<classname>[.<method|signal|...>].<sectionName>
     * @return string
     */    
    function getSectionContentsById($id)
    {
        if (empty($this->startElement)) {
            return '';
        }

        $section = $this->startElement->searchById($id);
		
		// If the returned result is the start element,
		// only get data for the classmeta element.
		if ($section->getName() == $this->startElement->getName()) {
			$section = $this->startElement->search('classmeta');
		}
        
        return empty($section) ? '' : $section->getData();
    }

    function getSectionContentsByName($name)
	{
        if (empty($this->startElement)) {
            return '';
        }

		$section = $this->startElement->search($name);
		return empty($section) ? '' : $section->getData();
	}

    /**
     * Sets up the parser then parses the file for the given class.
     *
     * @access public
     * @param  string $class
     * @return void
     */
    function prepare($class)
    {
        $this->parser = xml_parser_create();
        xml_parser_set_option($this->parser, XML_OPTION_CASE_FOLDING, 1);
        xml_parser_set_option($this->parser, XML_OPTION_TARGET_ENCODING, "UTF-8");
        xml_set_object($this->parser,$this);
        xml_set_element_handler($this->parser,"xml_start_element","xml_end_element");
        xml_set_character_data_handler($this->parser,"xml_char_data");
        
        $filename = DocMerger::find_class_file($this->docs_dir, $this->lang, $this->prefix, $class);
        
        if($filename) {
            $this->parse($filename);
        }
        return TRUE;
    }
    
    /**
     * Calls the correctly spelled function.
     *
     * @see prepare
     */
    function prepair($class)
    {
        $this->prepare($class);
    }
    
    /**
     * Parses the given file.
     *
     * This method should only be called from the prepare method.
     *
     * @access private
     * @param  string  $file The path to the file to be parsed.
     * @reutrn void
     */
    function parse($file)
    {
        // Try to open the file.
        $fp = fopen($file, "r");
        if(!$fp) {
            die("Could not open input file");
        }

        // Parse each line of the file.
        while ($data = fread($fp, 4096)) {
            if (!xml_parse($this->parser, $data, feof($fp))) {
                // Handle any errors found.
                DocMerger::XMLErrorHandler($this->parser);
            }
        }
    }
    
    /**
     * Finds the file for a given classname.
     *
     * @access private
     * @param  string $docs_dir
     * @param  string $lang
     * @param  string $prefix
     * @param  string $classname
     * @return string The path to the file or NULL if not found.
     */
    function find_class_file($docs_dir, $lang, $prefix, $classname)
    {
        /* -*- Check Current Langs Dir -*- */
        if(file_exists($docs_dir."/".$lang."/reference/".$prefix."/".$classname.".xml"))
                return $docs_dir."/".$lang."/reference/".$prefix."/".$classname.".xml";
        
        /* -*- Check EN dir -*- */
        if($lang != 'en') {
            if(file_exists($docs_dir."/en/reference/".$prefix."/".$classname.".xml")) {
                return $docs_dir."/en/reference/".$prefix."/".$classname.".xml";
            }
        }

        // Scott's check for file in -s directory.
        if (file_exists($docs_dir . '/' . strtolower($classname) . '.xml')) {
            return $docs_dir . '/' . strtolower($classname) . '.xml';
        }
        /* -*- Fail -*- */
        return NULL;        
    }
    
    /**
     * Handles opening element tags.
     * 
     * @access private
     * @param  object  $parser
     * @param  string  $name
     * @param  array   $attrs
     * @return void
     */
    function xml_start_element($parser, $name, $attrs)
    {
        if (!isset($this->startElement)) {
            // This is the first element. Create it and set it
            // as the current element.
            $this->startElement = new XMLElement($name, $attrs);
            $this->currentElement =& $this->startElement;
        } else {
            // This is not the first element. Create a new one
            // set the current element as the parent and then 
            // set the new element as the current element.
            $element = new XMLElement($name, $attrs);
            $this->currentElement->addChild($element);
            $this->currentElement =& $element;
        }
    }
    
    /**
     * Handles closing element tags.
     *
     * @access private
     * @param  object  $parser
     * @param  string  $name
     * @return void
     */
    function xml_end_element($parser, $name)
    {
        // Set the current element's parent as the current element.
        $this->currentElement =& $this->currentElement->getParent();
    }
    
    /**
     * Handles character data.
     *
     * The XML parser does not care if the text is just text between
     * a set of element tags or if it was between <![CDATA[ tags.
     * This is causing problems. 
     *
     * @access private
     * @param  object  $parser
     * @param  string  $data
     * @return void
     */
    function xml_char_data($parser, $data)
    {
        // We don't care about empty strings.
        if (trim($data)) {
            // Create a CDATA XMLElement object.
            $this->xml_start_element($parser, 'CDATA', array());
            // Strip out new lines.
			if (strtolower($this->currentElement->parent->getName()) != 'programlisting') {
				$this->currentElement->setData(str_replace("\n", '', $data));
			} else {
				$this->currentElement->setData($data);
			}
            // Close it off.
            $this->xml_end_element($parser, 'CDATA');
        }
    }

    /**
     * Handles anything left over.
     *
     * @access private
     * @param  object  $parser
     * @param  string  $data
     * @return void
     */
    function xml_default_handler($parser, $data)
    {
        echo "default hanlder called";    
    }

    /**
     * Outputs a message about what went wrong.
     *
     * @access private
     * @param  &object $xml_parser
     * @return 
     */
    function XMLErrorHandler(&$xml_parser)
    {
        die(sprintf("XML error: %s at line %d\n",
                    xml_error_string(xml_get_error_code($xml_parser)),
                    xml_get_current_line_number($xml_parser)));
    }

    /**
     * Turns an attribute array into an attribute string.
     *
     * @access public
     * @param  array  $attrs
     * @return string
     */
    function AttrsToString($attrs)
    {
        if ($attrs) {
            while (list($key, $value) = each($attrs)) {
                $retval .= ' '.strtolower($key).'="'.$value.'"';
            }
        }
        return $retval;
    }

    /**
     * Returns an array of elements that should not be followed by 
     * a new line.
     *
     * @static
     * @access public
     * @param  none
     * @reutrn array
     */
    function noNewLine()
    {
        $noNewLine   = array();
        $noNewLine[] = 'classtitle';
        $noNewLine[] = 'classparent';
        $noNewLine[] = 'optional';
        $noNewLine[] = 'function';
        $noNewLine[] = 'funcdef';
        $noNewLine[] = 'paramdef';
        $noNewLine[] = 'parameter';
        $noNewLine[] = 'classname';
        $noNewLine[] = 'enumname';
        $noNewLine[] = 'propname';
        $noNewLine[] = 'proptype';
        $noNewLine[] = 'literal';
        $noNewLine[] = 'title';
        return $noNewLine;
    }

    /**
     * Returns an array of closing element tags that should 
     * not be followed by a new line.
     *
     * @static
     * @access public
     * @param  none
     * @reutrn array
     */
    function noClosingNewLine()
    {
        $noNewLine   = array();
        $noNewLine[] = 'optional';
        $noNewLine[] = 'function';
        $noNewLine[] = 'parameter';
        $noNewLine[] = 'classname';
        $noNewLine[] = 'enumname';
        $noNewLine[] = 'literal';

        return $noNewLine;
    }
}
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
?>