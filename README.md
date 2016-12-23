PHP-GTK: PHP language bindings for GTK+ toolkit
==============================================
PHP-GTK is a PHP extension that enables you to write client-side cross-platform
GUI applications. This is the first such extension of this kind and one of the
goals behind it was to prove that PHP is a capable general-purpose scripting
language that is suited for more than just Web applications.

This extension will _not_ allow you to display GTK+ programs in a Web browser,
and can be used in the Web environment only if you are running the webserver
locally. It is intended for creating standalone GUI applications.


Requirements
============
As a prerequisite, you should have a CLI binary (command-line) version of PHP
installed along with the header files and development scripts. They should
normally be installed when you do 'make install' in the PHP source tree.

Currently, PHP-GTK requires PHP 5.1.2 or greater to run, and to build.

PHP-GTK supports GTK+ v2.6 and requires v2.6 or greater. The latest stable
release of GTK+ can be obtained from ftp://ftp.gnome.org/pub/gnome/sources/gtk+/.


Installation and testing
========================
After unpacking the distribution or checking out the sources from the CVS,
change into the base directory and do the following.

First, run `./buildconf` script that comes with PHP-GTK. It will set up
necessary files and create 'configure' script for you.  Next, run './configure'.
It will check that you have proper version of GTK+ and other files required for
compilation and create the Makefile's.

PHP-GTK supports extensions which can be configured via 'configure' script
switches that you can see by running `./configure --help'`

Finally, run `make` to compile the extension. Some source files are generated
from .defs files by the code generator. If you see messages like "Could not
write...", that just means that certain GTK+ objects or features are not yet
supported. If the compilation is successful, run 'make install' to install the
extension into your default PHP extension directory.

To test the extension, try running the demo scripts in the demos/ directory,
especially phpgtk2-demo.php. These are also good for showing how the extension
can be used.


Website
=======
The website for this extension can be found at:
[PHP-GTK](http://gtk.php.net/)

This is where you should go to obtain the latest releases and news about
PHP-GTK.


Mailing lists
=============
There are several mailing lists for PHP-GTK.

   * General list
     ------------
     This list is for general discussion of PHP-GTK, techniques, problems,
     development of applications, questions, and other issues.

     [List address]( mailto:php-gtk-general@lists.php.net)
     [Subscription](mailto:php-gtk-general-subscribe@lists.php.net)
     [Archive]( http://marc.info/?l=php-gtk-general)

   * Developers list
     ---------------
     This list is for those wanting to help with development of PHP-GTK and its
     extensions.

     [List address](mailto:php-gtk-dev@lists.php.net)
     [Subscription](mailto:php-gtk-dev-subscribe@lists.php.net)
     [Archive](http://marc.info/?l=php-gtk-dev)

   * Documenters list
     ----------------
     This list is for those discussion and development of PHP-GTK documentation.

     [List address](mailto:php-gtk-doc@lists.php.net)
     [Subscription](mailto:php-gtk-doc-subscribe@lists.php.net)
     [Archive]( http://marc.info/?l=php-gtk-doc)

To subscribe to any of the lists, send blank email to the appropriate
subscription address.


Patches and feedback
====================
Please use the mailing lists for most of your questions -- many people read
them and you are more likely to get quicker feedback if you post there first.
Patches in the unified diff (diff -u) format should be sent to the developers
list (see above).


Acknowledgements
================
The person I would like to thank the most is [James Henstridge](http://www.jamesh.id.au/), the author of [PyGTK (Python GTK+ bindings)](www.pygtk.org/
). His help and
advice have proved invaluable during the development of this extension, and a
fair amount of source code is based on or inspired by what he has developed for
PyGTK.

I'd also like to thank PHP team for making such a great language and the GTK+
team for developing what has to be the best C-based GUI toolkit.

