<?php
/* Create base window */
$window = new GtkWindow();
$window->set_title('Notebooks and Pages');
$window->connect_simple('destroy', array('gtk', 'main_quit'));
$window->add($vbox = new GtkHBox());
$group = 'MyGroup';

$vbox->add($notebook1 = new GtkNoteBook());
$notebook1->append_page($label = new GtkLabel('This is the first page'));
$notebook1->set_tab_detachable($label, true);
$notebook1->append_page($label = new GtkLabel('This is the second page'));
$notebook1->set_tab_detachable($label, true);
$notebook1->append_page($label = new GtkLabel('This is the third page'));
$notebook1->set_tab_detachable($label, true);
$notebook1->set_group($group);
var_dump($notebook1->get_group());

$vbox->add($notebook2 = new GtkNoteBook());
$notebook2->append_page($label = new GtkLabel('This is the first page'));
$notebook2->set_tab_detachable($label, true);
$notebook2->append_page($label = new GtkLabel('This is the second page'));
$notebook2->set_tab_detachable($label, true);
$notebook2->append_page($label = new GtkLabel('This is the third page'));
$notebook2->set_tab_detachable($label, true);
$notebook2->set_group($group);
var_dump($notebook2->get_group());

unset($group);

$vbox->add($notebook3 = new GtkNoteBook());
$notebook3->append_page($label = new GtkLabel('This is the first page'));
$notebook3->set_tab_detachable($label, true);
$notebook3->append_page($label = new GtkLabel('This is the second page'));
$notebook3->set_tab_detachable($label, true);
$notebook3->append_page($label = new GtkLabel('This is the third page'));
$notebook3->set_tab_detachable($label, true);
$notebook3->set_group($notebook1);
var_dump($notebook3->get_group());

$window->show_all();
Gtk::main();
?>