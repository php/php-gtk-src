<?php
/* $Id$ */

    if (!extension_loaded('gtk'))
        dl( 'php_gtk.' . (strstr( PHP_OS, 'WIN') ? 'dll' : 'so'));

    $w = &new GtkWindow;
    $w->set_title('GtkPieMenu Example');
    $w->set_border_width(20);
    $w->connect('destroy', create_function('','Gtk::main_quit();'));

    $button = &new GtkButton('Press button to activate PieMenu');
    $button->connect('button_press_event', 'activate_pie_menu');
    $w->add($button);

    $piemenu = &new GtkPieMenu;

    foreach (array('North', 'East', 'South', 'West') as $d) {
        $m = &new GtkMenuItem($d);
        $m->connect_object('activate', 'menu_callback', $d);
        $piemenu->append($m);
    }

    $piemenu->show_all();

    $w->show_all();

    Gtk::main();

    function activate_pie_menu($w, $e) {
        global $piemenu;

        $piemenu->popup($e->button, $e->time);
    }

    function menu_callback($b) {
        echo "'$b' button pressed\n";
    }
?>
