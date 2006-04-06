<?php


class ImgBrowser extends GtkWindow
{
    var $img = null;



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

        //filename, fullpath, is_dir
        $mFile = new GtkListStore(Gtk::TYPE_STRING, Gtk::TYPE_STRING, Gtk::TYPE_BOOLEAN);
        $vFile = new GtkTreeView($mFile);
        $vFile->append_column(
            new GtkTreeViewColumn(
                'Files',
                new GtkCellRendererText(),
                'markup',
                0
            )
        );
        $vFile->connect('key-press-event', array($this, 'onPressFile'));
        $vFile->connect('button-press-event', array($this, 'onPressFile'));
        $this->loadFiles($mFile, getcwd());

        $scrwndFiles = new GtkScrolledWindow();
        $scrwndFiles->add($vFile);
        $scrwndFiles->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

        $paned->add1($scrwndFiles);


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
        while (false !== ($file = $hdl->read())) {
            $path = $hdl->path . DIRECTORY_SEPARATOR . $file;
            if (is_dir($path) && $file !== '.') {
                $mFile->append(array('<span color="#00F">' . $file . '</span>', $path, true));
            } else if (substr($path, -4) == '.png' || substr($path, -4) == '.jpg') {
                $mFile->append(array($file, $path, false));
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
                $path = $model->get_value($iter, 1);
                if ($model->get_value($iter, 2)) {
                    $this->loadFiles($model, $path);
                } else {
                    $this->showImage($path);
                }
            }
        }
    }



    function showImage($path)
    {
        $this->img->set_from_file($path);
    }

}

$GLOBALS['class']       = 'ImgBrowser';
$GLOBALS['description'] = 'Uses GdkPixbuf and GtkIconView to show the list of images in folders
of your hard disk.';

if (!@$GLOBALS['framework']) {
    new ImgBrowser();
    Gtk::main();
}
?>