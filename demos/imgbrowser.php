<?php


class ImgBrowser extends GtkWindow
{
    var $img = null;
    var $bLoadPreview = true;


    function __construct($parent = null)
    {
        parent::__construct();

        if (@$GLOBALS['framework']) {
            return;
        }

        if ($parent) {
            $this->set_screen($parent->get_screen());
        } else {
            $this->connect_simple('destroy', array('Gtk', 'main_quit'));
        }

        $this->set_title(__CLASS__);
        $this->set_position(Gtk::WIN_POS_CENTER);
        $this->set_default_size(500, 400);

        $this->add($this->__create_box());
        $this->show_all();
    }



    function __create_box()
    {
        $box = new GtkVBox();
        $box->pack_start(new GtkLabel('Double click with the mouse on a file, or select by key and press return'), false);

        $paned = new GtkHPaned();
        $paned->set_position(200);

        //filename, markup-filename, fullpath, is_dir, preview image
        $mFile = new GtkListStore(GObject::TYPE_STRING, GObject::TYPE_STRING, GObject::TYPE_STRING, GObject::TYPE_BOOLEAN, GdkPixbuf::gtype);
        $mFile->set_sort_column_id(0, Gtk::SORT_ASCENDING);

        $vFile = new GtkTreeView($mFile);
        $col = new GtkTreeViewColumn(
                'Image',
                new GtkCellRendererPixbuf(),
                'pixbuf',
                4
            );
        $text = new GtkCellRendererText();
        $col->pack_start($text);
        $col->add_attribute($text, 'markup', 1);
        $vFile->append_column($col);
        $vFile->set_headers_visible(false);

        $vFile->connect('key-press-event', array($this, 'onPressFile'));
        $vFile->connect('button-press-event', array($this, 'onPressFile'));
/*
    GtkIconView has some problems with text that are too long
      and missing icons
        $vFile = new GtkIconView();
        $vFile->set_model($mFile);
        $vFile->set_columns(1);
        $vFile->set_pixbuf_column(3);
        $vFile->set_text_column(0);
        $vFile->set_item_width(100);
*/
        $this->loadFiles($mFile, getcwd());

        $scrwndFiles = new GtkScrolledWindow();
        $scrwndFiles->add($vFile);
        $scrwndFiles->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

        $vboxFile = new GtkVBox();
        $vboxFile->pack_start($scrwndFiles);

        $chkImg = new GtkCheckbutton('Load preview images');
        $chkImg->set_active(true);
        $chkImg->connect('toggled', array($this, 'onCheckPreview'));
        $vboxFile->pack_start($chkImg, false);
        $paned->add1($vboxFile);


        $this->img = new GtkImage();

        $scrwndImg = new GtkScrolledWindow();
        $scrwndImg->add_with_viewport($this->img);
        $scrwndImg->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        $paned->add2($scrwndImg);

        $box->pack_end($paned);
        return $box;
    }



    function loadFiles($mFile, $strDir)
    {
        $hdl = dir($strDir);
        $mFile->clear();
        $pb = null;
        while (false !== ($file = $hdl->read())) {
            $path = $hdl->path . DIRECTORY_SEPARATOR . $file;
            if (is_dir($path) && $file !== '.') {
                $mFile->append(array($file, '<span color="#00F">' . $file . '</span>', $path, true, null));
            } else if (substr($path, -4) == '.png' || substr($path, -4) == '.jpg') {
                if ($this->bLoadPreview) {
                    $pb = GdkPixbuf::new_from_file($path)->scale_simple(32, 32, Gdk::INTERP_BILINEAR);
                }
                $mFile->append(array($file, $file, $path, false, $pb));
            }
        }
    }



    function onPressFile($vFile, $event)
    {
        if (
            ($event->type == Gdk::KEY_PRESS && $event->keyval == Gdk::KEY_Return)
         || ($event->type == Gdk::_2BUTTON_PRESS && $event->button == 1)
        ) {
            list($model, $iter) = $vFile->get_selection()->get_selected();
            if ($iter !== null) {
                $path = $model->get_value($iter, 2);
                if ($model->get_value($iter, 3)) {
                    $this->loadFiles($model, $path);
                } else {
                    $this->showImage($path);
                }
            }
        }
    }



    public function onCheckPreview($chkImg)
    {
        $this->bLoadPreview = $chkImg->get_active();
    }



    function showImage($path)
    {
        $this->img->set_from_file($path);
    }

}

$GLOBALS['class']       = 'ImgBrowser';
$GLOBALS['description'] = 'Uses GdkPixbuf and GtkTreeView to show images
from your hard disk.';

if (!@$GLOBALS['framework']) {
    new ImgBrowser();
    Gtk::main();
}
?>