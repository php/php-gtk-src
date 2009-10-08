--TEST--
GtkInfoBar->set_response_sensitive method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkInfoBar', 'set_response_sensitive')) die('skip - GtkInfoBar->set_response_sensitive not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$win = new GtkWindow();
$ib = new GtkInfoBar();
$win->add($ib);
$ib->add_button("TestButton", 1);
$win->show_all();

$ib->set_response_sensitive(1, false);
$ib->set_response_sensitive();
?>
--EXPECTF--
Warning: GtkInfoBar::set_response_sensitive() requires exactly 2 arguments, 0 given in %s on line %d
