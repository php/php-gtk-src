<?php

/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001 James Moore <jmoore@php.net>
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

define(METHOD, "method");
define(CLASSENTRY, "classentry");
define(SHORTDESC, "shortdesc");
define(DESC, "desc");
define(SIGNAL, "signal");
define(SIGNALS, "signals");
define(ATTRIBUTE, "attribute");
define(DOCMERGER_OUTPUT_ON, 1 << 0);

set_time_limit(300);

class DocMerger 
{
    var $docs_dir             = null;
    var $lang                 = null;
    var $prefix               = null;
    var $parser               = null;
    var $flags                = null;
    var $stack                = null;
    var $arr_pointer          = null;
    var $current_element_name = null;
	var $current_class        = null;
    var $class_contents       = array();

    function DocMerger($docs_dir, $lang, $prefix)
    {
        $this->docs_dir = $docs_dir;
        $this->lang = $lang;
        $this->prefix = $prefix;
        $this->stack = new XMLStack();

    }

    function get_section_contents($elementname, $type, $section)
    {
        switch ($type)
        {
            case CLASSENTRY:
                switch ($section)
                {
                    case SHORTDESC:
                        return $this->class_contents[SHORTDESC];
                    case DESC:
                        return $this->class_contents[DESC];
                }
                break;
            case CONSTRUCTOR:
                switch ($section)
                {
                    case SHORTDESC:
                    case DESC:
                        return $this->class_contents[$type][$section];
                        break;
                    default:
                        die("Unknown sections contents requested for $elementname");
                }
            case METHOD:
			case SIGNAL:
                switch ($section)
                {
                    case SHORTDESC:
                    case DESC:
                        return $this->class_contents[$type][$elementname][$section];
                        break;
                    default:
                        die("Unknown sections contents requested for $elementname");
                }
                break;
			case SIGNALS:
				if($this->class_contents[SIGNAL])
				{
					$retval = "\n <signals>\n";
					while(list($signalname, $signal) = each($this->class_contents[SIGNAL]))
					{
							$retval .= "\n  <signal id=\"".DocMerger::gen_signal_id($this->prefix, $this->current_class ,$signalname)."\">";
							$retval .= "\n   <signalname>".$signalname."</signalname>";
							$retval .= "\n   <shortdesc>".$this->get_section_contents($signalname, SIGNAL, SHORTDESC)."</shortdesc>";
							$retval .= "\n   <desc>";
							$retval .= $this->get_section_contents($signalname, SIGNAL, DESC);
							$retval .= "\n   </desc>";
							$retval .= "\n  </signal>\n";

				
					}
					$retval .= " </signals>\n";
					return $retval;
				}
				break;
            case ATTRIBUTE:
                break;
            default:
                die("Unknown section contents requested.");
        }
    }

    function prepair($classname)
    {
        if(!$this->parser) {
            $this->parser = xml_parser_create();
            xml_parser_set_option($this->parser, XML_OPTION_CASE_FOLDING, 1);
            xml_parser_set_option($this->parser, XML_OPTION_TARGET_ENCODING, "UTF-8");
            xml_set_object($this->parser,&$this);
            xml_set_element_handler($this->parser,"xml_start_element","xml_end_element");
            xml_set_character_data_handler($this->parser,"xml_char_data");
        }

		$this->current_class = $classname;

        $filename = DocMerger::find_class_file($this->docs_dir, $this->lang, $this->prefix, $classname);
        
        if($filename)
        {
            $this->parse($filename);
        }
        return TRUE;
    }

    function parse($file)
    {
        $fp = fopen($file, "r");
        if(!$fp)
            die("Could not open input file");
        
        while ($data = fread($fp, 4096)) {
            if (!xml_parse($this->parser, $data, feof($fp))) {
              DocMerger::XMLErrorHandler($this->parser);
            }
        }

    }

    function find_class_file($docs_dir, $lang, $prefix, $classname)
    {
        /* -*- Check Current Langs Dir -*- */
        if(file_exists($docs_dir."/".$lang."/reference/".$prefix."/".$classname.".xml"))
                return $docs_dir."/".$lang."/reference/".$prefix."/".$classname.".xml";
        
        /* -*- Check EN dir -*- */
        if($lang != 'en')
            if(file_exists($docs_dir."/en/reference/".$prefix."/".$classname.".xml"))
                    return $docs_dir."/en/reference/".$prefix."/".$classname.".xml";
        
        /* -*- Fail -*- */
        return NULL;
    }

