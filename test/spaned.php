<?php
/* $Id$ */

    if (!extension_loaded('gtk'))
        dl( 'php_gtk.' . (strstr( PHP_OS, 'WIN') ? 'dll' : 'so'));

    $w = &new GtkWindow;
    $w->set_title('Comparing GtkPaned and GtkSPaned');
    $w->set_border_width(10);
    $w->set_usize(600,300);
    $w->connect('destroy', create_function('','Gtk::main_quit();'));

    $hbox = &new GtkHBox;
    $w->add($hbox);

    $frameleft = &new GtkFrame('GtkHPaned');
    $hbox->pack_start($frameleft);

    $frameright = &new GtkFrame('GtkSHPaned');
    $hbox->pack_start($frameright);

    $vpaned = &new GtkVPaned;
    $frameleft->add($vpaned);

    $tleft = &new GtkText;
    $tleft->insert_text('They behave the same', 0);
    $vpaned->pack1($tleft);
    $vpaned->pack2(new GtkText());

    $svpaned = &new GtkSVPaned;
    $frameright->add($svpaned);

    $tright = &new GtkText;
    $tright->insert_text('But the handle is much easier to grab', 0);
    $svpaned->pack1($tright);
    $svpaned->pack2(new GtkText());

    $w->show_all();

    Gtk::main();
?>
