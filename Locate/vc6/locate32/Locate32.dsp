# Microsoft Developer Studio Project File - Name="Locate32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Locate32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Locate32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Locate32.mak" CFG="Locate32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Locate32 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Locate32 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Locate32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "LANG_EN" /Yu"hfclib.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "LANG_EN"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "Locate32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "LANG_EN" /Yu"hfclib.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "LANG_EN"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /profile /debug /machine:I386

!ENDIF 

# Begin Target

# Name "Locate32 - Win32 Release"
# Name "Locate32 - Win32 Debug"
# Begin Group "Sources"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\..\Locate32\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Locate32\Background.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Locate32\DatabaseInfos.cpp

!IF  "$(CFG)" == "Locate32 - Win32 Release"

!ELSEIF  "$(CFG)" == "Locate32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Locate32\FileObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Locate32\LocateApp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Locate32\LocatedItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Locate32\LocateDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Locate32\ResultsDialogs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Locate32\SettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Locate32\Std.cpp
# ADD CPP /Yc"hfclib.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\Background.h
# End Source File
# Begin Source File

SOURCE=.\Data.h
# End Source File
# Begin Source File

SOURCE=.\FileObject.h
# End Source File
# Begin Source File

SOURCE=.\Locate32.h
# End Source File
# Begin Source File

SOURCE=.\LocateApp.h
# End Source File
# Begin Source File

SOURCE=.\LocatedItem.h
# End Source File
# Begin Source File

SOURCE=.\LocateDlg.h
# End Source File
# Begin Source File

SOURCE=.\ResultsDialogs.h
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc bmp ico wav"
# Begin Source File

SOURCE=..\..\Locate32\commonres.rc
# End Source File
# End Group
# Begin Group "Documents"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\HFCDebug.log
# End Source File
# Begin Source File

SOURCE=.\Korjaukset.txt
# End Source File
# End Group
# Begin Source File

SOURCE=.\commonres\helptext.txt
# End Source File
# Begin Source File

SOURCE=.\commonres\toolbarb.bmp
# End Source File
# End Target
# End Project
