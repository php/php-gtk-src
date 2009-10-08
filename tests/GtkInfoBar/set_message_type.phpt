--TEST--
GtkInfoBar->set_message_type method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkInfoBar', 'set_message_type')) die('skip - GtkInfoBar->set_message_type not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$win = new GtkWindow();
$ib = new GtkInfoBar();
$win->add($ib);
$ib->set_message_type(Gtk::MESSAGE_INFO);
var_dump($ib->get_message_type() == Gtk::MESSAGE_INFO);
$ib->set_message_type();
?>
--EXPECTF--
bool(true)

Warning: GtkInfoBar::set_message_type() requires exactly 1 argument, 0 given in %s on line %d

