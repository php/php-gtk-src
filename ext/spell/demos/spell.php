<?php

$wÊ= new GtkWindow();
$t = new GtkTextBuffer();
$v = new GtkTextView($t);

$sÊ= new GtkSpell($v);
$s->set_language('en');

$w->set_title('SpellÊCheck');
$w->set_size_request(200,Ê200);
$w->add($v);
$w->show_all();

$w->connect_simple('destroy', array('Gtk', 'main_quit'));

Gtk::main();

?>

