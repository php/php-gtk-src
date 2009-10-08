<?php
if( !class_exists('gtk')) {
    die('Please load the php-gtk2 module in your php.ini' . "\r\n");
} elseif (!class_exists('GtkInfoBar')) {
    return;
}


class InfoBarBox extends GtkWindow
{
    protected $entry1;
    protected $entry2;
    
    protected $counter = 1;
    
    function __construct($parent = null)
    {
        parent::__construct();

        if (isset($GLOBALS['framework'])) {
            return;
        }

        if ($parent)
            $this->set_screen($parent->get_screen());
        else
            $this->connect_simple('destroy', array('gtk', 'main_quit'));

        $this->set_title(__CLASS__);
        $this->set_position(Gtk::WIN_POS_CENTER);
        $this->set_border_width(8);
        
        $this->add($this->__create_box());
        $this->show_all();
    }

    function create_error() {
        $this->messagelabel->set_text('This is an error message to show');
        $this->infobar->set_message_type(Gtk::MESSAGE_ERROR);
        $this->infobar->show_all();
    }

    function __create_box()
    {
        $this->infobar = new GtkInfoBar();
        $this->messagelabel = new GtkLabel('Default shows an Info message');
        $contentarea = $this->infobar->get_content_area();
        $contentarea->add($this->messagelabel);
        $this->infobar->add_button(GTK::STOCK_OK, GTK::RESPONSE_OK);
        $this->infobar->connect_simple('response', array($this->infobar, 'hide'));

        $vbox = new GtkVBox(false);
        $button = new GtkButton('_Create Error Message');
        $button->connect('clicked', array($this, 'create_error'));
        $vbox->pack_start($button, false, false, 0);
        $vbox->pack_start(new GtkHSeparator(), false, false, 0);
        $vbox->pack_start($this->infobar, false, false, 0);
        return $vbox;
    }
}

$GLOBALS['class']       = 'InfoBarBox';
$GLOBALS['description'] = 'Report important messages to the user';

if (!isset($GLOBALS['framework'])) {
    new InfoBarBox();
    Gtk::main();
}

?>