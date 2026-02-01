# Microsoft Developer Studio Project File - Name="Field" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Field - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Field.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Field.mak" CFG="Field - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Field - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Field - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
F90=df.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Field - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE F90 /compile_only /nologo /warn:nofileopt /winapp
# ADD F90 /browser /compile_only /nologo /warn:nofileopt /winapp
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /Op /I "." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 oxliba.lib msimg32.lib hdf5-1.6.1.lib /nologo /subsystem:windows /machine:I386 /libpath:"\prj\rle" /libpath:"\pkg\utb\lib\libs"

!ELSEIF  "$(CFG)" == "Field - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE F90 /check:bounds /compile_only /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt /winapp
# ADD F90 /browser /check:bounds /compile_only /debug:full /nologo /traceback /warn:argument_checking /warn:nofileopt /winapp
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /Op /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_GXDLL" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 oxdllad.lib msimg32.lib hdf5-1.6.1_debug.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /nodefaultlib:"msvcprt.lib" /pdbtype:sept /libpath:"\prj\rle" /libpath:"\pkg\utb\lib\dlls"

!ENDIF 

# Begin Target

# Name "Field - Win32 Release"
# Name "Field - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;f90;for;f;fpp"
# Begin Source File

SOURCE=.\ActField.cpp
# End Source File
# Begin Source File

SOURCE=.\CrossSection.cpp
# End Source File
# Begin Source File

SOURCE=.\CumulativeS.cpp
# End Source File
# Begin Source File

SOURCE=.\DataMapping.cpp
# End Source File
# Begin Source File

SOURCE=.\DigitizeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Field.cpp
# End Source File
# Begin Source File

SOURCE=.\Field.rc
# End Source File
# Begin Source File

SOURCE=.\FieldDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldForm.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldImport.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldView.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldView_Prop.cpp
# End Source File
# Begin Source File

SOURCE=.\FindWell.cpp
# End Source File
# Begin Source File

SOURCE=.\IntegralProp.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\OverView.cpp
# End Source File
# Begin Source File

SOURCE=.\PlotDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ProbeDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\redirect.cpp
# End Source File
# Begin Source File

SOURCE=.\ResDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\CrossSection.h
# End Source File
# Begin Source File

SOURCE=.\CumulativeS.h
# End Source File
# Begin Source File

SOURCE=.\DataMapping.h
# End Source File
# Begin Source File

SOURCE=.\DigitizeDlg.h
# End Source File
# Begin Source File

SOURCE=.\Field.h
# End Source File
# Begin Source File

SOURCE=.\FieldDoc.h
# End Source File
# Begin Source File

SOURCE=.\FieldForm.h
# End Source File
# Begin Source File

SOURCE=.\FieldImport.h
# End Source File
# Begin Source File

SOURCE=.\FieldView.h
# End Source File
# Begin Source File

SOURCE=.\FieldView_PageGrid.h
# End Source File
# Begin Source File

SOURCE=.\FieldView_Prop.h
# End Source File
# Begin Source File

SOURCE=.\FindWell.h
# End Source File
# Begin Source File

SOURCE=.\IntegralProp.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\OverView.h
# End Source File
# Begin Source File

SOURCE=.\PlotDialog.h
# End Source File
# Begin Source File

SOURCE=.\ProbeDialog.h
# End Source File
# Begin Source File

SOURCE=.\redirect.h
# End Source File
# Begin Source File

SOURCE=.\ResDialog.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\elb_add.bmp
# End Source File
# Begin Source File

SOURCE=.\res\elb_del.bmp
# End Source File
# Begin Source File

SOURCE=.\res\elb_down.bmp
# End Source File
# Begin Source File

SOURCE=.\res\elb_up.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Field.ico
# End Source File
# Begin Source File

SOURCE=.\res\Field.rc2
# End Source File
# Begin Source File

SOURCE=.\res\FieldDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\open.ico
# End Source File
# Begin Source File

SOURCE=.\res\save.ico
# End Source File
# Begin Source File

SOURCE=.\res\stateicon.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
