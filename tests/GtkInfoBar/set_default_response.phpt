--TEST--
GtkInfoBar->set_default_response method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkInfoBar', 'set_default_response')) die('skip - GtkInfoBar->set_default_response not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$win = new GtkWindow();
$ib = new GtkInfoBar();
$win->add($ib);
$ib->add_button("TestButton", 1);
$ib->set_default_response(1);
$ib->set_default_response();
?>
--EXPECTF--
Warning: GtkInfoBar::set_default_response() requires exactly 1 argument, 0 given in %s on line %d
