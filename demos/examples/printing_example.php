<?php
function request_page_setup($operation, $context, $page_nr, $setup)
{
	if($page_nr == 1)
	{
		$a5 = new GtkPaperSize("iso_a5");
		$setup->set_orientation(Gtk::PAGE_ORIENTATION_LANDSCAPE);
		$setup->set_paper_size($a5);
	}
}

function draw_page($operation, $context, $page_nr)
{
	$con = $context->get_cairo_context();
	$con->setSourceRgb(1,0,0);
	$con->rectangle(0,0,20,20);
	$con->stroke();
}

$print = new GtkPrintOperation();
$print->set_n_pages(2);
$print->set_unit(Gtk::UNIT_MM);
$print->set_export_filename(dirname(__FILE__) . "/test.pdf");
$print->connect("draw_page","draw_page", NULL);
$print->connect("request_page_setup", "request_page_setup", NULL);
$res = $print->run(Gtk::PRINT_OPERATION_ACTION_EXPORT, NULL);
?>
