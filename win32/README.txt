Welcome to PHP-GTK 2 for Windows
================================

The win32 binary distribution for PHP-GTK 2 is a zip file with this structure:

php-gtk				->	PHP-GTK binary files
					->	gtkpath.bat, a batch file to temporarily set the GTK+ runtime path
					->	php.ini file with some appropriate settings
					->	COPYING.LIB (LGPL license)
					->	NEWS
					->	README.txt (this file)
php-gtk\gtk+2.6.9	->	GTK+ 2.6.9 runtime environment
php-gtk\demos		->	a few samples to demonstrate PHP-GTK usage
php-gtk\debug-pack	->	debug headers for PHP-GTK 2 and its extensions

Licensing:
=========

Both PHP-GTK and GTK+ are covered by the LGPL license.

You may obtain the sources for PHP-GTK 2 either by downloading the non-binary
distribution tarball at http://gtk.php.net/download.php or via the php.net CVS
repository following the instructions on the same page.

You may obtain the sources for GTK+ by downloading the source distribution
tarballs from ftp://ftp.gtk.org/pub/gtk/v2.6/. This distribution of PHP-GTK 2
was built against GTK+ 2.6.9.

How to install PHP-GTK 2:
========================

NOTE: THIS RELEASE OF PHP-GTK 2 WILL ONLY WORK WITH PHP 5.1.*

Unzip the archive in a place of your choice. It will create a directory named
php-gtk containing everything in this distribution.

You will need to put a copy of php.exe (the CLI version) and php5ts.dll into the
top level php-gtk directory, alongside the binaries for any other PHP extensions
you intend to use.

Check the php.ini and make sure the settings there are sane for your system. You
may add further PHP-specific settings anywhere between [PHP] and [PHP-GTK].

How to use PHP-GTK 2:
====================

You will need to either set your PATH to include the gtk+2.6.9 directory, or use
the batch file supplied to set both it and the path to php-gtk for the duration
of the current commandline session. To do this, check that the absolute paths
in gtkpath.bat are correct for your setup, open up your commandline, cd to your
php-gtk directory and type 'gtkpath'.

PHP-GTK applications can be started from the command line (or a shortcut) with this syntax:

	php demos\phpgtk2-demo.php