    function xml_start_element($parser, $name, $attrs)
    {
        switch (strtolower($name))
        {
            case "shortdesc":
            case "desc":
                $this->output = null;
                switch($this->stack->gettos())
                {
                    case "classmeta":
                    case "attribute":
                    case "method":
                    case "signal":
                    case "constructor":
                        $this->flags = $this->flags | DOCMERGER_OUTPUT_ON;
                }    
                /* -- Fall through -- */
            case "classmeta":
            case "methods":
            case "method":
            case "signals":
            case "signal":
            case "attributes":
            case "attribute":
            case "constructor":
                $this->stack->push(strtolower($name));
                break;
            case "function":
            case "signalname":
                if(($this->stack->gettos() == "method") || ($this->stack->gettos()  == "signal") || ($this->stack->gettos() == "constructor"))
                {
                    $this->stack->push(strtolower($name));
                    $this->output = null;
                    $this->flags = $this->flags | DOCMERGER_OUTPUT_ON;
                    break;
                }
                /* -- Else Fall through -- */
            default:
                if($this->flags & DOCMERGER_OUTPUT_ON)
                    $this->output .= '<'.strtolower($name).DocMerger::AttrsToString($attrs).'>';
        }

    }

    function xml_end_element($parser, $name)
    {
        switch (strtolower($name))
        {
            case "shortdesc":
            case "desc":
                $this->flags = $this->flags & (~DOCMERGER_OUTPUT_OFF);
                $this->stack->pop();
                switch($this->stack->gettos())
                {
                    case "classmeta":
                        $this->class_contents[((strtolower($name)=="desc")?DESC:SHORTDESC)] = $this->output;
                        break;
                    case "method":
                        $this->class_contents[METHOD][$this->current_element_name][((strtolower($name)=="desc")?DESC:SHORTDESC)] = $this->output;
                        break;
                    case "constructor":
                        $this->class_contents[CONSTRUCTOR][((strtolower($name)=="desc")?DESC:SHORTDESC)] = $this->output;
                        break;
                    case "attribute":
                        $this->class_contents[ATTRIBUTE][$this->current_element_name][((strtolower($name)=="desc")?DESC:SHORTDESC)] = $this->output;
                        break;
                    case "signal":
                        $this->class_contents[SIGNAL][$this->current_element_name][((strtolower($name)=="desc")?DESC:SHORTDESC)] = $this->output;
                        break;
                }
                break;
            case "classmeta":
            case "methods":
            case "signals":
            case "attributes":
            case "method":
            case "attribute":
            case "signal":
                $this->stack->pop();
                break;
            case "function":
            case "signalname":
                if($this->stack->gettos() == strtolower($name))
                {
                    $this->flags = $this->flags & (~DOCMERGER_OUTPUT_OFF);
                    $this->current_element_name = $this->output;
                    $this->stack->pop();
                    break;
                }
                /* -- Else Fall through -- */
            default:
                if($this->flags & DOCMERGER_OUTPUT_ON)
                    $this->output .= '</'.strtolower($name).'>';
        }
    }

    function xml_char_data($parser, $data)
    {   
        if($this->flags & DOCMERGER_OUTPUT_ON)
            $this->output .= $data;
    }

    function xml_default_handler($parser, $data)
    {
        echo "default hanlder called";    
    }

    function XMLErrorHandler(&$xml_parser)
    {
        die(sprintf("XML error: %s at line %d\n",
                    xml_error_string(xml_get_error_code($xml_parser)),
                    xml_get_current_line_number($xml_parser)));
    }

    function AttrsToString($attrs)
    {

        if ($attrs) {
            while (list($key, $value) = each($attrs)) {
                $retval .= " ".strtolower($key)."='".$value."'";
            }
        }
        return $retval;
    }

	function gen_signal_id($prefix, $classname ,$signalname)
	{
		return $prefix.".".$classname.".signal.".$signalname;
	}

}

class XMLStack 
{
    var $tos   = null;
    var $stack = array();

    function push($data)
    {
        $this->stack[++$this->tos] = $data;
    }

    function pop()
    {
        return $this->stack[$this->tos--];
    }

    function gettos()
    {
        return $this->stack[$this->tos];
    }
}

?>