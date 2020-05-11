@echo off

if '%1'=='' goto nopar

if '%1'=='t' echo Creando librer¡a de gr ficos TINY
if '%1'=='T' echo Creando librer¡a de gr ficos TINY
if '%1'=='s' echo Creando librer¡a de gr ficos SMALL
if '%1'=='S' echo Creando librer¡a de gr ficos SMALL
if '%1'=='c' echo Creando librer¡a de gr ficos COMPACT
if '%1'=='C' echo Creando librer¡a de gr ficos COMPACT
if '%1'=='m' echo Creando librer¡a de gr ficos MEDIUM
if '%1'=='M' echo Creando librer¡a de gr ficos MEDIUM
if '%1'=='l' echo Creando librer¡a de gr ficos LARGE
if '%1'=='L' echo Creando librer¡a de gr ficos LARGE
if '%1'=='h' echo Creando librer¡a de gr ficos HUGE
if '%1'=='H' echo Creando librer¡a de gr ficos HUGE

bcc -c -d -O -G -m%1 grf.c grfblq.c impchr.c pcx.c ratong.c

if exist graf%1.lib del graf%1.lib > NUL

tlib graf%1.lib +grf.obj +grfblq.obj +impchr.obj +pcx.obj +ratong.obj

del grf.obj > NUL
del grfblq.obj > NUL
del impchr.obj > NUL
del pcx.obj > NUL
del ratong.obj > NUL

goto fin

:nopar
echo.
echo Uso: GRAFLIB x
echo    x = modelo de memoria a usar
echo        T = Tiny    C = Compact
echo        S = Small   L = Large
echo        M = Medium  H = Huge
echo.

:fin
