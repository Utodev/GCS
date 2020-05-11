#
# Fichero MAKE para SINTAC
#
.autodepend

CC = bcc
CINCLUDE = c:\util\bc\include;..\graflib;..\ventanas;..\flilib
CFLAGS = -ml -N -G -O -v -d -I$(CINCLUDE)
ASM = tasm
AFLAGS = /mx /zi /o

.c.obj:
	$(CC) -c $(CFLAGS) $<

.asm.obj:
	$(ASM) $(AFLAGS) $<


#all: cs.exe ds.exe lks.exe eds.exe instalar.exe
all: cs.exe ds.exe

#
# CS.EXE
#
CS = cs.obj compil.obj ..\graflib\grafl.lib ..\ventanas\ventanal.lib
cs.exe : $(CS)
	$(CC) -ecs.exe @&&|
$(CFLAGS)
$(CS)
|

#
# DS.EXE
#
DS = ds.obj condacto.obj vv.obj grfpcx.obj sonido.obj ..\graflib\grafl.lib \
  ..\flilib\fli.lib
ds.exe : $(DS)
	$(CC) -c $(CFLAGS) -DDEBUGGER=1 -DRUNTIME=0 ds.c condacto.c
	$(CC) -eds.exe @&&|
$(CFLAGS)
$(DS)
|

#
# LKS.EXE
#
LKS = lks.obj ..\graflib\grafl.lib ..\ventanas\ventanal.lib
lks.exe : $(LKS)
	$(CC) -elks.exe @&&|
$(CFLAGS)
$(LKS)
|

#
# EDS.EXE
#
EDS = sspawn.obj eds.obj ..\graflib\grafl.lib ..\ventanas\ventanal.lib
eds.exe : $(EDS)
	$(CC) -eeds.exe @&&|
$(CFLAGS)
$(EDS)
|

#
# INSTALAR.EXE
#
INSTALAR = instalar.obj ..\graflib\grafl.lib ..\ventanas\ventanal.lib
instalar.exe : $(INSTALAR)
	$(CC) -einstalar.exe @&&|
$(CFLAGS)
$(INSTALAR)
|

#
# RUNTIME.EXE
#
RUNTIME = ds.obj condacto.obj vv.obj grfpcx.obj sonido.obj \
  ..\graflib\grafl.lib ..\flilib\fli.lib
runtime.exe : $(RUNTIME)
	$(CC) -c $(CFLAGS) -DDEBUGGER=0 -DRUNTIME=1 ds.c condacto.c
	$(CC) -eruntime.exe @&&|
$(CFLAGS)
$(RUNTIME)
|

#
# OBJS
#

cs.obj          : cs.c
compil.obj      : compil.c

ds.obj          : ds.c
condacto.obj    : condacto.c
vv.obj          : vv.c
grfpcx.obj      : grfpcx.c
sonido.obj      : sonido.c

lks.obj         : lks.c

sspawn.obj      : sspawn.asm
eds.obj         : eds.c

instalar.obj    : instalar.c
