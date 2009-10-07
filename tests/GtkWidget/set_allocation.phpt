--TEST--
GtkWidget->set_allocation method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'set_allocation')) die('skip - GtkWidget->set_allocation not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();

// can use new gdkrectangle or array with 4 parts
$window->set_allocation(array(2, 2, 2, 2));
$window->set_allocation(null);
$allocation = new GdkRectangle(2, 2, 2, 2);
$window->set_allocation($allocation);

/* Wrong number args*/
$window->set_allocation();
$window->set_allocation(null, 1);

/* wants array to change or gdkrectangle instance or null */
$window->set_allocation(array());
?>
--EXPECTF--
Warning: GtkWidget::set_allocation() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkWidget::set_allocation() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkWidget::set_allocation() requires exactly 4 arguments, 0 given in %s on line %d

Warning: unable to convert to type GdkRectangle in %s on line %d

Warning: GtkWidget::set_allocation() expects allocation argument to be a 4-element array, a GdkRectangle object, or null in %s on line %d