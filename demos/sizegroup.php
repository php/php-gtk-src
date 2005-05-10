<?php
if( !class_exists('gtk')) {
    die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}


class SizeGroup extends GtkWindow
{
    protected $size_group = null;
    
    
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
        $this->set_default_size(-1, -1);
        $this->set_border_width(8);
        
        $this->add($this->__create_box());
        $this->show_all();
    }

    
    
    function __create_box()
    {
        $vbox = new GtkVBox(false, 5);
        $vbox->set_border_width(5);
        
        $this->size_group = new GtkSizeGroup(gtk::SIZE_GROUP_HORIZONTAL);
        
        $frame = new GtkFrame('Color options');
        $vbox->pack_start($frame, true, true, 0);
        
        $table = new GtkTable(2, 2, false);
        $table->set_border_width(5);
        $table->set_row_spacings(5);
        $table->set_col_spacings(10);
        $frame->add($table);
        
        $color_options = array('Red', 'Green', 'Blue');
        
        $this->add_row($table, 0, '_Foreground', $color_options);
        $this->add_row($table, 1, '_Background', $color_options);
        
        $frame = new GtkFrame('Line options');
        $vbox->pack_start($frame, true, true, 0);
        
        $table = new GtkTable(2, 2, false);
        $table->set_border_width(5);
        $table->set_row_spacings(5);
        $table->set_col_spacings(10);
        $frame->add($table);
        
        $dash_options  = array('Solid', 'Dashed', 'Dotted');
        $end_options   = array('Square', 'Round', 'Arrow');
        
        $this->add_row($table, 0, '_Dashing', $dash_options);
        $this->add_row($table, 1, '_Line ends', $end_options);
        
        
        $check_button = new GtkCheckButton('_Enable grouping');
        $check_button->set_use_underline(true);
        $vbox->pack_start($check_button, false, false, 0);
        
        $check_button->set_active(true);
        $check_button->connect('toggled', array($this, 'on_toggle_grouping'));
        
        return $vbox;
    }
    
    
    
    function add_row($table, $row, $label_text, $options)
    {
        $label = new GtkLabel($label_text);
        $label->set_use_underline(true);
        $label->set_alignment(0, 1);
        $table->attach($label, 0, 1, $row, $row + 1, Gtk::EXPAND + Gtk::FILL, 0, 0, 0);
        
        $option_menu = $this->create_option_menu($options);
        $label->set_mnemonic_widget($option_menu);
        $this->size_group->add_widget($option_menu);
        $table->attach($option_menu, 1, 2, $row, $row + 1, 0, 0, 0, 0);
    }
    
    
    
    function create_option_menu($options)
    {
//		$option_menu = new GtkComboBox();
//		$model = new GtkListStore(Gtk::TYPE_STRING);
//		$option_menu->set_model($model);
        
        $option_menu = GtkComboBox::new_text();
        
        foreach ($options as $option) {
//			$iter = $model->append();
//			$model->set($iter, 0, $option);

            $option_menu->append_text($option);
        }
        
        $option_menu->set_active(0);
        return $option_menu;
    }
    
    
    
    function on_toggle_grouping($check_button) {
        if ($check_button->get_active()) {
            $this->size_group->set_mode(gtk::SIZE_GROUP_HORIZONTAL);
        } else {
            $this->size_group->set_mode(gtk::SIZE_GROUP_NONE);
        }
    }
}

$GLOBALS['class']       = 'SizeGroup';
$GLOBALS['description'] = "GtkSizeGroup provides a mechanism for grouping a number of widgets together so
they all request the same amount of space. This is typically useful when you
want a column of widgets to have the same size, but you can't use a GtkTable
widget.

Note that size groups only affect the amount of space requested, not the size
that the widgets finally receive. If you want the widgets in a GtkSizeGroup to
actually be the same size, you need to pack them in such a way that they get
the size they request and not more. For example, if you are packing your
widgets into a table, you would not include the GTK_FILL flag.";

if (!@$GLOBALS['framework']) {
    new SizeGroup();
    Gtk::main();
}

?>