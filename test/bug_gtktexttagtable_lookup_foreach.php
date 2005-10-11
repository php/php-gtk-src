<?php
function varDumpTag($tag)
{
  var_dump($tag);
}

$view      = new GtkTextView();
$buffer    = $view->get_buffer();
$textTable = $buffer->get_tag_table();

$tag = new GtkTextTag('Bob');
$textTable->add($tag);
$textTable->lookup('Bob');
$textTable->foreach('varDumpTag');
?>