# Microsoft Developer Studio Project File - Name="hfclib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=hfclib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hfclib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hfclib.mak" CFG="hfclib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hfclib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "hfclib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hfclib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "LibRelease"
# PROP Intermediate_Dir "LibRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "HFC_NOFORCELIBS" /D "HFC_COMPILE" /YX /FD /c
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"LibRelease\hfclib32.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying library files
PostBuild_Cmds=copy LibRelease\HFCLib32.lib D:\Progra~1\HFC\Lib6
# End Special Build Tool

!ELSEIF  "$(CFG)" == "hfclib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "LibDebug"
# PROP Intermediate_Dir "LibDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "HFC_NOFORCELIBS" /D "HFC_COMPILE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"LibDebug\hfclib32d.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying library files
PostBuild_Cmds=copy LibDebug\HFCLib32d.lib D:\Progra~1\HFC\Lib6
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "hfclib - Win32 Release"
# Name "hfclib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Src\AppClasses.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\BaseClasses.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\BaseWindowFunctions.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\COM.CPP
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\CommonControls.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Controls.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Converting.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Crypting.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Debugging.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Dialogs.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Error.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Exceptions.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\File.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\GeneralClasses.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\MainFunctions.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\MathClasses.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\MathFunctions.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Memory.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\ShellExtension.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\StdHFCLibrary.cpp
# ADD CPP /Yc"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\StringClass.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\Strings.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\System.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\VB.CPP
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\WindowClasses.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\WindowControls.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# Begin Source File

SOURCE=..\Src\WindowsFunctions.cpp
# ADD CPP /Yu"HFCLib.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Src\HFCApp.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCArray.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCCmn.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCCom.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCCtrl.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCDebug.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCDef.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCDlgs.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCExcp.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCFunc.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCGen.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCLib.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCList.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCMath.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCMem.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCNmsp.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCRc.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCStr.h
# End Source File
# Begin Source File

SOURCE=..\Src\HFCWin.h
# End Source File
# End Group
# Begin Group "Inline Files"

# PROP Default_Filter "inl"
# Begin Source File

SOURCE=..\Src\CommonControls.inl
# End Source File
# Begin Source File

SOURCE=..\Src\Controls.inl
# End Source File
# Begin Source File

SOURCE=..\Src\File.inl
# End Source File
# Begin Source File

SOURCE=..\Src\GdiObject.inl
# End Source File
# Begin Source File

SOURCE=..\Src\General.inl
# End Source File
# Begin Source File

SOURCE=..\Src\Math.inl
# End Source File
# Begin Source File

SOURCE=..\Src\Memory.inl
# End Source File
# Begin Source File

SOURCE=..\Src\Strings.inl
# End Source File
# Begin Source File

SOURCE=..\Src\Windows.inl
# End Source File
# End Group
# End Target
# End Project
