<?php

class IconView extends GtkWindow
{
    function __construct()
    {
        parent::__construct();
        require_once('WidgetEditor.php');

		if (@$GLOBALS['framework']) {
			return;
		}

        $this->set_default_size(700,300);
        $this->set_title('GtkIconView');
        $this->connect_object('destroy', array('gtk', 'main_quit'));
        
        $this->add($this->__create_box());
        
        $this->show_all();
    }


    
    function __create_box() 
    {
        $iv = new GtkIconView();
        
        $model = new GtkListStore(GdkPixbuf::gtype, Gtk::TYPE_STRING);
        $iv->set_model($model);
        $iv->set_columns(1);
        
        $ids = Gtk::stock_list_ids();
        sort($ids);
        foreach ($ids as $id) {
            $pixbuf = $iv->render_icon($id, Gtk::ICON_SIZE_DIALOG);
        
            $model->set(
                $model->append(),//get the iterator for the next new item
                0, //column 0
                $pixbuf,//value for column 0
                1,//column 1
                $id//value for column 1
            );
        }
        
        $iv->set_pixbuf_column(0);
        $iv->set_text_column(1);
        
        //multi select incl. zooming up an rectangle to select icons
        $iv->set_selection_mode(Gtk::SELECTION_MULTIPLE);
        //labels at the right side
        $iv->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        
        //enough place for the text so that it doesn't wrap
        $iv->set_item_width(200);
        //spacing between icon and label
        $iv->set_spacing(0);
        //spacing between single rows
        $iv->set_row_spacing(0);
        //spacing between cols
        
        //margin from the edges of the view widget -> like the CSS margin property
        $iv->set_margin(5);
        
        //in how many columns the view will be split
        //Icon order (for 3 columns):
        // 1    2    3
        // 4    5    6
        //FIXME: is there a way to arrange it horizontally so that it's arranged that way:
        // 1    4    7
        // 2    5    8
        // 3    6    9   ?
        //0 is auto-fit
        $iv->set_columns(0);
        
        
        $editor = new WidgetEditor(
            $iv, 
            array(
                array( 'selection_mode', GtkComboBox::gtype, 'Gtk::SELECTION_'),
                array( 'orientation'   , GtkComboBox::gtype, 'Gtk::ORIENTATION_'),
                array( 'item_width'    , GtkScale::gtype   ,    0, 500),
                array( 'spacing'       , GtkScale::gtype   , -100, 100),
                array( 'row_spacing'   , GtkScale::gtype   , -100, 100),
                array( 'column_spacing', GtkScale::gtype   , -100, 100),
                array( 'margin'        , GtkScale::gtype   , -100, 200),
                array( 'columns'       , GtkScale::gtype   , 0, 20)
            )
        );
        
        
        $scrollwin = new GtkScrolledWindow();
        $scrollwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        $scrollwin->add($iv);
        
        return $scrollwin;
    }
}//class IconView extends GtkWindow


$GLOBALS['class']		= 'IconView';
$GLOBALS['description']	= 'List with icons';

if (!@$GLOBALS['framework']) {
	new IconView();
	Gtk::main();
}
?>