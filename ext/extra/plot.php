<?php
/* $Id$ */

if (!extension_loaded('gtk')) {
	dl( 'php_gtk.' . PHP_SHLIB_SUFFIX);
}

$window = &new GtkWindow();
$window->set_policy(false, false, false);
$window->connect_object('destroy', array('gtk', 'main_quit'));

$canvas = &new GtkPlotCanvas(400, 400, 1);
$plot = &new GtkPlot(NULL, 0.7, 0.7);

$plot->axis_set_visible(GTK_PLOT_AXIS_TOP, FALSE);
$plot->axis_set_visible(GTK_PLOT_AXIS_RIGHT, FALSE);

$plot->axis_set_title(GTK_PLOT_AXIS_BOTTOM, "x");
$plot->axis_set_title(GTK_PLOT_AXIS_LEFT, "y");

$plot->set_xrange(0, 1.0);
$plot->set_yrange(0, .8);

$data = &new GtkPlotData();
$plot->add_data($data);
$data->show();
$data->set_legend("usage");
$data->set_points(
			array(0.0, 0.2, 0.4, 0.6, 0.8, 1.0),
 			array(.2, .4, .5, .35, .30, .40),
			array (.2, .2, .2, .2, .4, .2),
			array (.05, .05, .05, .05, .05, .05),
			6);
$cmap = gdk::colormap_get_system();
$color = $cmap->alloc('#e81418');
$data->set_line_attributes(GTK_PLOT_LINE_SOLID, 2, $color);
$data->set_connector(GTK_PLOT_CONNECT_SPLINE);
$data->show_yerrbars();

/* position of plot on canvas */
$canvas->add_plot($plot, 0.15, 0.15);

$window->add($canvas);
$window->show_all();

gtk::main();

?>
