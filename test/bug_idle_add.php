<?php
//idle_add segfault
//Fixed by andrei 2006-03-16
//Reason: Uninitialized variable
function cb() {
}
//segfaults
Gtk::idle_add('cb');
Gtk::main();
?>