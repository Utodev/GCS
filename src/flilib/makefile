.asm.obj:
	tasm /w2 /mx $*;

.c.obj:
	bcc -ml -c $*.c

LIBO = \
brun.obj \
compfram.obj \
copyscre.obj \
creatfli.obj \
fccomp.obj \
flierr.obj \
flimem.obj \
flinext.obj \
fliuntil.obj \
dosint.obj \
jclose.obj \
jcreate.obj \
jopen.obj \
jreadwri.obj \
jseek.obj \
jtell.obj \
lccomp.obj \
makelong.obj \
makeptr.obj \
memscree.obj \
openfli.obj \
playfli.obj \
playonce.obj \
ptrseg.obj \
pttolong.obj \
uncompfl.obj \
vgavars.obj \
bcompare.obj \
bcontras.obj \
bsame.obj \
clock.obj \
copybyte.obj \
copyword.obj \
csetcolo.obj \
fcompare.obj \
fcuncomp.obj \
normptr.obj \
skip.obj \
unbrun.obj \
unlccomp.obj \
waitvbl.obj \
zerobyte.obj \
zeroword.obj

fli.lib: $(LIBO)
	del fli.lib
	tlib fli.lib @flilib.lbd
