# Microsoft Developer Studio Project File - Name="Scheme" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Scheme - Win32 Release_TS
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scheme.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scheme.mak" CFG="Scheme - Win32 Release_TS"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Scheme - Win32 Release_TS" (based on "Win32 (x86) Static Library")
!MESSAGE "Scheme - Win32 Debug_TS" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Scheme - Win32 Release_TS"

# PROP BASE Use_MFC 0
# PROP BASE Output_Dir "Release_TS"
# PROP BASE Intermediate_Dir "Release_TS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Output_Dir "Release_TS"
# PROP Intermediate_Dir "Release_TS"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDebug_TS" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /D "NDebug_TS" /D ZEND_DEBUG=0 /D _WIN32_WINNT=0x400 /D "_LIB" /D "TSRM_EXPORTS" /D "LIBZEND_EXPORTS" /D "ZTS" /D "ZEND_WIN32" /D "WIN32" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40d /d "NDebug_TS"
# ADD RSC /l 0x40d /d "NDebug_TS"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Scheme - Win32 Debug_TS"

# PROP BASE Use_MFC 0
# PROP BASE Output_Dir "Debug_TS"
# PROP BASE Intermediate_Dir "Debug_TS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Output_Dir "Debug_TS"
# PROP Intermediate_Dir "Debug_TS"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_Debug_TS" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /D "_Debug_TS" /D ZEND_DEBUG=1 /D "_LIB" /D "TSRM_EXPORTS" /D "LIBZEND_EXPORTS" /D "ZTS" /D "ZEND_WIN32" /D "WIN32" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x40d /d "_Debug_TS"
# ADD RSC /l 0x40d /d "_Debug_TS"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Scheme - Win32 Release_TS"
# Name "Scheme - Win32 Debug_TS"
# Begin Group "Parsers"

# PROP Default_Filter "php"
# Begin Source File

SOURCE="..\ext\gtk+\gdk.defs"

!IF  "$(CFG)" == "Scheme - Win32 Release_TS"

USERDEP__GDK_D="..\ext\gtk+\gdk.overrides"	"..\ext\gtk+\gdk.defs"	
# Begin Custom Build
InputDir=..\ext\gtk+
InputPath="..\ext\gtk+\gdk.defs"

"$(InputDir)\gen_gdk.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\generator\generator.php -o ext\gtk%%2b\gdk.overrides -p gdk ext\gtk%%2b\gdk.defs >..\ext\gtk+\gen_gdk.c

# End Custom Build

!ELSEIF  "$(CFG)" == "Scheme - Win32 Debug_TS"

USERDEP__GDK_D="..\ext\gtk+\gdk.overrides"	"..\ext\gtk+\gdk.defs"	
# Begin Custom Build
InputDir=..\ext\gtk+
InputPath="..\ext\gtk+\gdk.defs"

"$(InputDir)\gen_gdk.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\generator\generator.php -o ext\gtk%%2b\gdk.overrides -p gdk ext\gtk%%2b\gdk.defs >..\ext\gtk+\gen_gdk.c

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\generator\generator.php

!IF  "$(CFG)" == "Scheme - Win32 Release_TS"

# PROP Ignore_Default_Tool 1
USERDEP__GENER="..\ext\gtk+\gen_gtk.c"	"..\ext\gtk+\gen_gdk.c"	
# Begin Custom Build
InputDir=..\generator
InputPath=..\generator\generator.php

BuildCmds= \
	grep -h "^PHP_GTK_EXPORT_CE" ..\ext\gtk+\gen_gtk.c ..\ext\gtk+\gen_gdk.c | sed -e "s/^/extern /" > ..\ext\gtk+\gen_ce_gtk.h \
	grep -h "^PHP_GTK_EXPORT_CE" ..\ext\gtk+\gen_gtk.c ..\ext\gtk+\gen_gdk.c | sed -e "s/^/extern /" > ..\ext\gtk+\gen_gtk.h \
	grep -h "PHP_GTK_EXPORT_FUNC" ..\ext\gtk+\gen_gtk.c ..\ext\gtk+\gen_gdk.c | sed -e "s/*obj)/&;/" >> ..\ext\gtk+\gen_gtk.h \
	

