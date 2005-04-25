<?php
if( !class_exists('gtk')) {
	die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}


class Expander extends GtkWindow
{
	function __construct($parent = null)
	{
		parent::__construct();

		if (@$GLOBALS['framework']) {
			return;
		}

		if ($parent)
			$this->set_screen($parent->get_screen());
		else
			$this->connect_object('destroy', array('gtk', 'main_quit'));

		$this->set_title(__CLASS__);
		$this->set_position(Gtk::WIN_POS_CENTER);
		$this->set_default_size(-1, 500);
		$this->set_border_width(8);
		
		$this->add($this->__create_box());
		$this->show_all();
	}

	
	
	function __create_box()
	{
		$vbox = new GtkVBox(false, 5);
		$vbox->set_border_width(5);
		
		$label = new GtkLabel();
		$label->set_markup('Expander demo. Click on the triangle for details.');
		$vbox->pack_start($label, false, false, 0);
		
		$expander = new GtkExpander('Details');
		$vbox->pack_start($expander, false, false, 0);
		
		$label2 = new GtkLabel('Details can be shown or hidden');
		$expander->add($label2);
		
		return $vbox;
	}
}

$GLOBALS['class']		= 'Expander';
$GLOBALS['description']	= 'GtkExpander allows to provide additional content that is initially hidden.
This is also known as "disclosure triangle".';

if (!@$GLOBALS['framework']) {
	new Expander();
	Gtk::main();
}

?>