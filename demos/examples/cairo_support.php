<?php
class Clock extends GtkDrawingArea {

	protected $context;

	public function __construct() {
		parent::__construct();
		$this->connect_simple('expose-event', array($this, 'expose'));
	}

	public function expose() {
		$this->realize();
		$this->context = $this->window->cairo_create();
		var_dump($this->context);
		//$this->context->rectangle(1,1,50,50);
		//$this->context->stroke();
	}
}

$window = new GtkWindow();
$clock = new Clock();
$window->add($clock);
$window->connect_simple('destroy', array('Gtk', 'main_quit'));
$window->show_all();

Gtk::main();
?>