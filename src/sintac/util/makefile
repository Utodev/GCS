#
# Fichero MAKE para UTILIDADES SINTAC
#
.autodepend

CC = bcc
CINCLUDE = c:\borlandc\include;..;..\..\graflib;..\..\ventanas;..\..\ventanag
CFLAGS = -ml -N -G -O -v -d -I$(CINCLUDE)
ASM = tasm
AFLAGS = /mx /zi /o

.c.obj:
	$(CC) -c $(CFLAGS) $<

.asm.obj:
	$(ASM) $(AFLAGS) $<


all: gcs.exe grfs.exe gmuss.exe

#
# GCS.EXE
#
GCS = gcs.obj ..\..\graflib\grafl.lib ..\..\ventanag\ventangl.lib
gcs.exe : $(GCS)
	$(CC) -egcs.exe @&&|
$(CFLAGS)
$(GCS)
|

#
# GRFS.EXE
#
GRFS = grfs.obj grfpcx.obj ..\..\graflib\grafl.lib ..\..\ventanas\ventanal.lib
grfs.exe : $(GRFS)
	$(CC) -egrfs.exe @&&|
$(CFLAGS)
$(GRFS)
|

#
# GMUSS.EXE
#
GMUSS = gmuss.obj sonido.obj ..\..\graflib\grafl.lib ..\..\ventanas\ventanal.lib
gmuss.exe : $(GMUSS)
	$(CC) -egmuss.exe @&&|
$(CFLAGS)
$(GMUSS)
|

#
# OBJS
#
gcs.obj		: gcs.c

grfs.obj	: grfs.c
grfpcx.obj	: ..\grfpcx.c
	$(CC) -c $(CFLAGS) ..\grfpcx.c

gmuss.obj	: gmuss.c
sonido.obj	: ..\sonido.c
	$(CC) -c $(CFLAGS) ..\sonido.c
