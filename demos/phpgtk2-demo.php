<?php
/*
* php5 discourages the use of dl(), so the user should add it to php.ini
*/
if (!class_exists('gtk')) {
    die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}



class PHPGtk2Demo extends GtkWindow
{
    protected $demos = array();
    protected $description_buffer;
    protected $demobox;
    
    static protected $colors = array("blue", "red", "orange", "darkgreen", "black");
    
    protected $cache = array();
    
    function __construct()
    {
        parent::__construct();

        $this->connect_object('destroy', array('gtk', 'main_quit'));
        
        $this->set_title('PHP-Gtk2 Demos');
        $this->set_default_size(800, 400);
        
        $hbox = new GtkHBox(false, 3);
        $this->add($hbox);
        
        $treeview = $this->__create_treeview();
        $hbox->pack_start($treeview, false, false);
        
        $notebook = new GtkNotebook();
        $hbox->pack_start($notebook, true);
        
        $notebook->append_page($this->__create_text(), new GtkLabel('Info'));
        
        $scroll_demo = new GtkScrolledWindow();
        $scroll_demo->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        
//        $this->demobox = new GtkHBox();
        $this->demobox = $scroll_demo;
        $page = $notebook->append_page($this->demobox, new GtkLabel('Demo'));

        list($text, $this->sourcebuffer) = $this->__create_source();
        $notebook->append_page($text, new GtkLabel('Source'));
        
        $this->show_all();
    }//function __construct()
    
    
    
    function __create_treeview()
    {
        $this->load_demos();
        
        $model = new GtkListStore(Gtk::TYPE_PHP_VALUE, Gtk::TYPE_STRING);
//		$model->append(array("asd", "aswdasd"));//???????
        
        foreach ($this->demos as $demo) {
            $iter = $model->append();
            $model->set($iter, 0, $demo);
        }
        
        $treeview = new GtkTreeView($model);
        
        $cell_renderer = new GtkCellRendererText();
        $treeview->insert_column_with_data_func(-1, 'Demos', $cell_renderer, array(&$this,'label_setter'));
        
        $selection = $treeview->get_selection();
        $selection->set_mode(Gtk::SELECTION_SINGLE);
        $selection->connect('changed', array($this, 'demo_selected'));
        
        //crashes as gboxed can't be instantiated
//		$treeview->connect('row-activated', array($this, 'demo_activate'));

//		$column = new GtkTreeViewColumn();
//		$column->set_title('Demos');
//		$treeview->append_column($column);

        return $treeview;
    }//function __create_treeview()
    
    
    
    function __create_source()
    {
        $scrolled_window = new GtkScrolledWindow();
        $scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        $scrolled_window->set_shadow_type(Gtk::SHADOW_IN);
        
        $text_view = new GtkTextView();
        $scrolled_window->add($text_view);

        $buffer = new GtkTextBuffer();
        $text_view->set_buffer($buffer);
        $text_view->set_editable(false);
        $text_view->set_cursor_visible(false);

		$tag_table = $buffer->get_tag_table();	
	
		foreach(self::$colors as $color) {
			$tag = new GtkTextTag($color);
			$tag->set_property("foreground", $color);
			$tag_table->add($tag);
		}
        
//        $text_view->set_wrap_mode(false);
        
        return array($scrolled_window, $buffer);
    }
    
    
    function label_setter($column, $cell, $model, $iter)
    {
        $info = $model->get_value($iter, 0);
        $cell->set_property('text', $info->classname);
    }
    
    
    
    function demo_selected($selection)
    {
        list($model, $iter) = $selection->get_selected();
        if (!$iter) {
            return;
        }
        $info = $model->get_value($iter, 0);
    
        $text = $info->classname . "\r\n\r\n" . $info->description;

	//  [JSJ]: I get a too many arguements error using this form, so changed to the later form
	//  Seems silly we would require the strlen() anyway as PHP should be storing that in the zval already anyway
    //	$this->description_buffer->set_text($text, strlen($text));
        $this->description_buffer->set_text($text);
        
        //source code highlighting
        $this->highlightSource($info->file);
        
        $classname = $info->classname;
        $obj = new $classname();
    
        //remove kid
        if ($this->demobox->get_child() !== null) {
            $this->demobox->remove($this->demobox->get_child());
        }
        
        if (!isset($this->cache[$info->classname])) {
            $subobj = $obj->__create_box();
            $this->cache[$info->classname] = null;
            $this->cache[$info->classname] = $subobj;
        }
        $this->demobox->add_with_viewport($this->cache[$info->classname]);
        $this->demobox->show_all();
        
    }
    
    
    
