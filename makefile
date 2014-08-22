#
# MKMF template makefile for executables.
#

cc              = ztc
debug		= 1

!if $(debug)
linkopt         += /co /li
cflags          = -g $w $u $a -b 
!else
cflags      = $w $u $a $o -R -b
linkopt     = /pac /f
!endif

dest		= c:\bin\
exthdrs		=
hdrs		= bmg.h date.h editmail.h fido.h maincmds.h menu.h mkfp.h \
		  msged.h nedit.h quick.h screen.h screen2.h vfossil.h
linker		= blink
makefile	= makefile
model           = v
lobjs           = msged.obj screen.obj screen2.obj menu.obj (string.obj editmail.obj) \
		  (areas.obj) (bmg.obj) (config.obj) (dir.obj) (fido.obj) (quick.obj) \
		  (maintmsg.obj) (makemsg.obj) (readmail.obj normal.obj) (date.obj) \
		  (settings.obj) (showmail.obj) (textfile.obj) (userlist.obj)
objs		= areas.obj bmg.obj config.obj date.obj dir.obj editmail.obj \
		  fido.obj maintmsg.obj makemsg.obj menu.obj \
		  msged.obj normal.obj quick.obj readmail.obj \
		  screen.obj screen2.obj settings.obj showmail.obj \
		  string.obj textfile.obj userlist.obj 
program		= msged.exe
srcs		= areas.c bmg.c config.c date.c dir.c editmail.c fido.c \
		  maintmsg.c makemsg.c menu.c msged.c \
		  normal.asm quick.c readmail.c screen.c screen2.asm \
		  settings.c showmail.c string.c textfile.c userlist.c
startup 	= $(swap) $(lib)\int.obj
ulibs           =

vpath           =

!if $(model) != v
lobjs = $(objs)
!endif

all:		$(program)

$(program):	$(objs) $(ulibs) $(makefile)
		$(link)

$(program,B).lzh: $(srcs) $(hdrs) $(makefile)
		!foreach f $?
		    lharc u $(program,B) $f
		!end

archive:	$(program,B).lzh
			exp

clean:;		@rm -f $(objs)

depend:;	@mkmf -f $(makefile) program=$(program) dest=$(dest)

.PRECIOUS:	msged.tag

msged.tag:      $(obj) 
!if $(debug)
		ztag /I $(objs),$(program,B)
!endif

install:	$(program)
		@echo installing $(program) in $(dest)
		@if not $(dest)x==.x copy $(program) $(dest)

!if $(debug)
.asm.obj :
	masm /mx /zi $*;
!endif

### OPUS MKMF:  Do not remove this line!  Automatic dependencies follow.

areas.obj: fido.h msged.h nedit.h quick.h screen.h

bmg.obj: bmg.h

config.obj: fido.h msged.h nedit.h quick.h screen.h

date.obj: date.h fido.h msged.h nedit.h quick.h screen.h

editmail.obj: editmail.h fido.h msged.h nedit.h quick.h screen.h

fido.obj: date.h fido.h msged.h nedit.h quick.h screen.h

maintmsg.obj: date.h fido.h msged.h nedit.h quick.h screen.h

makemsg.obj: date.h fido.h msged.h nedit.h quick.h screen.h

menu.obj: menu.h screen.h

msged.obj: bmg.h fido.h maincmds.h msged.h nedit.h quick.h screen.h

quick.obj: date.h fido.h msged.h nedit.h quick.h screen.h

readmail.obj: bmg.h date.h fido.h msged.h nedit.h quick.h screen.h

screen.obj: mkfp.h screen.h screen2.h vfossil.h

settings.obj: fido.h menu.h msged.h nedit.h quick.h screen.h

showmail.obj: date.h fido.h msged.h nedit.h quick.h screen.h

textfile.obj: date.h fido.h msged.h nedit.h quick.h screen.h

userlist.obj: fido.h msged.h nedit.h quick.h screen.h

string.obj: string.c
