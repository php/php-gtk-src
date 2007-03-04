<?php

$someHtml = '
    <h1>Hello World!</h1>
    <p><font color="red" size="20">This is GtkHTML</font></p>
    <h2>Bye Bye World!</h2>';

$x = new GtkWindow();

$y = new GtkHTML();
$y->set_title("HTML Test!");
$y->load_from_string($someHtml);

$x->add($y);
$x->show_all();
$x->connect_simple('destroy', array('Gtk', 'main_quit'));

Gtk::main();

?>
