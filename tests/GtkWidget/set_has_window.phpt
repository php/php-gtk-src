--TEST--
GtkWidget->set_has_window method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'set_has_window')) die('skip - GtkWidget->set_has_window not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();

// underlying implementation changes with gtk 2.18 - but looks same to userland
$window->set_has_window(true);

var_dump($window->get_has_window());

$window->set_has_window(false);

var_dump($window->get_has_window());

/* Wrong number args*/
$window->set_has_window();
$window->set_has_window(true, 1);

/* arg should be boolean or cast to boolean*/
$window->set_has_window(array());
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::set_has_window() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkWidget::set_has_window() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkWidget::set_has_window() expects argument 1 to be boolean, array given in %s on line %d