--TEST--
GtkInfoBar->add_action_widget method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkInfoBar', 'add_action_widget')) die('skip - GtkInfoBar->add_action_widget not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$win = new GtkWindow();
$ib = new GtkInfoBar();
$win->add($ib);
$ib->add_action_widget(new GtkButton("test"), 1);
$ib->add_action_widget();
?>
--EXPECTF--
Warning: GtkInfoBar::add_action_widget() requires exactly 2 arguments, 0 given in %s on line %d
