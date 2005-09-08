The Win32 Build System.

===========================================================
Contents:

1. How to build PHP-GTK 2 under Windows

  a. Requirements
  b. Opening a command prompt
  c. Generating configure.js
  d. Configuring
  e. Building
  f. Cleaning Up

2. How to install and run PHP-GTK 2

  a. Requirements
  b. Installation
  c. Running a PHP-GTK test script

===========================================================
1. How to build PHP-GTK under Windows

a. Requirements

 You need:
  - Everything you already use for building PHP 5 under 'the new build system'
  - A freshly-built copy of PHP 5 CGI or CLI, and the source you built it from

 You also need:
  - The development packages for GTK+ 2.6.9, from http://www.gimp.org/~tml/gimp/win32/downloads.html
  - Copies of grep.exe and sed.exe. If you have cygwin installed, they will probably be in there. If
    you don't have cygwin installed, you can download them easily from http://unxutils.sourceforge.net/
    - put them either in your root directory or in <dev_dir>\bin.
  - The latest version of PHP-GTK from the CVS server

 Unzip the GTK+ development packages and put the libs and headers into your development environment:

    <dev_dir>\include\gdkconfig.h
    <dev_dir>\include\glibconfig.h
    <dev_dir>\include\glib.h
    <dev_dir>\include\glib-object.h
    <dev_dir>\include\gmodule.h

    <dev_dir>\include\atk\<atk headers>
    <dev_dir>\include\gdk\<gdk headers>
    <dev_dir>\include\gdk-pixbuf\<gdkpixbuf headers>
    <dev_dir>\include\glib\<glib headers>
    <dev_dir>\include\gobject\<gobject headers>
    <dev_dir>\include\gtk\<gtk headers>
    <dev_dir>\lib\<all the libraries can go in the toplevel lib dir>

 Check out the php-gtk directory as a sibling directory for <dev_dir>\php5:
    cvs -d :pserver:cvsread@cvs.php.net:/repository login
    Password: phpfi
    cvs -d :pserver:cvsread@cvs.php.net:/repository co php-gtk

b. Opening a command prompt

 Whatever approach you adopted when you built PHP, you will need to repeat in the php-gtk
 directory.

 You will need to add the path to your PHP source (<dev_dir>\php5) to the Include env var
 in your compiler. You will also need to add the path to the php5ts.lib you are building
 against to the Library env var, and the path to a PHP executable binary (any version of
 php.exe will be fine for this) to %PATH%.

c. Generating configure

 Change directory to where you have your PHP-GTK sources, i.e. cd \<dev_dir>\php-gtk
 Run buildconf.bat by typing "buildconf" at the command prompt.

d. Configuring

    cscript /nologo configure.js --help

 Will give you a list of configuration options.

  --with-php-build will tell the configure process (not the compiler) where your headers
  and libraries are stored. You probably won't need to specify this.

  --with-cygwin will tell the configure process where cygwin lives on your system. You might
  need this to enable it to find grep and sed, which are used during the process. The
  default setting is \cygwin.

  --enable-object-out-dir allows you to choose where your php-gtk2.dll should be created.
  By default, the path will be either php-gtk\Release or php-gtk\Debug.

  --enable-debug will build the debug version. Please use PHP CGI if you're using debug.

 There is no 'shared' option because PHP-GTK libraries will always be shared.

 The log files generated during configuration are sent to win32\logs.

e. Building

 Once you have successfully configured your build, you can build the code; simply type
 "nmake" at the command prompt.

f. Cleaning Up

 Cleaning occurs automatically during buildconf.

2. How to install and run PHP-GTK 2

a. Requirements

 You need:
  - The runtime packages for GTK+ 2.6.9, from http://www.gimp.org/~tml/gimp/win32/downloads.html

b. Installation

 At present the Release version of php-gtk2.dll will only run from C:\php5, so you will
 need to put php.exe (either CLI or CGI), php5ts.dll and the .dll files for any other
 PHP extensions you need, into that directory. Also a copy of php.ini.

 Just to be awkward, the Debug version of php-gtk2.dll currently will _only_ run from
 C:\php5\Debug using a PHP CGI executable. There is a PHP debug symbols pack available
 from http://snaps.php.net.

 Retrieve the .dll files from the various GTK+ runtime zips. The 'pango\modules', 'pixbufloader'
 and 'immodules' .dlls can go anywhere you like, but the rest need to be somewhere the
 php5ts.dll can see them - that is, either in your root directory or in C:\php5.

 Copy the 'etc' directory and everything in it (pango\* and gtk-2.0\*) directly to
 C:\Windows\System32. You will need to manually alter the paths in pango.modules,
 gdk-pixbuf.loaders and (possibly) gtk.immodules to reflect the whereabouts of the
 relevant .dll files. PHP-GTK 2 won't work at all without the first two of these.

 Add the following lines to your php.ini:

    extension=php_gtk.dll
    php-gtk.codepage = CP1252

 (unless of course you're using some other codepage...)

c. Running a PHP-GTK test script

 Using the command line, navigate to your PHP-GTK directory and type:

    C:\php5\php stock-browser.php

 Or create a shortcut, starting from php-gtk\demos:

    C:\php5\php.exe -q stock-browser.php


Enjoy!

Steph Fox <sfox@php.net>
