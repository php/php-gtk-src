<?php
/**
* The exception is not visible in the Gtk::main() loop
* But visible when doing the main_iteration - that's
* because of the while that is a php method.
*
* Change the "if (true)" into "if (false)" to see that
*  the exception is thrown in the while loop
*/
function cb()
{
	echo "Exception will be thrown next line\r\n";
    throw new Exception('Will not get displayed anywhere');
}

Gtk::timeout_add(10, 'cb');

if (true) {
    Gtk::main();
} else {
    sleep(1);
    while (Gtk::events_pending()) {
        Gtk::main_iteration();
    }
}
echo "done throwing exception\r\n";
?>