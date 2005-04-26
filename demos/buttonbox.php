<?php
if( !class_exists('gtk')) {
	die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}


class ButtonBox extends GtkWindow
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
		$box = new GtkVBox();
		
		$frame_horiz = new GtkFrame("Horizontal Button Boxes");
		$box->pack_start($frame_horiz, true, true, 10);
		
		$vbox = new GtkVBox();
		$vbox->set_border_width(10);
		
		$frame_horiz->add($vbox);
		
		$vbox->pack_start($this->create_framedbox(true, "Spread", 40, Gtk::BUTTONBOX_SPREAD));
		$vbox->pack_start($this->create_framedbox(true, "Edge"	, 40, Gtk::BUTTONBOX_EDGE));
		$vbox->pack_start($this->create_framedbox(true, "Start"	, 40, Gtk::BUTTONBOX_START));
		$vbox->pack_start($this->create_framedbox(true, "End"	, 40, Gtk::BUTTONBOX_END));
		
		$frame_vert = new GtkFrame("Vertical Button Boxes");
		$box->pack_start($frame_vert, true, true, 10);
		
		$hbox = new GtkHBox();
		$hbox->set_border_width(10);
		
		$frame_vert->add($hbox);
		
		$hbox->pack_start($this->create_framedbox(false, "Spread"	, 40, Gtk::BUTTONBOX_SPREAD));
		$hbox->pack_start($this->create_framedbox(false, "Edge"		, 40, Gtk::BUTTONBOX_EDGE));
		$hbox->pack_start($this->create_framedbox(false, "Start"	, 40, Gtk::BUTTONBOX_START));
		$hbox->pack_start($this->create_framedbox(false, "End"		, 40, Gtk::BUTTONBOX_END));
		
		return $box;
	}
	
	
	
	function create_framedbox($horizontal, $title, $spacing, $layout)
	{
		$frame = new GtkFrame($title);
		if ($horizontal) {
			$box = new GtkHButtonBox();
		} else {
			$box = new GtkVButtonBox();
		}
		
		$box->set_border_width(5);
		$box->set_layout($layout);
		$box->set_spacing($spacing);
		
		$frame->add($box);
		
		$button = GtkButton::new_from_stock(Gtk::STOCK_OK);
		$box->add($button);

		$button = GtkButton::new_from_stock(Gtk::STOCK_CANCEL);
		$box->add($button);
		
		$button = GtkButton::new_from_stock(Gtk::STOCK_HELP);
		$box->add($button);

		return $frame;
	}
}

$GLOBALS['class']		= 'ButtonBox';
$GLOBALS['description']	= 'This demo shows various button box configurations available.  It also
uses stock buttons, and use of mnemonics for navigation.';

if (!@$GLOBALS['framework']) {
	new ButtonBox();
	Gtk::main();
}

?>