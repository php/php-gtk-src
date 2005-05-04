<?php
if( !class_exists('gtk')) {
    die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}


class ComboBox extends GtkWindow
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
        

        $combo = GtkComboBox::new_text();
        
        $data = array(
            'PHP-Gtk2',
            'is',
            'really',
            'cool'
        );
        foreach ($data as $string) {
            $combo->append_text($string);
        }
        $combo->set_active(2);
        
        
        $button = new GtkButton('Check this');
        
        $vbox->pack_start($combo, false, true);
        $vbox->pack_start($button, false, true);
        
        $button->connect('clicked', array($this, 'onClickedButton'), $combo);
        
        
        
        //GtkComboBoxEntry
        $vbox->pack_start(new GtkHSeparator(), false, true);
        
        
        $comboentry = GtkComboBoxEntry::new_text();
        $data2 = array('You', 'can', 'edit', 'this', 'box');
        foreach ($data2 as $string) {
            $comboentry->append_text($string);
        }
        $comboentry->get_child()->set_text('That\'s an own text for the entry');
        $vbox->pack_start($comboentry, false, true);
        
        $button2 = new GtkButton('Check that');
        $vbox->pack_start($button2, false, true);
        $button2->connect('clicked', array($this, 'onClickedButton'), $comboentry);
        
        return $vbox;
    }
    
    
    
    function onClickedButton($button, $combo)
    {
        $number = $combo->get_active();
        $text   = $combo->get_active_text();
        
        if ($combo instanceof GtkComboBoxEntry) {
            $owntext = "\r\n\r\nAs this is an GtkComboBoxEntry, we have an extra text:\r\n" . $combo->get_child()->get_text();
        } else {
            $owntext = '';
        }
        $dialog = new GtkMessageDialog($this, Gtk::DIALOG_MODAL | Gtk::DIALOG_DESTROY_WITH_PARENT,
                        Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK,
                        sprintf(
                            'The %s displays "%s" which is item #%d.%s',
                            get_class($combo),
                            $text, 
                            $number,
                            $owntext
                        )
                    );
        $dialog->run();
        $dialog->destroy();

    }
    
    
    //FIXME: use real list models
}

$GLOBALS['class']       = 'ComboBox';
$GLOBALS['description'] = 'Demonstrates the use of GtkComboBox and how to get the options';

if (!@$GLOBALS['framework']) {
    new ComboBox();
    Gtk::main();
}

?>