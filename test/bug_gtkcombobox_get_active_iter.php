<?php
// FIXED by Christian on 2006-03-06
$cmb = new GtkComboBox();
$mod = new GtkListStore(Gtk::TYPE_STRING);
$cmb->set_model($mod);

$mod->append(array("one"));
$mod->append(array("two"));
$mod->append(array("three"));

$cmb->set_active(1);

//that should return an iter, not require one
var_dump($cmb->get_active_iter());
?>