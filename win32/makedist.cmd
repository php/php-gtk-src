@echo off
SET zip_file=php-gtk-0.5.0-win32.zip
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
copy \php4\php_gtk_libglade.dll php4
copy \php4\php_gtk_sqpane.dll php4
copy \php\php4\php.ini-dist winnt\php.ini
type \php\php-gtk\win32\php.ini >>winnt\php.ini
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
copy \php\php-gtk\NEWS
"c:\Program Files\WinZip\WZZip" -ampr %zip_file% 
copy %zip_file% \php\php-gtk-web\distributions
del %zip_file%
cd \php\php-gtk\win32