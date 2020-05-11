#
# Fichero MAKE para CAPGRF
#
.autodepend

CC = bcc
CINCLUDE = c:\borlandc\include;..\..\graflib;..
CFLAGS = -ml -N -G -O -v -d -I$(CINCLUDE)
ASM = tasm
AFLAGS = /mx /zi /o

.c.obj:
	$(CC) -c $(CFLAGS) $<

.asm.obj:
	$(ASM) $(AFLAGS) $<


all: capgrf.exe

#
# CAPGRF.EXE
#
CAPGRF = capgrf.obj ..\ventangl.lib ..\..\graflib\grafl.lib
capgrf.exe : $(CAPGRF)
	$(CC) -ecapgrf.exe @&&|
$(CFLAGS)
$(CAPGRF)
|

#
# OBJS
#

capgrf.obj	: capgrf.c
