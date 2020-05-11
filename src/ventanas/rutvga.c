/****************************************************************************
				 RUTVGA.C

	Biblioteca de funciones varias para modo texto de VGA.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- vga_parpadeo: activa/desactiva el atributo de parpadeo
		- vga_activa_fuente: carga y activa un fichero de juego
		  de caracteres

****************************************************************************/

#include <stdio.h>
#include <alloc.h>
#include "rutvga.h"

/*** Variables globales internas */
char *vga_fuente8x16=NULL;	/* puntero a juego de caracteres 8x16 */

/****************************************************************************
	VGA_PARPADEO: activa/desactiva el parpadeo.
	  Entrada:      'modo' 0 desactiva el parpadeo, 1 lo activa
****************************************************************************/
void vga_parpadeo(int modo)
{

asm {
	mov ax,1003h
	mov bl,byte ptr modo
	int 10h
}

}

/****************************************************************************
	VGA_ACTIVA_FUENTE: carga y activa un fichero de juego de caracteres.
	  Entrada:	'nombre' nombre del fichero (NULL o vacio para
 			restaurar el juego de caracteres de la ROM)
	  Salida:	0 si error o no es VGA, 1 si hubo ‚xito

	  NOTA:		el formato del fichero esperado es con una cabecera
 			de 16 bytes, seguido de una tabla de anchuras de los
 			caracteres de 8x16, seguido de las definiciones de
 			los caracteres de 8x16
****************************************************************************/
int vga_activa_fuente(char *nombre)
{
FILE *ffuente;
char b[1024];
int vga;

/* reserva memoria para juego de caracteres (si no est  ya reservada) */
if(vga_fuente8x16==NULL) vga_fuente8x16=(char *)malloc(4096);

/* pasa a modo de 400 lineas */
vga=0;
asm {
	mov ah,12h
	mov al,2
	mov bl,30h
	int 10h
	mov byte ptr vga,al
}
if(!vga) return(0);

/* si no dio nombre de fichero restaura el juego de caracteres de la ROM */
if(!*nombre || (nombre==NULL)) {
	asm {
		mov ah,11h
		mov al,14h
		mov bl,0
		int 10h
	}
	return(1);
}

/* si no hay memoria para juego de caracteres, sale */
if(vga_fuente8x16==NULL) return(0);

/* abre el fichero para lectura */
ffuente=fopen(nombre,"rb");
/* sale si hubo error */
if(ffuente==NULL) return(0);

/* lee cabecera */
if(fread(b,sizeof(char),16,ffuente)<16) {
	fclose(ffuente);
	return(0);
}

/* lee tabla de anchuras 8x16 */
if(fread(b,sizeof(char),256,ffuente)<256) {
	fclose(ffuente);
	return(0);
}

/* lee definiciones 8x16 */
if(fread(vga_fuente8x16,sizeof(char),4096,ffuente)<4096) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

/* activa fuente de 8x16 */
asm {
	push bp
	mov ax,1100h
	mov bx,1000h
	mov cx,256
	mov dx,0
	les si,vga_fuente8x16
	mov bp,si
	int 10h
	pop bp
}

return(1);
}
