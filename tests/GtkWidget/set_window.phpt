--TEST--
GtkWidget->set_window method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'set_window')) die('skip - GtkWidget->set_window not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();

$contents = new GtkWindow();
$contents->realize();

$window->set_window($contents->window);

var_dump($window->window == $contents->window);

/* Wrong number args*/
$window->set_window();
$window->set_window($contents->window, 1);

/* arg should be GdkWindow */
$window->set_window(1);
?>
--EXPECTF--
bool(true)

Warning: GtkWidget::set_window() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkWidget::set_window() requires exactly 1 argument, 2 given in %s on line %d

Catchable fatal error: Argument 1 passed to GtkWidget::set_window() must be an instance of GdkWindow, integer given in %s on line %d