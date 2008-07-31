<?php
/* Create base window */
$window = new GtkWindow();
$window->set_title('Notebooks and Pages');
$window->connect_simple('destroy', array('gtk', 'main_quit'));
$window->add($vbox = new GtkHBox());

$vbox->add($notebook = new GtkNoteBook());
$notebook->append_page($label = new GtkLabel('This is the first page'));
$notebook->set_tab_detachable($label, true);
$notebook->append_page($label = new GtkLabel('This is the second page'));
$notebook->set_tab_detachable($label, true);
$notebook->append_page($label = new GtkLabel('This is the third page'));
$notebook->set_tab_detachable($label, true);
$notebook->set_group('Mygroup');
var_dump($notebook->get_group());

$vbox->add($notebook = new GtkNoteBook());
$notebook->append_page($label = new GtkLabel('This is the first page'));
$notebook->set_tab_detachable($label, true);
$notebook->append_page($label = new GtkLabel('This is the second page'));
$notebook->set_tab_detachable($label, true);
$notebook->append_page($label = new GtkLabel('This is the third page'));
$notebook->set_tab_detachable($label, true);
$notebook->set_group('Mygroup');

var_dump($notebook->get_group());
$window->show_all();
Gtk::main();
?>