# Microsoft Developer Studio Project File - Name="Scheme" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Scheme - Win32 Release_TS_inline
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scheme.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scheme.mak" CFG="Scheme - Win32 Release_TS_inline"
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

SOURCE=..\generator\scheme.php

!IF  "$(CFG)" == "Scheme - Win32 Release_TS"

# Begin Custom Build
InputDir=..\src
InputPath=..\generator\scheme.php

BuildCmds=php.exe -q ..\generator\generator.php ../generator/gtk.defs ../generator/gtk.overrides

"$(InputDir)\php_gtk_gen.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\php_gtk_gen.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

# End Custom Build

!ELSEIF  "$(CFG)" == "Scheme - Win32 Debug_TS"

# Begin Custom Build
InputDir=..\src
InputPath=..\generator\scheme.php

BuildCmds=php.exe -q ..\generator\generator.php ../generator/gtk.defs ../generator/gtk.overrides

"$(InputDir)\php_gtk_gen.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\php_gtk_gen.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
