Installing php-gtk on Windows NT/Windows 2000

Building PHP-GTK from scratch requires the PHP4 build environment 
as well as MS Visual Studio.

Get the latest version of PHP-GTK from the cvs server

cvs -d:cvsreas@cvs.php.net:/repository login
Password: phpfi
cvs -d:cvsreas@cvs.php.net:/repository co php-gtk

Download the following development files
ftp://ftp.gnu.org/gnu/gettext/gettext-runtime-0.12.1.bin.woe32.zip
http://www.gimp.org/~tml/gimp/win32/glib-dev-2.2.3.zip
http://www.gimp.org/~tml/gimp/win32/gtk+-dev-1.3.0-20030115.zip
ftp://ftp.gnu.org/gnu/libiconv/libiconv-1.9.1.bin.woe32.zip

Unzip these files to a directory structure like this:

<path to development>\src\gettext-0.12.1
<path to development>\src\glib-2.2.3
<path to development>\src\gtk+1.3.0
<path to development>\src\libiconv-1.9.1

Download these runtime environments
http://www.gimp.org/~tml/gimp/win32/gtk+-1.3.0-20030717.zip
http://www.gimp.org/~tml/gimp/win32/glib-2.2.3.zip

Unzip both viles to 

<path to development>\src\gtk-runtime

Filename might change with new releases.
Links to the latest files are found on 
http://www.gimp.org/~tml/gimp/win32/downloads.html
http://www.zlatkovic.com/projects/libxml/binaries.html

Compile PHP4 as a CLI and/or CGI module and copy at lease these files to c:\php4 (or any directory of your choice).

php.exe
php4ts.dll
php.ini

Add the directory to the list of Executable files in Visual Studio.
You doo thie by:
	Select "Options" in the tools menu
	Select "Directories" tab
	Select "Ececutable files" in the drop down list
	Go to the empty line and add the path to php (c:\php4)
	Close the options window.

include_path in php.ini should contain ".;" in addition to other include paths used

You should also add the folowing include and lib paths to Visual Studio

Inclide files:
<path to development>\src\libiconv-1.9.1\include
<path to development>\src\gtk+1.3.0\include
<path to development>\src\gtk+1.3.0\lib\gtk+\include
<path to development>\src\glib-2.2.3\include\glib-2.0

Library files
<path to development>\src\libiconv-1.9.1\lib
<path to development>\src\gtk+1.3.0\lib
<path to development>\src\glib-2.2.3\lib
<path to development>\src\gettext-0.12.1\lib

Building php-gtk:
Open the workspace php-gtk.dsw
Select "php_gtk" as the active project and compile it.
It is also possible to build "combobutton", "scrollpane", "spaned" and "sqpane"

Building the libglade extension requires download and installation of
http://prdownloads.sourceforge.net/wingtk/libglade-0.17.exe?download

Add these paths to Visual Studio:

Inclide files:
<path to development>\src\libglade\source\libglade-0.17

Library files:
<path to development>\src\libglade

Building the scintilla extension requires download and installation of
http://prdownloads.sourceforge.net/moleskine/GtkScintilla-0.8.2.tar.gz

This file contains source files only.

For updated versions of GTK libraries, check http://wingtk.sourceforge.net/
Updated versions of all libraries can also be found at http://kromann.info/php4-gtk

Installation:
Copy all the dll files from <path to development>\php-gtk\win32\Release_TS to c:\php4 (or the directory where you copied the PHP binaries to).

Copy these files to c:\php4
<path to development>\src\gtk-runtime\bin\libglib-2.0-0.dll
<path to development>\src\gtk-runtime\bin\libgmodule-2.0-0.dll
<path to development>\src\gtk-runtime\bin\libgobject-2.0-0.dll
<path to development>\src\gtk-runtime\bin\libgthread-2.0-0.dll
<path to development>\src\gtk-runtime\lib\libgdk-0.dll
<path to development>\src\gtk-runtime\lib\libgtk-0.dll
<path to development>\src\gettext-0.12.1\bin\intl.dll
<path to development>\src\libiconv-1.9.1\bin\iconv.dll
<path to development>\src\libglade\libglade.dll


You can load this extension from php.ini by adding this line

	extension=php_gtk.dll

It is not recomended to use this method if php is used for both Gtk and Web.
The extenstion can't be loaded by php running under a web service.
The extension can also be loaded with the dl() command.

	dl("php_gtk.dll");

On a command line type:
	cd \php-gtk\test
	c:\php4\php -q gtk.php
	
Or create a shortcut:
	C:\php4\php.exe -q c:\php-gtk\test\gtk.php



Happy hacking

Frank M. Kromann <frank@kromann.info>
