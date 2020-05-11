@echo off

if '%1'=='' goto nopar

if '%1'=='t' echo Creando librer¡a de ventanas TINY
if '%1'=='T' echo Creando librer¡a de ventanas TINY
if '%1'=='s' echo Creando librer¡a de ventanas SMALL
if '%1'=='S' echo Creando librer¡a de ventanas SMALL
if '%1'=='c' echo Creando librer¡a de ventanas COMPACT
if '%1'=='C' echo Creando librer¡a de ventanas COMPACT
if '%1'=='m' echo Creando librer¡a de ventanas MEDIUM
if '%1'=='M' echo Creando librer¡a de ventanas MEDIUM
if '%1'=='l' echo Creando librer¡a de ventanas LARGE
if '%1'=='L' echo Creando librer¡a de ventanas LARGE
if '%1'=='h' echo Creando librer¡a de ventanas HUGE
if '%1'=='H' echo Creando librer¡a de ventanas HUGE

bcc -c -d -N -m%1 rutvga.c raton.c ventana.c menu.c cuadro.c ayuda.c editor.c

if exist ventana%1.lib del ventana%1.lib > NUL

tlib ventana%1.lib +rutvga.obj +raton.obj +ventana.obj +menu.obj +cuadro.obj +ayuda.obj +editor.obj

del rutvga.obj > NUL
del raton.obj > NUL
del ventana.obj > NUL
del menu.obj > NUL
del cuadro.obj > NUL
del ayuda.obj > NUL
del editor.obj > NUL

goto fin

:nopar
echo.
echo Uso: VENTANAS x
echo    x = modelo de memoria a usar
echo        T = Tiny    C = Compact
echo        S = Small   L = Large
echo        M = Medium  H = Huge
echo.

:fin
