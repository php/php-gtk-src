@echo off
md \php-gtk-dist
cd \php-gtk-dist
md php4
md winnt
md winnt\system32
md test
copy \php4\php.exe php4
copy \php4\php_win.exe php4
copy \php4\php4ts.dll php4
copy \php4\php4isapi.dll php4
copy \php4\php_gtk.dll php4
copy \php\php4\php.ini-dist winnt\php.ini
copy %SystemRoot%\system32\gtk-1.3.dll winnt\system32
copy %SystemRoot%\system32\gdk-1.3.dll winnt\system32
copy %SystemRoot%\system32\gmodule-1.3.dll winnt\system32
copy %SystemRoot%\system32\glib-1.3.dll winnt\system32
copy %SystemRoot%\system32\iconv-1.3.dll winnt\system32
copy %SystemRoot%\system32\gnu-intl.dll winnt\system32
copy %SystemRoot%\system32\libglade-0.14.dll winnt\system32
copy %SystemRoot%\system32\libxml.dll winnt\system32
copy \php\php-gtk\test\fileselection.php test
copy \php\php-gtk\test\gtk.php test
copy \php\php-gtk\test\hello.php test
copy \php\php-gtk\test\list.php test
copy \php\php-gtk\test\scribble.php test
copy \php\php-gtk\test\testgtkrc test
copy \php\php-gtk\test\testgtkrc2 test
copy \php\php-gtk\win32\README.txt 
"c:\Program Files\WinZip\WZZip" -ampr php-gtk-0.1.1a-win32.zip
cd \php\php-gtk\win32