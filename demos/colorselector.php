<?php
if( !class_exists('gtk')) {
	die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}


class ColorSelector extends GtkWindow
{
	protected $d_area;
	protected $color;
	
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
		$this->set_border_width(8);
		
		$this->add($this->__create_box());
		$this->show_all();
	}

	
	
	function __create_box()
	{
		$vbox = new GtkVBox(false, 5);
		$vbox->set_border_width(8);
		
		//Create the color swatch area
		$frame = new GtkFrame();
		$frame->set_shadow_type(Gtk::SHADOW_IN);
		$vbox->pack_start($frame, true, true, 8);
		
		$this->d_area = new GtkDrawingArea();
		$this->d_area->set_size_request(200, 200);
		
//		$this->d_area->modify_bg(Gtk::STATE_NORMAL, $this->color);
		
		$frame->add($this->d_area);
		
		$alignment = new GtkAlignment(1.0, 0.5, 0.0, 0.0);
		
		$button = new GtkButton('_Change the above color');
		$alignment->add($button);
		
		$vbox->pack_start($alignment, true, true);
		
		$button->connect('clicked', array($this, 'on_change_color_clicked'));
		$button->set_flags(Gtk::CAN_DEFAULT);
		
		return $vbox;
	}
	
	
	
	function on_change_color_clicked($button)
	{
		$dialog = new GtkColorSelectionDialog('Changing color');
		$dialog->set_transient_for($this);
		
		$colorsel = $dialog->colorsel;
		
		if ($this->color !== null) {
			$colorsel->set_previous_color($this->color);
			$colorsel->set_current_color($this->color);
		}
		$colorsel->set_has_palette(true);
		
		$response = $dialog->run();
		
		if ($response == Gtk::RESPONSE_OK) {
			$this->color = $colorsel->get_current_color();
			$this->d_area->modify_bg(Gtk::STATE_NORMAL, $this->color);
		}
		
		$dialog->destroy();
		return true;
	}
}

$GLOBALS['class']		= 'ColorSelector';
$GLOBALS['description']	= 'GtkColorSelection lets the user choose a color. GtkColorSelectionDialog is a
prebuilt dialog containing a GtkColorSelection.';

if (!@$GLOBALS['framework']) {
	new ColorSelector();
	Gtk::main();
}

?>