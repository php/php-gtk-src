<?php

$x = new GtkSheet(10, 10, 'test');
$y = new GtkWindow();

$y->add($x);
$y->connect_simple('destroy', array('Gtk', 'main_quit'));
$y->set_title('Testing GtkSheet');
$y->show_all();
Gtk::main();

?>
