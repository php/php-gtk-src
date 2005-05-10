<?php
if( !class_exists('gtk')) {
    die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}


class EntryCompletion extends GtkWindow
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
        $this->set_default_size(-1, -1);
        $this->set_border_width(8);
        
        $this->add($this->__create_box());
        $this->show_all();
    }

    
    
    function __create_box()
    {
        $vbox = new GtkVBox(false, 5);
        $vbox->set_border_width(5);
        
        $label = new GtkLabel();
        $label->set_markup('Completion demo, try writing <b>total</b> or <b>gnome</b> for example.');
        
        $vbox->pack_start($label, false, false, 0);
        
        $entry = new GtkEntry();
        $vbox->pack_start($entry, false, false, 0);
        
        $completion = new GtkEntryCompletion();
        $completion_model = $this->__create_completion_model();
        $completion->set_model($completion_model);
        $completion->set_text_column(0);
        
        $entry->set_completion($completion);
        
        
        return $vbox;
    }
    
    
    
    function __create_completion_model()
    {
        $store = new GtkListStore(Gtk::TYPE_STRING);
        
        $iter = $store->append();
        $store->set($iter, 0, 'GNOME');
        
        $iter = $store->append();
        $store->set($iter, 0, 'total');
        
        $iter = $store->append();
        $store->set($iter, 0, 'totally');
        
        $iter = $store->append();
        $store->set($iter, 0, 'PHP');
        
        $iter = $store->append();
        $store->set($iter, 0, 'PHP-Gtk2');
        
        return $store;
    }
}

$GLOBALS['class']       = 'EntryCompletion';
$GLOBALS['description'] = 'GtkEntryCompletion provides a mechanism for adding support for
completion in GtkEntry.';

if (!@$GLOBALS['framework']) {
    new EntryCompletion();
    Gtk::main();
}

?>