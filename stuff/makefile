!if defined(NODEBUG)
!if $(NODEBUG)==0
!undef NODEBUG
!endif
!endif

!include <win32.mak>

cc = icl /WX /Qlong_double
fc = ifort
# cc = cl

!ifdef STATIC
!ifdef NODEBUG
CFG= _static
!else
CFG= _debug_static
!endif
!else
!ifdef NODEBUG
CFG= 
!else
CFG= _debug
!endif
!endif

!if 0 # use default output directories
!ifndef OBJFOLD
OBJFOLD= obj$(CFG)
!endif
!ifndef OUTFOLD
OUTFOLD= .
!endif
!else # use output directories for Visual Studio
!ifdef NODEBUG
!ifndef OBJFOLD
OBJFOLD= ..\Release
!endif
!ifndef OUTFOLD
OUTFOLD= ..\Release
!endif
!else
!ifndef OBJFOLD
OBJFOLD= ..\Debug
!endif
!ifndef OUTFOLD
OUTFOLD= ..\Debug
!endif
!endif
!endif

!ifdef STATIC
LFLAGS= $(ldebug) $(conlflags) $(conlibsmt)
!ifdef NODEBUG
FFLAGS= /nologo /c /MT /Ox
CFLAGS= $(cflags) $(cdebug) $(cvarsmt)
!else
FFLAGS= /nologo /c /MTd /Zi /Fd$(OBJFOLD)\ 
CFLAGS= $(cflags) $(cdebug) $(cvarsmt) /Fd$(OBJFOLD)\stuff$(CFG).pdb
!endif
!else
LFLAGS= $(ldebug) $(conlflags) $(conlibsdll)
!ifdef NODEBUG
FFLAGS= /nologo /c /MD /Ox
CFLAGS= $(cflags) $(cdebug) $(cvarsdll)
!else
FFLAGS= /nologo /c /MDd /Zi /Fd$(OBJFOLD)\stuff$(CFG).pdb
CFLAGS= $(cflags) $(cdebug) $(cvarsdll) /Fd$(OBJFOLD)\stuff$(CFG).pdb
!endif
!endif

.cpp{$(OBJFOLD)}.obj:
  $(cc) $(CFLAGS) /GX /Fo$@ $<
{ps_demo}.cpp{$(OBJFOLD)\ps_demo}.obj:
  $(cc) $(CFLAGS) /GX /Fo$@ /I. $<
{ps_demo}.for{$(OBJFOLD)\ps_demo}.obj:
  $(fc) $(FFLAGS) /Fo$@ $<

goal: $(OBJFOLD) $(OUTFOLD)\ps_font.dll $(OUTFOLD)\stuff$(CFG).lib

check: goal $(OBJFOLD)\ps_demo $(OUTFOLD)\ps_demo.exe

$(OUTFOLD)\ps_demo.exe: $(OBJFOLD)\ps_demo\demo.obj $(OBJFOLD)\ps_demo\frame.obj \
$(OBJFOLD)\ps_demo\voro.obj $(OBJFOLD)\ps_demo\clip.obj $(OBJFOLD)\ps_demo\inside.obj \
$(OBJFOLD)\ps_demo\hull.obj $(OBJFOLD)\ps_demo\simple.obj $(OBJFOLD)\ps_demo\conrec.obj \
$(OBJFOLD)\ps_demo\02_random.obj $(OBJFOLD)\ps_demo\trilin.obj $(OUTFOLD)\stuff$(CFG).lib
  link $(LFLAGS) /out:$@ $** getopt$(CFG).lib

$(OUTFOLD)\ps_font.dll: $(OBJFOLD)\ps_font.res
  link /dll /nologo /noentry /machine:x86 /out:$@ $**

$(OBJFOLD)\ps_font.res: ps_font.rc
  rc /fo$@ ps_font.rc

ps_font.rc: $(OBJFOLD)\ps_font.exe
  $(OBJFOLD)\ps_font.exe

$(OBJFOLD)\ps_font.exe: $(OBJFOLD)\ps_font.obj
  link $(LFLAGS) /out:$@ $**

$(OUTFOLD)\stuff$(CFG).lib: $(OBJFOLD)\ps_blin.obj $(OBJFOLD)\ps_clip.obj $(OBJFOLD)\ps_conrec.obj \
$(OBJFOLD)\ps_grid.obj $(OBJFOLD)\ps_image.obj $(OBJFOLD)\ps_label.obj $(OBJFOLD)\ps_proc.obj \
$(OBJFOLD)\ps_stream.obj $(OBJFOLD)\ps_text.obj $(OBJFOLD)\ps_voro.obj $(OBJFOLD)\ps_cross.obj \
$(OBJFOLD)\ps_bbox.obj $(OBJFOLD)\surface_pick.obj
  lib /nologo /out:$@ $**

$(OBJFOLD):
  if not exist $(OBJFOLD) mkdir $(OBJFOLD)
$(OBJFOLD)\ps_demo: $(OBJFOLD)
  if not exist $(OBJFOLD)\ps_demo mkdir $(OBJFOLD)\ps_demo

!if "$(OBJFOLD)"!="$(OUTFOLD)"
$(OUTFOLD):
  if not exist $(OUTFOLD) mkdir $(OUTFOLD)
!endif

clean:
  if exist $(OBJFOLD) rmdir /Q /S $(OBJFOLD)
  if exist $(OBJFOLD)\ps_font.exe erase $(OBJFOLD)\ps_font.exe
  if exist ps_font.rc erase ps_font.rc
  if exist $(OUTFOLD)\ps_font.dll erase $(OUTFOLD)\ps_font.dll
  if exist $(OUTFOLD)\stuff$(CFG).lib erase $(OUTFOLD)\stuff$(CFG).lib
  if exist *.pdb erase /Q *.pdb

ps_blin.cpp: ps_data.h
ps_clip.cpp: ps_data.h
ps_conrec.cpp: ps_data.h
ps_grid.cpp: ps_data.h
ps_image.cpp: ps_data.h
ps_label.cpp: ps_data.h
ps_proc.cpp: ps_data.h
ps_stream.cpp: ps_data.h
ps_text.cpp: ps_data.h
ps_voro.cpp: ps_data.h
ps_cross.cpp: ps_data.h

$(OBJFOLD)\ps_demo\demo.obj: ps_demo\demo.cpp
$(OBJFOLD)\ps_demo\frame.obj: ps_demo\frame.cpp
$(OBJFOLD)\ps_demo\voro.obj: ps_demo\voro.cpp
$(OBJFOLD)\ps_demo\clip.obj: ps_demo\clip.cpp
$(OBJFOLD)\ps_demo\inside.obj: ps_demo\inside.cpp
$(OBJFOLD)\ps_demo\hull.obj: ps_demo\hull.cpp
$(OBJFOLD)\ps_demo\simple.obj: ps_demo\simple.cpp
