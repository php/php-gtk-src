<?php
error_reporting('E_ALL');
dl('php_gtk.so');
 
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
