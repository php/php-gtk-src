--TEST--
GtkWidget->set_can_default method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'set_can_default')) die('skip - GtkWidget->set_can_default not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();

// underlying implementation changes with gtk 2.18 - but looks same to userland
$window->set_can_default(true);

var_dump($window->get_can_default());

$window->set_can_default(false);

var_dump($window->get_can_default());

/* Wrong number args*/
$window->set_can_default();
$window->set_can_default(true, 1);

/* arg should be boolean or cast to boolean*/
$window->set_can_default(array());
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::set_can_default() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkWidget::set_can_default() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkWidget::set_can_default() expects argument 1 to be boolean, array given in %s on line %d