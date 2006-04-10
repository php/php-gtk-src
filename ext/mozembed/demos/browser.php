<?php

    global $location;
    global $mozComponent;
    $location = new GtkEntry();
    $mozComponent = new GtkMozEmbed();
    
    $goButton = GtkButton::new_from_stock(Gtk::STOCK_OK);
    $stopButton = GtkButton::new_from_stock(Gtk::STOCK_STOP);
    $goForward = GtkButton::new_from_stock(Gtk::STOCK_GO_FORWARD);
    $goBackward = GtkButton::new_from_stock(Gtk::STOCK_GO_BACK);
    
    $goButton->connect_simple("clicked", "goto");
    $stopButton->connect_simple("clicked", "stop");
    $goForward->connect_simple("clicked", "forward");
    $goBackward->connect_simple("clicked", "back");

    /* Home Page! */
    $mozComponent->load_url("http://gtk.php.net/");

    $locationBar = new GtkHBox();
    $locationBar->pack_start($location);
    $locationBar->pack_start($goButton);
    $locationBar->pack_start($stopButton);
    $locationBar->pack_start($goForward);
    $locationBar->pack_start($goBackward);

    $mainBox = new GtkVBox();
    $mainBox->pack_start($locationBar, false, false);
    $mainBox->pack_start($mozComponent, true, true);
    
    $mainWindow = new GtkWindow();
    $mainWindow->maximize();
    $mainWindow->set_title("PHP-Gtk2's first web browser!");
    $mainWindow->add($mainBox);

    $mainWindow->show_all();
    $mainWindow->connect_simple("destroy", array("Gtk", "main_quit"));

    Gtk::main();

    function goto()
    {
        global $location;
        global $mozComponent;
        $url = trim($location->get_text());
        $mozComponent->load_url($url);
    }

    function stop()
    {
        global $mozComponent;
        $mozComponent->stop_load();
    }

    function forward()
    {
        global $mozComponent;
        $mozComponent->go_forward();
    }

    function back()
    {
        global $mozComponent;
        $mozComponent->go_back();
    }

?>