    function __create_text()
    {
        $scrolled_window = new GtkScrolledWindow();
        $scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        $scrolled_window->set_shadow_type(Gtk::SHADOW_IN);
        
        $text_view = new GtkTextView();
        $scrolled_window->add($text_view);
        
        $this->description_buffer = new GtkTextBuffer();
        $text_view->set_buffer($this->description_buffer);
        $text_view->set_editable(false);
        $text_view->set_cursor_visible(false);
        $text_view->set_wrap_mode(Gtk::WRAP_WORD);
        
        return $scrolled_window;
    }
    
    
    
    protected function load_demos()
    {
        $files = glob(dirname(__FILE__).'/*.php');
        foreach ($files as $id => $file) {
            $GLOBALS['class'] = null;
            if (basename($file) != basename(__FILE__) 
            && basename($file) != 'stock-browser.php'
            && basename($file) != 'WidgetEditor.php') {
                if (!@include_once($file)) {
                    continue;
                }
                if ($GLOBALS['class'] === null) {
                    continue;
                }
                $this->demos[$file]					= null;
                $this->demos[$file]->classname		= $GLOBALS['class'];
                $this->demos[$file]->description	= $GLOBALS['description'];
                $this->demos[$file]->file			= $file;
            }
        }
    }//protected function load_demos()



    protected function highlightSource($filename)
    {
        $tokens = token_get_all(file_get_contents($filename));

		$highlighted = '';
		$color_codes = array();
		
		// this would be much easier if we had functions which supported varargs (like insert_with_tags_by_name), but improvised solution works in the meantime.
		
        foreach ($tokens as $token) {
	    $start = strlen($highlighted);
            if (is_string($token)) {
                //single string
				if($token == "(" || $token == ")" || $token == "{" || $token == "}" || $token == ";" || $token == "@" || $token == "[" || $token == "]" || $token == "!") {
		    		$color = "darkgreen";
				}
				else {
		    		$color = "black";
				}
                $highlighted .= $token;
            } else {
                list($type, $value) = $token;
				switch($type) {
		    		case T_COMMENT:
		    		case T_DOC_COMMENT: {
						$color = "orange";
						break;
		    		}
			    	case T_CLASS:
			    	case T_DOUBLE_ARROW:
			    	case T_NEW:
			    	case T_PAAMAYIM_NEKUDOTAYIM:
		    		case T_IF:
			    	case T_OBJECT_OPERATOR:
			    	case T_RETURN:
			    	case T_EXTENDS:
		    		case T_CLASS_C:
		    		case T_FUNC_C:
		    		case T_WHILE:
		   			case T_EXIT:
		   			case T_INC:
		   			case T_IS_EQUAL:
		   			case T_IS_EQUAL:
		   			case T_IS_IDENTICAL:
		   			case T_IS_NOT_EQUAL:
		   			case T_IS_NOT_IDENTICAL:
		   			case T_IS_SMALLER_OR_EQUAL:
		   			case T_MINUS_EQUAL:
		   			case T_MOD_EQUAL:
		   			case T_MUL_EQUAL:
		   			case T_OR_EQUAL:
		   			case T_PLUS_EQUAL:
		   			case T_FOREACH:
		   			case T_AS:
		    		case T_ELSE:
		    		case T_ELSEIF:
		    		case T_ARRAY:
		    		case T_BOOLEAN_AND:
		    		case T_BOOLEAN_OR:
		    		case T_CONCAT_EQUAL:
		    		case T_DIV_EQUAL:
		    		case T_CONST:
		    		case T_STATIC:
		    		case T_AND_EQUAL:
		    		case T_FUNCTION: {
						$color = "darkgreen";
						break;
		    		}
		    		case T_LNUMBER:
		    		case T_OPEN_TAG:
		    		case T_CLOSE_TAG:
		    		case T_NUM_STRING:
		   			case T_DNUMBER:
		    		case T_VARIABLE:
		    		case T_PRIVATE:
		    		case T_PUBLIC:
		    		case T_PROTECTED:
		    		case T_VAR:
		    		case T_INSTANCEOF:
		    		case T_STRING: {
						$color = "blue";
						break;
		    		}
		    		case T_CONSTANT_ENCAPSED_STRING: {
						$color = "red";
						break;
		    		}
		    		default: {
						$color = "black";
						break;
		    		}
				}
                $highlighted .= $value;
            }
	    	$end = strlen($highlighted);
	    	$color_codes[$color][] = $start . ":" . $end;	    
        }
		$this->sourcebuffer->set_text($highlighted);
        foreach($color_codes as $color_code=>$positions) {
	    	foreach($positions as $position) {
                list($start_pos,$end_pos) = explode(":", $position);
                $start = $this->sourcebuffer->get_iter_at_offset($start_pos);
		 		$end = $this->sourcebuffer->get_iter_at_offset($end_pos);
		 		$this->sourcebuffer->apply_tag_by_name($color_code, $start, $end);
            }
        }
    }//protected function highlightSource($filename)
}//class PHPGtk2Demo extends GtkWindow


$GLOBALS['framework'] = true;
new PHPGtk2Demo();
Gtk::main();

?>
