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

$plot->axis_set_title(GTK_PLOT_AXIS_BOTTOM, "y");
$plot->axis_set_title(GTK_PLOT_AXIS_LEFT, " x");

$plot->set_xrange(0, .5);
$plot->set_yrange(0, .5);

$data = &new GtkPlotData();
$data->set_legend("usage");
$data->set_points(
			array(0.1, 0.2, 0.3, 0.4, 0.5),
			array(0.0, 0.2, 0.3, 0.4, 0.5),
 			array(),
			array(), 5);
$plot->add_data($data);
$data->fill_area(true);

/* position of plot on canvas */
$canvas->add_plot($plot, 0.15, 0.15);

$window->add($canvas);
$window->show_all();

gtk::main();

?>