"$(InputDir)\gen_ce_gtk.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\gen_gtk.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Scheme - Win32 Debug_TS"

USERDEP__GENER="..\ext\gtk+\gen_gtk.c"	"..\ext\gtk+\gen_gdk.c"	
# Begin Custom Build
InputDir=..\generator
InputPath=..\generator\generator.php

BuildCmds= \
	grep -h "^PHP_GTK_EXPORT_CE" ..\ext\gtk+\gen_gtk.c ..\ext\gtk+\gen_gdk.c | sed -e "s/^/extern /" > ..\ext\gtk+\gen_ce_gtk.h \
	grep -h "^PHP_GTK_EXPORT_CE" ..\ext\gtk+\gen_gtk.c ..\ext\gtk+\gen_gdk.c | sed -e "s/^/extern /" > ..\ext\gtk+\gen_gtk.h \
	grep -h "PHP_GTK_EXPORT_FUNC" ..\ext\gtk+\gen_gtk.c ..\ext\gtk+\gen_gdk.c | sed -e "s/*obj)/&;/" >> ..\ext\gtk+\gen_gtk.h \
	

"$(InputDir)\gen_ce_gtk.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\gen_gtk.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\ext\gtk+\gtk.defs"

!IF  "$(CFG)" == "Scheme - Win32 Release_TS"

USERDEP__GTK_D="..\ext\gtk+\gtk.overrides"	"..\ext\gtk+\gtk.defs"	
# Begin Custom Build
InputDir=..\ext\gtk+
InputPath="..\ext\gtk+\gtk.defs"

"$(InputDir)\gen_gtk.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\generator\generator.php -o ext\gtk%%2b\gtk.overrides -p gtk ext\gtk%%2b\gtk.defs >..\ext\gtk+\gen_gtk.c

# End Custom Build

!ELSEIF  "$(CFG)" == "Scheme - Win32 Debug_TS"

USERDEP__GTK_D="..\ext\gtk+\gtk.overrides"	"..\ext\gtk+\gtk.defs"	
# Begin Custom Build
InputDir=..\ext\gtk+
InputPath="..\ext\gtk+\gtk.defs"

"$(InputDir)\gen_gtk.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\generator\generator.php -o ext\gtk%%2b\gtk.overrides -p gtk ext\gtk%%2b\gtk.defs >..\ext\gtk+\gen_gtk.c

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ext\libglade\libglade.defs

!IF  "$(CFG)" == "Scheme - Win32 Release_TS"

USERDEP__LIBGL="..\ext\libglade\libglade.overrides"	"..\ext\libglade\libglade.defs"	
# Begin Custom Build
InputDir=..\ext\libglade
InputPath=..\ext\libglade\libglade.defs

"$(InputDir)\gen_libglade.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\generator\generator.php -o ext\libglade\libglade.overrides -p libglade ext\libglade\libglade.defs -r ext\gtk%%2b\gtk.defs >..\ext\libglade\gen_libglade.c

# End Custom Build

!ELSEIF  "$(CFG)" == "Scheme - Win32 Debug_TS"

USERDEP__LIBGL="..\ext\libglade\libglade.overrides"	"..\ext\libglade\libglade.defs"	
# Begin Custom Build
InputDir=..\ext\libglade
InputPath=..\ext\libglade\libglade.defs

"$(InputDir)\gen_libglade.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	php.exe -q ..\generator\generator.php -o ext\libglade\libglade.overrides -p libglade ext\libglade\libglade.defs r ext\gtk%%2b\gtk.defs >..\ext\libglade\gen_libglade.c

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
