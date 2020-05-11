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

bcc -c -d -N -m%1 -I..\graflib ventanag.c menug.c cuadrog.c botong.c

if exist ventang%1.lib del ventang%1.lib > NUL

tlib ventang%1.lib +ventanag.obj +menug.obj +cuadrog.obj +botong.obj

del ventanag.obj > NUL
del menug.obj > NUL
del cuadrog.obj > NUL
del botong.obj > NUL

goto fin

:nopar
echo.
echo Uso: VENTANAG x
echo    x = modelo de memoria a usar
echo        T = Tiny    C = Compact
echo        S = Small   L = Large
echo        M = Medium  H = Huge
echo.

:fin
