Installing php-gtk on Windows NT/Windows 2000

Download the following files from http://www.gtk.org

http://www.gimp.org/win32/glib-dev-20001226.zip
http://www.gimp.org/win32/libiconv-dev-20001007.zip
http://www.gimp.org/win32/gtk+-dev-20001226.zip

Filename might change with new releases.
Links to the latest files are found on 
http://user.sgic.fi/~tml/gimp/win32/downloads.html

These zip files all have a directory stucture starting with \src.
Extract the files so the \src directory is placed on the same
level as php-gtk (and php4).

You need to compile PHP4 as a CGI add the directory where php.exe is stored
to the executables in Microsoft Visual Studio.
You doo thie by:
	Select "Options" in the tools menu
	Select "Directories" tab
	Select "Ececutables" in the drop down list
	Go to the empty line and add the path to php (c:\php4)
	Close the options window.

Compile php-gtk:
Open the workspace php-gtk.dsw
Compile the project. This process will execute a php-script and the output from this is
the file php_gtk_gen.c.
When all files are compiled the extension php_gtk.dll can be found in \php-gtk\win32\Release.
Copy this file to yor extension dir and you are ready to test the applications.

copy all gtk-dll's to \winnt\system32. You can do this by executing 
the script \php-gtk\win32\inst.cmd


On a command line type:
	cd \php-gtk\test
	c:\php4\php -q gtk.php
	
Or create a shortcut:
	C:\php4\php.exe -q c:\php-gtk\test\gtk.php

Happy hacking

Frank Kromann <frank@frontbase.com>