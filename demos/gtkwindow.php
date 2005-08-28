<?php

class WindowTest extends GtkWindow
{
    function __construct()
    {
        parent::__construct();
        require_once('WidgetEditor.php');

        $this->set_default_size(200,200);
        $this->set_title('GtkWindow demo');
        $this->connect_object('destroy', array('gtk', 'main_quit'));
        $this->add($this->__create_box());
        $this->show_all();
    }

    function __create_box() 
    {
        $vbox = new GtkVBox();
        $label = new GtkLabel('Window test');
        $button = new GtkButton('Click the button!');

        $vbox->pack_start($label);
        $vbox->pack_start($button);

        $editor = new WidgetEditor(
            $this, 
            array(
                array('decorated'           , GtkToggleButton::gtype),
//                array('has_frame'         , GtkToggleButton::gtype),//doesn't work after the window is realized
                array('resizable'           , GtkToggleButton::gtype),
                array('skip_pager_hint'     , GtkToggleButton::gtype),
                array('skip_taskbar_hint'   , GtkToggleButton::gtype),
                array('accept_focus'        , GtkToggleButton::gtype),
                array('modal'               , GtkToggleButton::gtype),
                array('keep_above'          , GtkToggleButton::gtype, 'on', 'off', false),
                array('keep_below'          , GtkToggleButton::gtype, 'on', 'off', false),

                array('iconify'             , GtkButton::gtype, 'deiconify'),
                array('fullscreen'          , GtkButton::gtype, 'unfullscreen'),
                array('maximize'            , GtkButton::gtype, 'unmaximize'),
                array('stick'               , GtkButton::gtype, 'unstick'),

                array('present'             , GtkButton::gtype),
            )
        );

        return $vbox;
    }
}

if (!@$GLOBALS['framework']) {
    new WindowTest();
    Gtk::main();
}
?>