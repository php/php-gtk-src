<?php
/* $Id$ */

    if (!extension_loaded('gtk'))
        dl( 'php_gtk.' . (strstr( PHP_OS, 'WIN') ? 'dll' : 'so'));

    $w = &new GtkWindow;
    $w->set_title('GtkComboButton Example');
    $w->set_border_width(20);
    $w->connect('destroy', create_function('','Gtk::main_quit();'));

	$table = &new GtkTable(2,3,true);
	$w->add($table);

	$row = 0; $column = 0;
	
	for ($i = GTK_ARROW_UP; $i <= GTK_ARROW_RIGHT; $i++) {

		$menu = &new GtkMenu;

		for ($j = 0; $j < 10; $j++) {

			$menuitem = &new GtkMenuItem("Menu item $j");
			$menuitem->show();
			$menuitem->connect_object('activate', 'menuitem_activate', $j);
			$menu->append($menuitem);
		}

		$combobutton = &new GtkComboButton($menu);
		$combobutton->connect_object('clicked', 'combobutton_clicked', $i);
		$combobutton->set_arrow_direction($i);
		$table->attach_defaults($combobutton, $column,
											  ++$column,
											  $row,
											  $row + 1);
		if ($column == 2) {
			$column = 0;
			$row++;
		}
	}

	$othermenu = &new GtkMenu;
	for ($j = 0; $j < 10; $j++) {

		$menuitem = &new GtkMenuItem("Menu item $j");
		$menuitem->show();
		$menuitem->connect_object('activate', 'menuitem_activate', $j);
		$othermenu->append($menuitem);
	}

	$othercombo = &new GtkComboButton($menu, true);
	$table->attach_defaults($othercombo, 2, 3, 0, 2);

	$w->show_all();

    Gtk::main();

    function combobutton_clicked($number) {

		echo "Combobutton number $number was clicked.\n";
    }

    function menuitem_activate($number) {
		
		echo "Menuitem number $number acitvated.\n";
    }
?>
