<?php

dl('php_gtk.so');
 

$window = &new GtkWindow();
$window->set_border_width(5);
$window->set_title('GdkCanvas example');
$window->set_policy(false, false, true);
$window->connect_object('destroy', array('gtk', 'main_quit'));




$canvas= &new GtkCanvasAA();
$canvas->set_usize(600,800);

$root = $canvas->root();

print_r($root);

$rect = &new GtkCanvasRect($root, array(   "x1"=> 90.0,
                                     "y1"=> 40.0,
                                     "x2"=> 180.0,
                                     "y2"=> 100.0,
                                     "fill_color_rgba" => 0x3cb37180,
                                     "outline_color"=> "black",
                                     "width_units"=> 4.0)); 
                                     
//print_r(get_class_methods(get_class($rect)));    
$rect->connect('event','canvas_event');
                                 
                                     
$ellipse = & new GtkCanvasEllipse($root,array(
                                     "x1"=> 210.0,
                                     "y1"=> 80.0,
                                     "x2"=> 280.0,
                                     "y2"=> 140.0,
                                     "fill_color_rgba" =>0xff0000c0,
                                     "outline_color"=> "black",
                                     "width_pixels"=> 0));

$ellipse->connect('event','canvas_event');                             
                                     
$text = & new GtkCanvasText($root,array(
                                     "x"=> 10.0,  
                                     "y"=> 10.0,
                                     "text" => "hello world",
                                     "fill_color" => 'steelblue',
                                    "font_gdk" => gdk::font_load    ('-*-helvetica-*'),
                                    "anchor" => 'center',
                                    ));
$text->connect('event','canvas_event');
$button = &new GtkButton('hello world');
$button->show();
$text = & new GtkCanvasWidget($root,array(
                                     "widget"=> $button,
                                     "x" => 80.0,
                                     "y"=> 100.0  ));
                                     

 /* PLYGONS ARE BUSTED - need to sort out the points stuff.. */
$polygon = & new GtkCanvasPolygon($root,


                        array(
                         "fill_color" =>"tan",
                         "outline_color" => "black",
                         "width_units" => 3.0  ));
 
 
 //print_r(get_class_methods(get_class($polygon)));    
/*                    
$polygon->set_arg( "fill_color" , "tan");
$polygon->set_arg( "outline_color" , "black");
$polygon->set_arg( "width_units", 3.0   );
  */                  
$polygon->set_points( array(
                      
                              170.0,  330.0,
                              170.0,  430.0,
                              390.0,  430.0,
                              390.0,  330.0,
                              310.0,  330.0,
                              310.0,  390.0,
                              350.0,  390.0,
                              350.0,  370.0,
                              330.0,  370.0,
                              330.0,  350.0,
                              370.0,  350.0,
                              370.0,  410.0,
                              290.0,  410.0,
                              290.0,  330.0
                        )  
                );
$polygon->connect('event','canvas_event');                
               


$window->add($canvas);

$window->show_all();
$dragging = false;

$canvas->set_pixels_per_unit(1);
print_r($canvas->w2c(100,200));
print_r($canvas->c2w(100,200));
print_r($canvas->window_to_world(100,200));
print_r($canvas->world_to_window(100,200));

gtk::main();



 


function canvas_event($canvasItem,$event) 
{
    
    //print_r($event);
    
    global $dragging,$px,$py;
    
    
    $bx = $event->x;
    $by = $event->y;
    switch($event->type) {
  
       
      case GDK_BUTTON_PRESS:
          
          $px = $bx;
          $py = $by;
          $cursor = gdk::cursor_new(52);
          $canvasItem->grab(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK, $cursor, $event->time);
          $dragging = true;
        /* Handle mouse button press */
        return TRUE;
      case GDK_BUTTON_RELEASE:
        /* Handle mouse button release */
           if (!$dragging) {
              return FALSE;
            }
            $canvasItem->ungrab($event->time);
           $dragging = false;
        
        return TRUE;
      case GDK_MOTION_NOTIFY:
        /* Handle mouse movement */
        if (!$dragging) {
            return FALSE;
        }
        $canvasItem->move($bx-$px,$by-$py);
        $px = $bx;
        $py = $by;
        
        
        return TRUE;
      }
    
      /* Event not handled; try parent item */
      return FALSE;
  
    
}


 




?>
