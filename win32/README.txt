Welcome to php-gtk for Win32.

The binary distribution for php-gtk contains this structure

\php4				->	php and php-gtk binary files
\winnt				->	the default php.ini file
\test				->	a few samples to demonstrate the usage
README.TXT			->	this file.

How to install install:

copy \php4 to your existing php directory. If this is a new instalation you 
should create c:\php4 and copy the files to this directory.

copy \winnt to you winnt directory. On Windows NT and Windows 2000 that is c:\winnt.
On Windows 95/98/XP that is c:\windows. If you have an existing php.ini you dont need to copy this file, but make sure
you are adding the php-gtk specific options found in \php4\php.ini-gtk.

copy \test to the location where you want to run your scripts (c:\php4\test)

How to use:

Php-gtk applications can be started from the command line (or a shortcut) with this syntax:

	c:\php4\php c:\php4\test\hello.php

or to avoid the dos-box

	c:\php4\php_win c:\php4\test\gtk.php
