<?php
/**
*   Scribble demo - simple painting with your mouse
*/

class Scribble extends GtkWindow
{
    protected $size_group = null;
    protected $pixmap = null;


    function __construct($parent = null)
    {
        parent::__construct();

        if (@$GLOBALS['framework']) {
            return;
        }

        if ($parent)
            $this->set_screen($parent->get_screen());
        else
            $this->connect_simple('destroy', array('gtk', 'main_quit'));

        $this->set_title(__CLASS__);
        $this->set_position(Gtk::WIN_POS_CENTER);
        $this->set_default_size(-1, -1);
        $this->set_border_width(8);

        $this->add($this->__create_box());
        $this->show_all();
    }//function __construct($parent = null)



    function __create_box()
    {
        $vbox = new GtkVBox();
        $vbox->show();

        $drawing_area = new GtkDrawingArea();
        $drawing_area->set_size_request(300, 300);
        $vbox->pack_start($drawing_area);
//        $drawing_area->realize();

        $drawing_area->connect('expose_event'       , array($this, 'expose_event'));
        $drawing_area->connect('configure_event'    , array($this, 'configure_event'));

        $drawing_area->connect('motion_notify_event', array($this, 'motion_notify_event'));
        $drawing_area->connect('button_press_event' , array($this, 'button_press_event'));

        $drawing_area->set_events(Gdk::EXPOSURE_MASK
                                | Gdk::LEAVE_NOTIFY_MASK
                                | Gdk::BUTTON_PRESS_MASK
                                | Gdk::POINTER_MOTION_MASK
                                | Gdk::POINTER_MOTION_HINT_MASK);

        return $vbox;
    }//function __create_box()



    function configure_event($widget, $event)
    {
        $this->pixmap = new GdkPixmap($widget->window,
                                $widget->allocation->width,
                                $widget->allocation->height,
                                -1);
        $this->pixmap->draw_rectangle($widget->style->white_gc,
                            true, 0, 0,
                            $widget->allocation->width,
                            $widget->allocation->height);
        return true;
    }


    function expose_event($widget, $event)
    {
        $widget->window->draw_drawable($widget->style->fg_gc[$widget->state],
                        $this->pixmap,
                        $event->area->x, $event->area->y,
                        $event->area->x, $event->area->y,
                        $event->area->width, $event->area->height);

        return false;
    }



    function button_press_event($widget, $event)
    {
        if ($event->button == 1 && $this->pixmap) {
            $this->draw_brush($widget, (int)$event->x, (int)$event->y);
        }

        return true;
    }



    function motion_notify_event($widget, $event)
    {
        $window  = $event->window;
        $pointer = $window->get_pointer();
        $x = $pointer[0];
        $y = $pointer[1];
        $state = $pointer[2];

        if (($state & Gdk::BUTTON1_MASK) && $this->pixmap) {
            $this->draw_brush($widget, $x, $y);
        }

        return true;
    }



    function draw_brush($widget, $x, $y)
    {
        $this->pixmap->draw_arc($widget->style->black_gc, true, $x - 4, $y - 4, 8, 8, 0, 64 * 360);
        $widget->queue_draw_area($x - 4, $y - 4, 8, 8);
    }
}//class Scribble extends GtkWindow


$GLOBALS['class']       = 'Scribble';
$GLOBALS['description'] = "Scribble lets you draw paintings with your mouse";

if (!@$GLOBALS['framework']) {
    new Scribble();
    Gtk::main();
}
?>
