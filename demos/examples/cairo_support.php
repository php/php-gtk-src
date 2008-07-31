<?php
class Clock extends GtkDrawingArea {

	public $__gsignals = array(
				'time_changed' => array(
					GObject::SIGNAL_RUN_FIRST, GObject::TYPE_NONE,
					array(GObject::TYPE_LONG, GObject::TYPE_LONG)
				),
			);

	private $time;
	private $minute_offset = 0;
	private $dragging = false;

	public function __construct() {
		parent::__construct();
		$this->connect('expose-event', array($this, 'expose'));
		$this->connect('button-press-event', array($this, 'button_press'));
		$this->connect('button-release-event', array($this, 'button_release'));
		$this->connect('motion-notify-event', array($this, 'motion_notify'));

		$this->add_events(Gdk::BUTTON_PRESS_MASK |
                        Gdk::BUTTON_RELEASE_MASK |
                        Gdk::POINTER_MOTION_MASK);

		$this->update();
		Gtk::timeout_add(1000, array($this, 'update'));
	}

	public function expose($widget, $event) {
		$context = $this->window->cairo_create();
		$context->rectangle($event->area->x, $event->area->y,
							$event->area->width, $event->area->height);
		$context->clip();
		$this->draw($context);
	}

	public function update() {
		$this->time = getdate();
        $this->redraw_canvas();
		return true;
	}

	protected function redraw_canvas() {
		if($this->window) {
			$alloc = $this->get_allocation();
			$rect = new GdkRectangle($alloc->x, $alloc->y, $alloc->width, $alloc->height);
			$this->window->invalidate_rect($rect, true);
			$this->queue_draw_area($alloc->x, $alloc->y, $alloc->width, $alloc->height);
			$this->window->process_updates(true);
		}
	}

	public function motion_notify($widget, $event) {
		if($this->dragging) {
			$x =$event->x - ($this->get_allocation()->width / 2);
			$y = $event->y - ($this->get_allocation()->height / 2);

			$phi = atan2($x, -$y);
			if($phi < 0) {
				$phi += pi() * 2;
			}

			$hour = $this->time['hours'];
			$minute = $phi * 30 / pi();

			$this->minute_offset = $minute - $this->time['minutes'];
			$this->redraw_canvas();
			$this->emit('time_changed', $hour, $minute);
		}
	}

	public function button_press($widget, $event) {
        $minutes = $this->time['minutes'] + $this->minute_offset;

        $px = $event->x - $widget->get_allocation()->width / 2;
        $py = $widget->get_allocation()->height / 2 - $event->y;
        $lx = sin(pi() / 30 * $minutes);
        $ly = cos(pi() / 30 * $minutes);
        $u = $lx * $px + $ly * $py;

        if ($u < 0)
            return;

        $d2 = pow($px - $u * $lx, 2) + pow($py - $u * $ly, 2);

        if(d2 < 25) {
            $this->dragging = true;
            echo "got minute hand\n";
		}
	}

	public function button_release($widget, $event) {
        if($this->dragging) {
			$this->motion_notify($widget, $event);
			$this->dragging = false;
		}
	}

	public function draw($context) {
		$rect = $this->get_allocation();

		$x = $rect->x + $rect->width / 2.0;
		$y = $rect->y + $rect->height / 2.0;
		$radius = min($rect->width / 2.0, $rect->height / 2.0) - 5;

		// clock background
		$context->arc($x, $y, $radius, 0, 2.0 * pi());
		$context->setSourceRgb(1, 1, 1);
        $context->fillPreserve();
        $context->setSourceRgb(0, 0, 0);
        $context->stroke();

		// clock ticks
		foreach(range(1, 12) as $i) {
			$context->save();

			if($i % 3 == 0) {
				$inset = 0.2 * $radius;
			} else {
				$inset = 0.1 * $radius;
				$context->setLineWidth(0.5 * $context->getLineWidth());
			}

            $context->moveTo($x + ($radius - $inset) * cos($i * pi() / 6.0),
                            $y + ($radius - $inset) * sin($i * pi() / 6.0));
            $context->lineTo($x + $radius * cos($i * pi() / 6.0),
                            $y + $radius * sin($i * pi() / 6.0));
            $context->stroke();
            $context->restore();
		}

		// clock hands
        $hours = $this->time['hours'];
        $minutes = $this->time['minutes'] + $this->minute_offset;
        $seconds = $this->time['seconds'];

        // hour hand
        //the hour hand is rotated 30 degrees (pi/6 r) per hour +  1/2 a degree (pi/360) per minute
        $context->save();
        $context->setLineWidth(2.5 * $context->getLineWidth());
        $context->moveTo($x, $y);
        $context->lineTo($x + $radius / 2 * sin(
            pi() / 6 * $hours + pi() / 360 * $minutes),
                        $y + $radius / 2 * - cos(
            pi() / 6 * $hours + pi() / 360 * $minutes));
        $context->stroke();
        $context->restore();

        // minute hand
        // the minute hand is rotated 6 degrees (pi/30 r) per minute
        $context->moveTo($x, $y);
        $context->lineTo($x + $radius * 0.75 * sin(pi() / 30 * $minutes),
                        $y + $radius * 0.75 * - cos(pi() / 30 * $minutes));
        $context->stroke();

        // seconds hand
        $context->save();
        $context->setSourceRgb(1, 0, 0); // red
        $context->moveTo($x, $y);
        $context->lineTo($x + $radius * 0.7 * sin(pi() / 30 * $seconds),
                        $y + $radius * 0.7 * - cos(pi() / 30 * $seconds));
        $context->stroke();
        $context->restore();
	}

}
Gobject::register_type('Clock');

function time_changed_cb($widget, $hours, $minutes) {
    echo "time-changed - $hours:$minutes\n";
}

$window = new GtkWindow();
$clock = new Clock();
$window->add($clock);
$window->connect_simple('destroy', array('Gtk', 'main_quit'));
$clock->connect('time_changed', 'time_changed_cb');
$window->show_all();

Gtk::main();
?>




