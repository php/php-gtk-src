Welcome to php-gtk for Win32.

The binary distribution for php-gtk contains this structure

\php4				->	php and php-gtk binary files
\winnt				->	the default php.ini file
\winnt\system32		->	gtk binaries uesd by the extension
\samples			->	a few samples to demonstrate the usage
README.TXT			->	this file.

How to install install:

copy \php4 to your existing php directory. If this is a new instalation you 
should create c:\php4 and copy the files to this directory.

copy \winnt to you winnt directory. On Windows NT and Windows 2000 that is c:\winnt.
On Windows 95/98 that is c:\windows. If you have an existing php.ini you dont need to copy this file.

copy \winnt\system32 to you winnt\system32 directory. On Windows NT and Windows 2000 that is c:\winnt\system32.
On Windows 95/98 that is c:\windows\system32.

copy \samples to the location where you want to run your scripts (c:\php4\samples)

How to use:

Php-gtk applications can be started from the command line (or a shortcut) with this syntax:

	c:\php4\php -q c:\php4\samples\hello.php
