<?php

dl('php_gtk.so');

if ($argc < 2)
	die("\nUsage: php -q gdk_pixbuf.php <imagefile>\n\n");

$image_file = $argv[1];

$window = &new GtkWindow();
$window->set_border_width(5);
$window->set_title('GdkPixbuf example');
$window->set_policy(false, false, true);
$window->connect_object('destroy', array('gtk', 'main_quit'));

$pixbuf = GdkPixbuf::new_from_file($image_file);
if (!$pixbuf) {
	die("\nCouldn't load $image_file\n\n");
}

$font = gdk::font_load("-*-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*");
$extents = $font->extents($image_file);

$area = &new GtkDrawingArea();
$area->size($pixbuf->get_width(), $pixbuf->get_height());
$area->connect('expose_event', 'expose_event', $pixbuf, $font, $extents, basename($image_file));
$frame = &new GtkFrame();
$frame->add($area);
$frame->set_shadow_type(GTK_SHADOW_IN);
$window->add($frame);

$window->show_all();

gtk::main();

function expose_event($area, $event, $pixbuf, $font, $extents, $image_file)
{
	$pixbuf->render_to_drawable($area->window,
								$area->style->fg_gc[GTK_STATE_NORMAL],
								$event->area->x, $event->area->y,
								$event->area->x, $event->area->y,
								$event->area->width, $event->area->height,
								GDK_RGB_DITHER_NORMAL, 0, 0);

	if ($event->area->x < $extents[2] + 3 &&
		$event->area->y < $extents[3] + $extents[4] + 3) {
		gdk::draw_string($area->window, $font, $area->style->white_gc, 3, 3 +
						 $extents[3] + $extents[4], $image_file);
		gdk::draw_string($area->window, $font, $area->style->black_gc, 2, 2 +
						 $extents[3] + $extents[4], $image_file);
	}
}

?>
