<?php

function cb()
{
    throw new Exception('Will not get displayed anywhere');
}

Gtk::idle_add('cb');
Gtk::main();
?>