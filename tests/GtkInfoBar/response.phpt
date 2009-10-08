--TEST--
GtkInfoBar->response method
--SKIPIF--
<?php
if(!extension_loaded('php-gtk')) die('skip - PHP-GTK extension not available');
if(!method_exists('GtkInfoBar', 'response')) die('skip - GtkInfoBar->response not available, requires GTK 2.18 or higher');
?>
--FILE--
<?php
$win = new GtkWindow();
$ib = new GtkInfoBar();
$win->add($ib);
$ib->add_button("TestButton", 1);
function test_response ($widget, $id, $data) {
	var_dump($id == 1);
}
$ib->connect("response", "test_response", NULL);
$ib->response(1);
$ib->response();
?>
--EXPECTF--
bool(true)

Warning: GtkInfoBar::response() requires exactly 1 argument, 0 given in %s on line %d
