<?php
$tooltips = new GtkTooltips(); // there can be only one

function showtips($tipsquery, $widget, $tip_text, $tip_private)
{
	$tipsquery->set_text($tip_private);
	$tipsquery->emit_stop_by_name('widget-entered');
	return false;
}

function starttips($tipsquery, $tooltips)
{
	if($tooltips->enabled)
	{
		$tipsquery->start_query();
		$tooltips->disable();
	}
	else
	{
		// $tipsquery->stop_query(); - this is automatic when the caller button is clicked
		$tooltips->enable();
	}
}

$window = new GtkWindow();
$window->set_title('Tooltip');
$window->connect_simple('destroy', array('gtk', 'main_quit'));

$window->add($vbox = new GtkVBox());
$vbox->pack_start($button = new GtkButton('Button1'));
$tooltips->set_tip($button, 'This is button 1', 'Here is some help for button 1');
$vbox->pack_start($button = new GtkButton('Button2'));
$tooltips->set_tip($button, 'This is button 2. This is also a really long tooltip which probably won\'t fit on a single line and will therefore need to be wrapped. Hopefully the wrapping will work correctly.',
	'Here is some Help for button 2');
$vbox->pack_start($button = new GtkButton('Override TipsQuery Label'));
$tooltips->set_tip($button, 'Toggle TipsQuery view.', 'Help for Help?');

$popup = new GtkWindow(Gtk::WINDOW_POPUP);
$popup->add($tipsquery = new GtkTipsQuery());
$tipsquery->set_labels('Click to get help', 'No help found');
$button->connect_simple('clicked', 'starttips', $tipsquery, $tooltips);
$tipsquery->set_caller($button);
$tipsquery->connect('widget-entered', 'showtips');
$window->show_all();
$popup->show_all();

Gtk::main();
?>
