<?php
/**
*
*/

if (!class_exists('gtk')) {
	die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}



class PHPGtk2Demo extends GtkWindow
{
	protected $demos = array();
	protected $description_buffer;
	protected $demobox;
	protected $lastbox = null;
	
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
		
		$this->demobox = new GtkHBox();
		$page = $notebook->append_page($this->demobox, new GtkLabel('Demo'));

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
		$this->description_buffer->set_text($text, strlen($text));
		
		$classname = $info->classname;
		$obj = new $classname();
	
		//remove kid
		if ($this->lastbox !== null) {
			$this->demobox->remove($this->lastbox);
		}
		
		if (!isset($this->cache[$info->classname])) {
			$subobj = $obj->__create_box();
			$this->cache[$info->classname] = null;
			$this->cache[$info->classname] = $subobj;
		}
		$this->lastbox = $this->cache[$info->classname];
		$this->demobox->pack_start($this->lastbox);
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
		$text_view->set_wrap_mode(GTK_WRAP_WORD);
		
		return $scrolled_window;
	}
	
	
	
	function load_demos()
	{
		$files = glob('*.php');
		foreach ($files as $id => $file) {
			if ($file != basename(__FILE__) && $file != 'stock-browser.php') {
				if (!@include_once($file)) {
					continue;
				}
				$this->demos[$file]					= null;
				$this->demos[$file]->classname		= $GLOBALS['class'];
				$this->demos[$file]->description	= $GLOBALS['description'];
				$this->demos[$file]->file			= $file;
			}
		}
	}//function load_demos()
}//class PHPGtk2Demo extends GtkWindow


$GLOBALS['framework'] = true;
new PHPGtk2Demo();
Gtk::main();

?>