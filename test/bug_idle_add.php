<?php
function cb() {
}
//segfaults
Gtk::idle_add('cb');
Gtk::main();
?>