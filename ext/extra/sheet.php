<?php
error_reporting('E_ALL');
if (!extension_loaded('gtk')) {
	dl( 'php_gtk.' . PHP_SHLIB_SUFFIX);
}
 
$window = &new GtkWindow(GTK_WINDOW_TOPLEVEL);
$scrolledwindow = &new GtkScrolledWindow();

$sheet   = &new GtkSheet(400,400,"test"); 


$scrolledwindow->add($sheet); 

$window->add($scrolledwindow);
  
//$scrolledwindow->show();   
//$sheet->show(); 
$window->show_all();
echo "MAIN";
gtk::main();
  
echo "DONE?";
 
?>
