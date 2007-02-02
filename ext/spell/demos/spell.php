<?php

$w= new GtkWindow();
$t = new GtkTextBuffer();
$v = new GtkTextView($t);

$s= new GtkSpell($v);
$s->set_language('en');

$w->set_title('Spell Check');
$w->set_size_request(200, 200);
$w->add($v);
$w->show_all();

$w->connect_simple('destroy', array('Gtk', 'main_quit'));

Gtk::main();

?>

