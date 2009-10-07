--TEST--
GtkWidget->is_toplevel method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkWidget', 'is_toplevel')) die('skip - GtkWidget->is_toplevel not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$window = new GtkWindow();
var_dump($window->is_toplevel());

$button = new GtkButton();
var_dump($button->is_toplevel());

/* Wrong number args*/
$window->is_toplevel(1);

/* Takes no args, so no arg type checking */
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::is_toplevel() expects exactly 0 parameters, 1 given in %s on line %d