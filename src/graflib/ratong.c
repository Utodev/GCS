/****************************************************************************
				   RATONG.C

	Biblioteca de funciones para gestionar el rat¢n en los modos
	gr ficos de 16 y 256 colores.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- rg_inicializa: inicializa las rutinas de control del rat¢n
		- rg_puntero: muestra u oculta el puntero del rat¢n
		- rg_estado: devuelve el estado actual del rat¢n
		- rg_pon_puntero: coloca el puntero en una posici¢n de
		    pantalla
		- rg_desconecta: desconecta rat¢n.

	Las siguientes estructuras est n definidas en RATONG.H
		STC_RATONG: informaci¢n sobre el estado del rat¢n
****************************************************************************/

#include "ratong.h"

/* Variables globales internas */
static int raton_inicializado=0;	/* 0 rat¢n no inicializado */

/****************************************************************************
	RG_INICIALIZA: inicializa las rutinas de control de rat¢n.
	  Salida:       1 si hay rat¢n conectado, 0 si no
****************************************************************************/
int rg_inicializa(void)
{
int raton;

/* inicializa rutinas de control */
asm {
	mov ax,0        // inicializar 'driver' de rat¢n
	int 33h
	mov raton,ax
}

/* sale si no hay rat¢n */
if(!raton) return(0);

/* si hay rat¢n, visualiza el puntero */
asm {
	mov ax,1
	int 33h
}

raton_inicializado=1;

return(1);
}

/****************************************************************************
	RG_PUNTERO: muestra u oculta el puntero del rat¢n.
	  Entrada:      'modo' puede ser RG_MUESTRA para mostrar el puntero
			o RG_OCULTA para ocultarlo
	  Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
			conectado
****************************************************************************/
int rg_puntero(int modo)
{

/* si rat¢n no est  inicializado, sale */
if(!raton_inicializado) return(0);

asm {
	mov ax,modo
	int 33h
}

return(1);
}

/****************************************************************************
	RG_ESTADO: recoge la posici¢n actual del rat¢n y el estado de los
	  botones.
	  Entrada:      'r' puntero a estructura en la que se devolver 
			la informaci¢n del rat¢n
	  Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
			conectado
****************************************************************************/
int rg_estado(STC_RATONG *r)
{
int bot, x, y, mvid;

r->boton1=0;
r->boton2=0;
r->xv=RG_NOVAL;
r->yv=RG_NOVAL;
r->x=RG_NOVAL;
r->y=RG_NOVAL;
r->fil=RG_NOVAL;
r->col=RG_NOVAL;

/* si rat¢n no est  inicializado, sale */
if(!raton_inicializado) return(0);

/* recoge modo de v¡deo, estado de los botones y posici¢n del rat¢n */
asm {
	mov ah,0fh	// buscar modo de v¡deo
	int 10h         // AH=n£mero de columnas, AL=modo de v¡deo
	xor ah,ah
	mov mvid,ax
	mov ax,3
	int 33h
	mov bot,bx
	mov x,cx
	mov y,dx
}

/* coordenadas virtuales */
r->xv=x;
r->yv=y;

/* coordenadas reales y fila y columna suponiendo caracteres de 8x16 en los */
/* modos de 16 colores y de 8x8 para el de 256 colores */
r->y=y;
if(mvid==0x13) {
	r->x=x/2;
	r->fil=r->y/8;
}
else {
	r->x=x;
	r->fil=r->y/16;
}
r->col=r->x/8;

/* comprueba estado de los botones */
if(bot & 0x0001) r->boton1=1;
if(bot & 0x0002) r->boton2=1;

return(1);
}

/****************************************************************************
	RG_PON_PUNTERO: coloca el puntero en una nueva posici¢n de pantalla.
	  Entrada:	'x', 'y' nuevas coordenadas del rat¢n
	  Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
			conectado
****************************************************************************/
int rg_pon_puntero(int x, int y)
{
int mvid, xv, yv;

/* si rat¢n no est  inicializado, sale */
if(!raton_inicializado) return(0);

/* recoge modo de v¡deo */
asm {
	mov ah,0fh	// buscar modo de v¡deo
	int 10h         // AH=n£mero de columnas, AL=modo de v¡deo
	xor ah,ah
	mov mvid,ax
}

/* convierte coordenadas reales en coordenadas virtuales */
if(mvid==0x13) xv=x*2;
else xv=x;
yv=y;

/* coloca puntero en nueva posici¢n */
asm {
	mov ax,4
	mov cx,xv
	mov dx,yv
	int 33h
}

return(1);
}

/****************************************************************************
	RG_DESCONECTA: desconecta rat¢n.
****************************************************************************/
void rg_desconecta(void)
{

asm {
	mov ax,0        // inicializar 'driver' de rat¢n
	int 33h
}

raton_inicializado=0;

}
