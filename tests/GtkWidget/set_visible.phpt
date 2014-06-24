--TEST--
GtkWidget->set_receives_default method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
?>
--FILE--
<?php
$window = new GtkWindow();

// underlying implementation changes with gtk 2.18 - but looks same to userland
$window->set_visible(true);

// 2.18+ use the real method - get_visible
if (method_exists('GtkWidget', 'get_visible')) {
    var_dump($window->get_visible());
} else {
    var_dump($window->is_visible());
}

$window->set_visible(false);

// 2.18+ use  the real method - get visible
if (method_exists('GtkWidget', 'get_visible')) {
    var_dump($window->get_visible());
} else {
    var_dump($window->is_visible());
}

/* Wrong number args*/
$window->set_visible();
$window->set_visible(true, 1);

/* arg should be boolean or cast to boolean*/
$window->set_visible(array());
?>
--EXPECTF--
bool(true)
bool(false)

Warning: GtkWidget::set_visible() requires exactly 1 argument, 0 given in %s on line %d

Warning: GtkWidget::set_visible() requires exactly 1 argument, 2 given in %s on line %d

Warning: GtkWidget::set_visible() expects argument 1 to be boolean, array given in %s on line %d