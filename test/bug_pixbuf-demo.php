<?php
define('FRAME_DELAY', 50);
define('BACKGROUND_NAME', "../demos/background.jpg");

$image_names = array(
"apple-red.png",
"gnome-applets.png",
"gnome-calendar.png",
"gnome-foot.png",
"gnome-gmush.png",
"gnome-gimp.png",
"gnome-gsame.png",
"gnu-keys.png"
);

define('N_IMAGES', count($image_names));
define('CYCLE_LEN', 60);

//Drawing area
$da = null;

//GdkPixbuf
$frame;

/* Background image */
//GdkPixbuf
$background;

//int
$back_width; $back_height;

/* Images */
//GdkPixbuf
$images = array();

function load_pixbufs()
{
    global $background, $back_width, $back_height, $images, $image_names;

    $i = 0;
    /* We pass NULL for the error return location, we don't care
     * about the error message.
     */
    $background = GdkPixbuf::new_from_file(BACKGROUND_NAME);
    if (!$background) {
        return false;
    }

    $back_width = $background->get_width();
    $back_height = $background->get_height();

    for ($i = 0; $i < N_IMAGES; $i++) {
        $images[$i] = GdkPixbuf::new_from_file('../demos/' . $image_names[$i]);
        if (!$images[$i])
            return false;
    }

    return true;
}

/* Expose callback for the drawing area */
function expose_cb(GtkWidget $widget, $event, $data = null)
{
    global $frame, $window;

    $rowstride = $frame->get_rowstride();

    //area.x area.y
    $pixels = $frame->get_pixels() + $rowstride * $event->area->y + $event->area->x * 3;

    if ($window->window !== null) {
        $window->window->draw_rgb_image_dithalign(
        $widget->style->black_gc,
        $event->area->x, $event->area->y,
        $event->area->width, $event->area->height,
        Gdk::RGB_DITHER_NORMAL,
        $pixels, $rowstride,
        $event->area->x, $event->area->y);
    }

    return true;
}


/* Timeout handler to regenerate the frame */
function timeout()
{
    global $da, $background, $back_width, $back_height, $frame, $images, $frame_num;

    $background->copy_area(0, 0, $back_width, $back_height, $frame, 0, 0);

    $f = ($frame_num % CYCLE_LEN) / CYCLE_LEN;

    $xmid = $back_width / 2.0;
    $ymid = $back_height / 2.0;

    $radius = min($xmid, $ymid) / 2.0;

    for ($i = 0; $i < N_IMAGES; $i++) {
        $ang = 2.0 * pi() * $i / N_IMAGES - $f * 2.0 * pi();

        $iw = $images[$i]->get_width();
        $ih = $images[$i]->get_height();

        $r = $radius + ($radius / 3.0) * sin($f * 2.0 * pi());

        $xpos = floor ($xmid + $r * cos($ang) - $iw / 2.0 + 0.5);
        $ypos = floor ($ymid + $r * sin($ang) - $ih / 2.0 + 0.5);

        $k = ($i & 1) ? sin($f * 2.0 * pi()) : cos($f * 2.0 * pi());
        $k = 2.0 * $k * $k;
        $k = max(0.25, $k);

        $r1 = new GdkRectangle($xpos, $ypos, $iw * $k, $ih * $k);
        $r2 = new GdkRectangle(0, 0, $back_width, $back_height);

        $dest = $r1->intersect($r2);
        $images[$i]->composite(
            $frame,
            $dest->x, $dest->y,
            $dest->width, $dest->height,
            $xpos, $ypos,
            $k, $k,
            Gdk::INTERP_NEAREST,
            (($i & 1)
            ? max(127, abs(255 * sin($f * 2.0 * pi())))
            : max(127, abs(255 * cos($f * 2.0 * pi())))));
    }

    $da->queue_draw();

    $frame_num++;
    return true;
}

$timeout_id = 0;

//pixbuf_init ();

if (!load_pixbufs()) {
    die("main(): Could not load all the pixbufs!");
}

$frame = new GdkPixbuf(Gdk::COLORSPACE_RGB, false, 8, $back_width, $back_height);
$window = new GtkWindow(Gtk::WINDOW_TOPLEVEL);

$window->set_size_request($back_width, $back_height);
$window->set_resizable(false);
$window->connect_simple('destroy', array('gtk', 'main_quit'));

$da = new GtkDrawingArea();
$da->connect('expose_event', 'expose_cb');

$window->add($da);

Gtk::timeout_add(FRAME_DELAY, 'timeout');

$window->show_all();
Gtk::main();
?>