<?php
/* $Id$ */

	if (!class_exists('gtk')) {
		dl( 'php_gtk.' . (strstr( PHP_OS, 'WIN') ? 'dll' : 'so'));
	}


	$w = &new GtkWindow;
	$w->set_border_width(10);
	$w->set_usize(400,400);
	$w->connect('delete_event', create_function('', 'Gtk::main_quit();'));


	$sq = &new GtkSQPane;
	$sq->set_handle_size(12);
	$w->add($sq);
	
	$text = &new GtkText;
	$text->set_editable(true);
	$text->insert_text("Pick the handle\nin the middle to\nresize all four sectors\nat once",0);
	$sq->pack1($text);

	$scroll = &new GtkScrolledWindow;
	$sq->pack2($scroll);
	$list = &new GtkCList(1, array('List'));
	foreach (get_declared_classes() as $class)
		$list->append(array($class));
	$scroll->add($list);

	$scroll2 = &new GtkScrolledWindow;
	$list2 = &new GtkCList(2, array('Constant', 'Value'));
	foreach (get_defined_constants() as $const => $value)
		$list2->append(array($const, $value));
	$scroll2->add($list2);
	$sq->pack3($scroll2);

	$frame = &new GtkFrame('Functions');
	$nb = &new GtkNotebook;
		$scroll3 = &new GtkScrolledWindow;
		$internal = &new GtkCList(1, array('Function'));
		$scroll3->add($internal);

		$scroll4 = &new GtkScrolledWindow;
		$user = &new GtkCList(1, array('Function'));
		$scroll4->add($user);

		foreach (get_defined_functions() as $where => $funcs)
			foreach ($funcs as $func)
				$$where->append(array($func));

		$nb->append_page($scroll3, new GtkLabel('Internal'));
		$nb->append_page($scroll4, new GtkLabel('User'));

	$frame->add($nb);
	$sq->pack4($frame);


	$w->show_all();

	Gtk::main();
?>
