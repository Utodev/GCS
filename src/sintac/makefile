#
# Fichero MAKE para SINTAC
#
.autodepend

CC = bcc
CINCLUDE = c:\util\borlandc\include;..\graflib;..\ventanas;..\flilib
CFLAGS = -ml -N -G -O -v -d -I$(CINCLUDE)
ASM = tasm
AFLAGS = /mx /zi /o

.c.obj:
	$(CC) -c $(CFLAGS) $<

.asm.obj:
	$(ASM) $(AFLAGS) $<


#all: cs.exe ds.exe lks.exe eds.exe instalar.exe
all:  ds.exe


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



ds.obj          : ds.c
condacto.obj    : condacto.c
vv.obj          : vv.c
grfpcx.obj      : grfpcx.c
sonido.obj      : sonido.c

