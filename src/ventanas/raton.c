/****************************************************************************
				  RATON.C

	Biblioteca de funciones para gestionar el rat¢n.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- r_inicializa: inicializa las rutinas de control del rat¢n
		- r_puntero: muestra u oculta el puntero del rat¢n
		- r_estado: devuelve el estado actual del rat¢n
		- r_pon_puntero: coloca el puntero en una posici¢n de
		    pantalla
		- r_desconecta: desconecta rat¢n

	Las siguientes estructuras est n definidas en RATON.H
		STC_RATON: informaci¢n sobre el estado del rat¢n
****************************************************************************/

#include "raton.h"

/* Variables globales internas */
static int raton_inicializado=0;	/* 0 rat¢n no inicializado */

/****************************************************************************
	R_INICIALIZA: inicializa las rutinas de control de rat¢n.
	  Salida:       1 si hay rat¢n conectado, 0 si no
****************************************************************************/
int r_inicializa(void)
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
	R_PUNTERO: muestra u oculta el puntero del rat¢n.
	  Entrada:      'modo' puede ser R_MUESTRA para mostrar el puntero
			o R_OCULTA para ocultarlo
	  Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
			conectado
****************************************************************************/
int r_puntero(int modo)
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
	R_ESTADO: recoge la posici¢n actual del rat¢n y el estado de los
	  botones.
	  Entrada:      'r' puntero a estructura en la que se devolver 
			la informaci¢n del rat¢n
	  Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
			conectado
****************************************************************************/
int r_estado(STC_RATON *r)
{
int bot, x, y;

r->boton1=0;
r->boton2=0;
r->xv=R_NOVAL;
r->yv=R_NOVAL;
r->fil=R_NOVAL;
r->col=R_NOVAL;

/* si rat¢n no est  inicializado, sale */
if(!raton_inicializado) return(0);

/* recoge estado de los botones y posici¢n del rat¢n */
asm {
	mov ax,3
	int 33h
	mov bot,bx
	mov x,cx
	mov y,dx
}

/* coordenadas virtuales */
r->xv=x;
r->yv=y;

/* calcula fila, columna suponiendo una pantalla virtual de 640x200 */
/* y que est  en un modo de texto de 80x25 */
r->col=x/8;
r->fil=y/8;

/* comprueba estado de los botones */
if(bot & 0x0001) r->boton1=1;
if(bot & 0x0002) r->boton2=1;

return(1);
}

/****************************************************************************
	R_PON_PUNTERO: coloca el puntero en una nueva posici¢n de pantalla.
	  Entrada:	'fil', 'col' nueva posici¢n del rat¢n
	  Salida:       1 si rat¢n inicializado, 0 si no o no hay rat¢n
			conectado
****************************************************************************/
int r_pon_puntero(int fil, int col)
{
int xv, yv;

/* si rat¢n no est  inicializado, sale */
if(!raton_inicializado) return(0);

/* convierte coordenadas reales en coordenadas virtuales, suponiendo una */
/* pantalla virtual de 640x200 y que est  en un modo de texto de 80x25 */
xv=col*8;
yv=fil*8;

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
	R_DESCONECTA: desconecta rat¢n.
****************************************************************************/
void r_desconecta(void)
{

asm {
	mov ax,0        // inicializar 'driver' de rat¢n
	int 33h
}

raton_inicializado=0;

}
