--TEST--
GtkInfoBar->add_button method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkInfoBar', 'add_button')) die('skip - GtkInfoBar->add_button not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$win = new GtkWindow();
$ib = new GtkInfoBar();
$win->add($ib);
$btn = $ib->add_button("TestButton", 1);
echo get_class($btn)."\n";
$ib->add_button();
?>
--EXPECTF--
GtkButton

Warning: GtkInfoBar::add_button() requires exactly 2 arguments, 0 given in %s on line %d
