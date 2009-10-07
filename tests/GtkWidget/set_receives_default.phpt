--TEST--
GtkWidget->set_receives_default method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'set_receives_default')) die('skip - GtkWidget->set_receives_default not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();

// underlying implementation changes with gtk 2.18 - but looks same to userland
$window->set_receives_default(true);

var_dump($window->get_receives_default());

$window->set_receives_default(false);

var_dump($window->get_receives_default());

/* Wrong number args*/
$window->set_receives_default();
$window->set_receives_default(true, 1);

/* arg should be boolean or cast to boolean*/
$window->set_receives_default(array());
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::set_receives_default() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkWidget::set_receives_default() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkWidget::set_receives_default() expects argument 1 to be boolean, array given in %s on line %d