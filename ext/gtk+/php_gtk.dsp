# Microsoft Developer Studio Project File - Name="php_gtk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=php_gtk - Win32 Debug_TS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "php_gtk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "php_gtk.mak" CFG="php_gtk - Win32 Debug_TS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "php_gtk - Win32 Release_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "php_gtk - Win32 Debug_TS" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "php_gtk - Win32 Release_TS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_TS"
# PROP BASE Intermediate_Dir "Release_TS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_TS"
# PROP Intermediate_Dir "Release_TS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_GTK_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\.." /I "..\..\main" /I "..\..\..\php5" /I "..\..\..\php5\main" /I "..\..\..\php5\Zend" /I "..\..\..\php5\TSRM" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_GTK_EXPORTS" /D "PHP_WIN32" /D "ZEND_WIN32" /D ZTS=1 /D ZEND_DEBUG=0 /D HAVE_PHP_GTK=1 /D "COMPILE_DL_PHP_GTK" /D "TSRM_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 php4ts.lib gtk.lib gdk.lib glib-2.0.lib intl.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../win32/Release_TS/php_gtk.dll" /libpath:"..\..\..\php5\Release_TS"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "php_gtk - Win32 Debug_TS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_TS"
# PROP BASE Intermediate_Dir "Debug_TS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_TS"
# PROP Intermediate_Dir "Debug_TS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_GTK_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\.." /I "..\..\main" /I "..\..\..\php5" /I "..\..\..\php5\main" /I "..\..\..\php5\Zend" /I "..\..\..\php5\TSRM" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_GTK_EXPORTS" /D "PHP_WIN32" /D "ZEND_WIN32" /D ZTS=1 /D ZEND_DEBUG=1 /D HAVE_PHP_GTK=1 /D "COMPILE_DL_PHP_GTK" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 php4ts_debug.lib gtk.lib gdk.lib glib-2.0.lib intl.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../win32/Debug_TS/php_gtk.dll" /pdbtype:sept /libpath:"..\..\..\php5\Debug_TS"

!ENDIF 

# Begin Target

# Name "php_gtk - Win32 Release_TS"
# Name "php_gtk - Win32 Debug_TS"
# Begin Group "Parsers"

# PROP Default_Filter "php"
# Begin Source File

SOURCE=.\gdk.defs

!IF  "$(CFG)" == "php_gtk - Win32 Release_TS"

USERDEP__GDK_D="gdk.overrides"	"gdk.defs"	
# Begin Custom Build
InputPath=.\gdk.defs

"gen_gdk.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\..\generator\generator.php -o ext\gtk%%2b\gdk.overrides -p gdk ext\gtk%%2b\gdk.defs >gen_gdk.c

# End Custom Build

!ELSEIF  "$(CFG)" == "php_gtk - Win32 Debug_TS"

USERDEP__GDK_D="gdk.overrides"	"gdk.defs"	
# Begin Custom Build
InputPath=.\gdk.defs

"gen_gdk.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\..\generator\generator.php -o ext\gtk%%2b\gdk.overrides -p gdk ext\gtk%%2b\gdk.defs >gen_gdk.c

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\generator\generator.php

!IF  "$(CFG)" == "php_gtk - Win32 Release_TS"

USERDEP__GENER="gen_gtk.c"	"gen_gdk.c"	
# Begin Custom Build
InputPath=..\..\generator\generator.php

BuildCmds= \
	grep -h "^PHP_GTK_EXPORT_CE" gen_gtk.c gen_gdk.c | sed -e "s/^/PHP_GTK_API extern /" > gen_ce_gtk.h \
	grep -h "^PHP_GTK_EXPORT_CE" gen_gtk.c gen_gdk.c | sed -e "s/^/PHP_GTK_API extern /" > gen_gtk.h \
	grep -h "PHP_GTK_EXPORT_FUNC" gen_gtk.c gen_gdk.c | sed -e "s/*obj)/&;/" >> gen_gtk.h \
	

"gen_ce_gtk.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"gen_gtk.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "php_gtk - Win32 Debug_TS"

USERDEP__GENER="gen_gtk.c"	"gen_gdk.c"	
# Begin Custom Build
InputPath=..\..\generator\generator.php

BuildCmds= \
	grep -h "^PHP_GTK_EXPORT_CE" gen_gtk.c gen_gdk.c | sed -e "s/^/PHP_GTK_API extern /" > gen_ce_gtk.h \
	grep -h "^PHP_GTK_EXPORT_CE" gen_gtk.c gen_gdk.c | sed -e "s/^/PHP_GTK_API extern /" > gen_gtk.h \
	grep -h "PHP_GTK_EXPORT_FUNC" gen_gtk.c gen_gdk.c | sed -e "s/*obj)/&;/" >> gen_gtk.h \
	

"gen_ce_gtk.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"gen_gtk.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\gtk.defs"

!IF  "$(CFG)" == "php_gtk - Win32 Release_TS"

USERDEP__GTK_D="gtk.overrides"	"gtk.defs"	
# Begin Custom Build
InputPath=".\gtk.defs"

"gen_gtk.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\..\generator\generator.php -o ext\gtk%%2b\gtk.overrides -p gtk -r ext\gtk%%2b\gdk.defs ext\gtk%%2b\gtk.defs >gen_gtk.c

# End Custom Build

!ELSEIF  "$(CFG)" == "php_gtk - Win32 Debug_TS"

USERDEP__GTK_D="gtk.overrides"	"gtk.defs"	
# Begin Custom Build
InputPath=".\gtk.defs"

"gen_gtk.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\..\generator\generator.php -o ext\gtk%%2b\gtk.overrides -p gtk ext\gtk%%2b\gtk.defs >gen_gtk.c

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=php_gdk.c
# End Source File
# Begin Source File

SOURCE=..\..\main\php_gtk.c
# End Source File
# Begin Source File

SOURCE=..\..\main\php_gtk_ext.w32.c
# End Source File
# Begin Source File

SOURCE=..\..\main\php_gtk_object.c
# End Source File
# Begin Source File

SOURCE=..\..\main\php_gtk_util.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\main\php_gtk.h
# End Source File
# Begin Source File

SOURCE=..\..\main\php_gtk_module.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Gtk+"

# PROP Default_Filter ""
# Begin Group "Source Files No. 1"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\gen_gdk.c
# End Source File
# Begin Source File

SOURCE=.\gen_gtk.c
# End Source File
# Begin Source File

SOURCE="php_gtk+.c"
# End Source File
# Begin Source File

SOURCE="php_gtk+_types.c"
# End Source File
# End Group
# Begin Group "Header Files No. 1"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\gen_ce_gtk.h
# End Source File
# Begin Source File

SOURCE=.\gen_gtk.h
# End Source File
# Begin Source File

SOURCE="php_gtk+.h"
# End Source File
# End Group
# Begin Group "Override Files No. 1"

# PROP Default_Filter "*.override"
# Begin Source File

SOURCE=.\gdk.overrides
# End Source File
# Begin Source File

SOURCE=.\gtk.overrides
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\README.txt
# End Source File
# End Target
# End Project
