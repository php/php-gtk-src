--TEST--
GtkInfoBar->get_content_area method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkInfoBar', 'get_content_area')) die('skip - GtkInfoBar->get_content_area not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$win = new GtkWindow();
$ib = new GtkInfoBar();
$win->add($ib);
var_dump($ib->get_content_area() instanceof GtkWidget);
$ib->get_content_area(1);
?>
--EXPECTF--
bool(true)

Warning: GtkInfoBar::get_content_area() expects exactly 0 parameters, 1 given in %s on line %d

