<?php
function printSelected($buffer, $iter, $mark)
{
	// Get the mark that wasn't moved. 
	if ($mark == $buffer->get_mark('insert')) {
		$mark2 = $buffer->get_mark('selection_bound');
	} else {
		$mark2 = $buffer->get_mark('insert');
	}
	// Get the iter at the other mark.
	$iter2 = $buffer->get_iter_at_offset(0);
	$buffer->get_iter_at_mark($iter2, $mark2);

	echo 'Iter1: ' . $iter->get_offset()  . "\t";
	echo 'Iter2: ' . $iter2->get_offset() . "\t";

	// Print the text between the two iters.
	echo 'SELECTION: ' . $buffer->get_text($iter, $iter2) . "\n";
}

// Create a GtkTextView.
$text = new GtkTextView();
// Get the buffer from the view.
$buffer = $text->get_buffer();

// Add some text.
$buffer->set_text('Moving a mark is done with either move_mark or move_mark_by_name.');

// Connect the printSelected method.
$buffer->connect('mark-set', 'printSelected');

// How NOT to move the cursor to the beginning of the text.
echo "Move to start\n";
$buffer->move_mark_by_name('insert',          $buffer->get_start_iter());
$buffer->move_mark_by_name('selection_bound', $buffer->get_start_iter());

// How NOT to select a range of text.
echo "Select range\n";
$buffer->move_mark_by_name('selection_bound', $buffer->get_iter_at_offset(7));
$buffer->move_mark_by_name('insert',          $buffer->get_iter_at_offset(16));

// The better way to move the cursor to the beginning of the text.
echo "Move to start\n";
$buffer->place_cursor($buffer->get_start_iter());

// The better way to select a range of text.
echo "Select range\n";
$buffer->select_range($buffer->get_iter_at_offset(7), $buffer->get_iter_at_offset(16));
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
?>