<?

if (strtoupper(substr(PHP_OS, 0, 3)) == 'WIN') {
  echo "SORRY NOT SUPPORTED YET";
  exit;
} else {
	dl('php_gtk.so');
	dl('curl.so');
} 
 

$loadurl = "http://gtk.php.net/";
   
  
function test_save() {
 global $gtkhtml;
  //$gtkhtml->save("save_callback");
} 

function save_callback($string) {
  echo $string;
}  


function my_url_request ($url,$stream) {
   global $gtkhtml,$host;
   $fn = $url;
   
   if (substr($fn,0,4) == "http") {
        $host = ereg_replace("http://([^/]+)/.*","\\1",$url);       
   	$path = ereg_replace("http://[^/]+(/.*)","\\1",$url);
   } else { 
        $path=$url;
	$url = "http://" . $host . $url;
   }
   $fs = 128;
   
 // if you do this, this call will become reentrant - hence be carefull with
 // the global variables  
  // while (gtk::events_pending())  gtk::main_iteration();


  if (function_exists("curl_init")) {
   // The easy way 
   echo "GET $url";
   $ch =  curl_init($url);
   curl_setopt ($ch, CURLOPT_RETURNTRANSFER, 1);
   $string  = curl_exec($ch); 
   $s = curl_getinfo($ch,CURLINFO_SIZE_DOWNLOAD);
   curl_close($ch);
   if ($string) $gtkhtml->write($stream, $string,$s);
   $gtkhtml->end($stream,GTK_HTML_STREAM_OK);
   return;
  } 
 
   
   // the slightly more portable way - but images bugger up
   // this is a bit flakey but should work for the example
   // it just downloads files...
   echo "GET $url";
   $fh =fopen($url,"r");
 
   $b=0;
   $found_start=0;
  if ($fh) while (!feof($fh)) {
      $d = fgets($fh,1024);
      if ($d) $gtkhtml->write($stream, $d,strlen($d));
      while (gtk::events_pending())  gtk::main_iteration();
  
   }
   fclose ($fh);  
     
   while (gtk::events_pending())  gtk::main_iteration();
   $gtkhtml->end($stream,GTK_HTML_STREAM_OK);
   
}
function loadit() {
   global $gtkhtml,$base,$loadurl;
   //while (gtk::events_pending())  gtk::main_iteration();
 
   $base = $loadurl;
   $html_sh = $gtkhtml->begin();
   my_url_request ($base,$html_sh);

}


/*
 * Create a window, and connect realize to the loadit function
 */
$window = &new GtkWindow();
$window->connect_after('show', 'loadit');
$window->set_border_width(10);
$window->set_usize(500,300);
$vbox = &new GtkVbox();

/*
 * You must add the html widget to a scrolled window
 */
 
$sw = &new gtkscrolledwindow(); 
$gtkhtml = &new gtkhtml();
echo "about to load!";


/* 
 * the core image handler callback - connect_url_request
 */
$gtkhtml->connect_url_request('my_url_request');
/* 
 * Some of the other callbacks available
 */

//$gtkhbml->connect('title-changed', "myevent", 'titlechanged');
//$gtkhtml->connect('load-done', "myevent",'load-done');
$gtkhtml->connect('on-url', "test_save");
//$gtkhtml->connect('set-base', "myevent",'on_set_base');
$gtkhtml->connect('redirect', myevent,'on_redirect');
//$gtkhtml->connect('object-requested', "myevent",'on_redirect');

 
echo serialize($gtkhtml);
$sw->add($gtkhtml);
$vbox->add($sw);
$vbox->show();
$window->add($vbox);
$gtkhtml->show();
$sw->show();
/* 
/*
 * Show the window and all its child widgets.
 */
$window->show();

/* Run the main loop. */
Gtk::main();

?>
