
<?php
/* $Id$ */

if (!extension_loaded('gtk')) {
	dl( 'php_gtk.' . PHP_SHLIB_SUFFIX);
}


/*
 * Called when delete-event happens. Returns false to indicate that the event
 * should proceed.
 */
function delete_event()
{
	return false;
}

/*
 * Called when the window is being destroyed. Simply quit the main loop.
 */
function destroy()
{
	Gtk::main_quit();
}

/*
 * Called when button is clicked. Print the message and destroy the window.
 */
function hello()
{
	global	$window;
	print "Hello World!\n";
	$window->destroy();
}

/*
 * Create a new top-level window and connect the signals to the appropriate
 * functions. Note that all constructors must be assigned by reference.
 */
$window = &new GtkWindow();
$window->connect('destroy', 'destroy');
$window->connect('delete-event', 'delete_event');
$window->set_border_width(10);

/*
 * Create a button, connect its clicked signal to hello() function and add
 * the button to the window.
 */
$color = &new GdkColor();
$color = gdk::color_parse(LIGHT_YELLOW );

$button = &new GtkButton('Hello World!');
$button->connect('clicked', 'hello');
$canvas = &new GtkPlotCanvas( 200, 200, 1);
$canvas->Show();
$plot   = &new GtkPlot( NULL, 0.7, 0.7 );
echo GTK_PLOT_AXIS_TOP;
$plot->axis_set_visible( GTK_PLOT_AXIS_TOP, FALSE);
$plot->axis_set_title( GTK_PLOT_AXIS_BOTTOM, "bottom" );
$plot->axis_set_title( GTK_PLOT_AXIS_LEFT, "left" );
$plot->axis_set_visible( GTK_PLOT_AXIS_RIGHT, FALSE );
$plot->set_xrange( 0, .5 );
$plot->set_yrange( 0, .5 );
$plot->Show();
$canvas->add_plot( $plot, 0.15, 0.15);
//$window->add($button);
$window->add($canvas);
$data = &new GtkPlotData();
//var_dump( $data );
$data->Show();
$plot->add_data( $data );
$data->set_legend( "prueba");
$data->set_points( array( .1,.3,.4,.5,.6,.7,.8,.9),
		   array( .1,.2,.3,.4,.5,.6,.7,.8),
 		   array( .2,.2,.2,.2,.2,.2,.2,.2), 
		   array( .1,.1,.1,.1,.1,.1,.1,.1), 8);
//$class_methods = get_class_methods(get_class($data));
var_dump( $class_methods);
/*
 * Create a new tooltips object and use it to set a tooltip for the button.
 */
$tt = &new GtkTooltips();
$tt->set_delay(200);
$tt->set_tip($button, 'Prints "Hello World!"', '');
$tt->enable();

/*
 * Show the window and all its child widgets.
 */
$window->show_all();

/* Run the main loop. */
Gtk::main();

?>
