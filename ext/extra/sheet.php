<?php
function test_traverse() {
	print_r( func_get_args());
}
error_reporting('E_ALL');

if (!extension_loaded('gtk')) {
	dl( 'php_gtk.' . PHP_SHLIB_SUFFIX);
}
 
$window = &new GtkWindow();
$window->connect_object('destroy', array('gtk', 'main_quit'));
$window->set_position(GTK_WIN_POS_CENTER);
$window->set_default_size(gdk::screen_width(), gdk::screen_height()-20);

$scrolledwindow = &new GtkScrolledWindow();

$sheet   = &new GtkSheet(100,100,"test"); 
$sheet->connect('traverse','test_traverse');

$scrolledwindow->add($sheet); 

$window->add($scrolledwindow);
$window->show_all();

gtk::main();

?>
