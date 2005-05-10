<?php
if( !class_exists('gtk')) {
    die('Please load the php-gtk2 module in your php.ini' . "\r\n");
}


class DialogMessagebox extends GtkWindow
{
    protected $entry1;
    protected $entry2;
    
    protected $counter = 1;
    
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
        $mvbox = new GtkVBox(false);
        
        $frame = new GtkFrame('Dialogs');
        
        $vbox = new GtkVBox(false, 8);
        $vbox->set_border_width(8);
        $frame->add($vbox);
        
        # Standard message dialog
        $hbox = new GtkHBox(false, 8);
        $vbox->pack_start($hbox);
        $button = new GtkButton('_Message Dialog');
        $button->connect('clicked', array($this, 'on_message_dialog_clicked'));
        $hbox->pack_start($button, false, false, 0);
        $vbox->pack_start(new GtkHSeparator(), false, false, 0);
        
        # Interactive dialog
        $hbox = new GtkHBox(false, 8);
        $vbox->pack_start($hbox, false, false, 0);
        $vbox2 = new GtkVBox();
        
        $button = new GtkButton('_Interactive Dialog');
        $button->connect('clicked', array($this, 'on_interactive_dialog_clicked'));
        $hbox->pack_start($vbox2, false, false, 0);
        $vbox2->pack_start($button, false, false, 0);
        
        $table = new GtkTable(2, 2);
        $table->set_row_spacings(4);
        $table->set_col_spacings(4);
        $hbox->pack_start($table, false, false, 0);
        
        $label = new GtkLabel('Entry _1');
        $label->set_use_underline(true);
        $table->attach($label, 0, 1, 0, 1);
        
        $this->entry1 = new GtkEntry();
        $table->attach($this->entry1, 1, 2, 0, 1);
        $label->set_mnemonic_widget($this->entry1);
        
        $label = new GtkLabel('Entry _2');
        $label->set_use_underline(true);
        $table->attach($label, 0, 1, 1, 2);
        
        $this->entry2 = new GtkEntry();
        $table->attach($this->entry2, 1, 2, 1, 2);
        $label->set_mnemonic_widget($this->entry2);
        
        return $frame;
    }
    
    
    function on_message_dialog_clicked($button)
    {
        $dialog = new GtkMessageDialog($this, Gtk::DIALOG_MODAL | Gtk::DIALOG_DESTROY_WITH_PARENT,
                        Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK,
                        sprintf('This message box has been popped up %d time%s.', $this->counter, $this->counter == 1 ? '' : 's'));
        $dialog->run();
        $dialog->destroy();
        $this->counter++;
    }
    
    
    function on_interactive_dialog_clicked($button)
    {
        $dialog = new GtkDialog('Interactive Dialog', $this, 0, array( Gtk::STOCK_OK, Gtk::RESPONSE_OK, '_Non-stock button', Gtk::RESPONSE_CANCEL));
    
        $hbox = new GtkHBox(false, 8);
        $hbox->set_border_width(8);
        $dialog->vbox->pack_start($hbox, false, false, 0);
    
        $stock = GtkImage::new_from_stock(
                Gtk::STOCK_DIALOG_QUESTION,
                Gtk::ICON_SIZE_DIALOG);
        $hbox->pack_start($stock, false, false, 0);
    
        $table = new GtkTable(2, 2);
        $table->set_row_spacings(4);
        $table->set_col_spacings(4);
        $hbox->pack_start($table, true, true, 0);
    
        $label = new GtkLabel('Entry _1');
        $label->set_use_underline(true);
        $table->attach($label, 0, 1, 0, 1);
        $local_entry1 = new GtkEntry();
        $local_entry1->set_text($this->entry1->get_text());
        $table->attach($local_entry1, 1, 2, 0, 1);
        $label->set_mnemonic_widget($local_entry1);
    
        $label = new GtkLabel('Entry _2');
        $label->set_use_underline(true);
        $table->attach($label, 0, 1, 1, 2);
        $local_entry2 = new GtkEntry();
        $local_entry2->set_text($this->entry2->get_text());
        $table->attach($local_entry2, 1, 2, 1, 2);
        $label->set_mnemonic_widget($local_entry2);
    
        $dialog->show_all();
    
        $response = $dialog->run();
    
        if ($response == Gtk::RESPONSE_OK) {
            $this->entry1->set_text($local_entry1->get_text());
            $this->entry2->set_text($local_entry2->get_text());
        }
        $dialog->destroy();
    }
}

$GLOBALS['class']       = 'DialogMessagebox';
$GLOBALS['description'] = 'Dialog widgets are used to pop up a transient window for user feedback.';

if (!@$GLOBALS['framework']) {
    new DialogMessagebox();
    Gtk::main();
}

?>