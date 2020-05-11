/****************************************************************************
				  VENTANAG.C

	Biblioteca de funciones para gestionar ventanas en pantalla en
	los modos gr ficos de 16 y 256 colores.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- vg_impcar: imprime un car cter y su atributo en una
		    posici¢n de pantalla
		- vg_crea: crea una ventana
		- vg_dibuja: dibuja una ventana
		- vg_abre: abre una ventana
		- vg_cierra: cierra una ventana
		- vg_pon_cursor: cambia la posici¢n de impresi¢n del
		    texto dentro de una ventana
		- vg_impc: imprime un car cter en una ventana
		- vg_impcad: imprime una cadena en una ventana
		- vg_modo_texto: cambia el modo de impresi¢n del texto en
		    una ventana
		- vg_borra: borra una ventana
		- vg_color: cambia el color de impresi¢n del texto en una
		    ventana
		- vg_borde: redefine los caracteres del borde de una ventana
		- vg_scroll_arr: scroll hacia arriba de una ventana
		- vg_scroll_abj: scroll hacia abajo de una ventana

	Las siguientes estructuras est n definidas en VENTANAG.H:
		STC_VENTANAG: definici¢n de una ventana
****************************************************************************/

#include <stddef.h>
#include <alloc.h>
#include <string.h>
#include "graf.h"
#include "ventanag.h"

/****************************************************************************
	VG_IMPCAR: imprime un car cter en una posici¢n de pantalla y con
	  un atributo dado.
	  Entrada:      'fil', 'col' fila y columna d¢nde se imprimir  el
			car cter (origen de pantalla en 0,0)
			'car' car cter a imprimir
			'colorf', 'color' colores de fondo y primer plano
****************************************************************************/
void vg_impcar(int fil, int col, BYTE car, BYTE colorf, BYTE color)
{

imp_chr_pos(col*8,fil*chr_altura());
imp_chr(car,colorf,color,CHR_NORM);

}

/****************************************************************************
	VG_CREA: inicializa una ventana rellenando la estructura con los
	  datos suministrados.
	  Entrada:      'ventana' puntero a estructura con datos de ventana
			'fil', 'col' posici¢n de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'clr_fondo', 'clr_pplano' colores de fondo y
			primer plano de la ventana
			'clr_s1' color para sombra 1
			'clr_s2' color para sombra 2
			'titulo' texto para encabezamiento de ventana (NULL
			si ninguno)
****************************************************************************/
void vg_crea(STC_VENTANAG *ventana, int fil, int col, int ancho,
  int alto, BYTE clr_fondo, BYTE clr_pplano, BYTE clr_s1, BYTE clr_s2,
  char *titulo)
{

/* inicializa datos ventana */
ventana->fil=fil;
ventana->col=col;
ventana->ancho=ancho;
ventana->alto=alto;
ventana->clr_fondo=clr_fondo;
ventana->clr_pplano=clr_pplano;
ventana->clr_s1=clr_s1;
ventana->clr_s2=clr_s2;
ventana->borde=VG_BORDE0;
ventana->titulo=titulo;

/* puntero a buffer para guardar el fondo */
ventana->fondo=NULL;

/* modo de impresi¢n de texto */
ventana->modo_texto=VG_LINEA_LINEA;

/* posici¢n de impresi¢n dentro de la ventana */
ventana->filc=0;
ventana->colc=0;

/* color del texto */
ventana->clr_textof=clr_fondo;
ventana->clr_textop=clr_pplano;

}

/****************************************************************************
	VG_DIBUJA: dibuja una ventana.
	  Entrada:     'ventana' puntero a estructura con datos de ventana
		       'rellena' 0 s¢lo dibuja marco, 1 rellena interior
****************************************************************************/
void vg_dibuja(STC_VENTANAG *ventana, int rellena)
{
int i, x0, y0, x1, y1, chr_alt, y, xr0, xr1, lng, col;
char *t;

rg_puntero(RG_OCULTA);

chr_alt=chr_altura();
x0=ventana->col*8;
y0=ventana->fil*chr_alt;
x1=x0+(ventana->ancho*8)-1;
y1=y0+(ventana->alto*chr_alt)-1;

/* bordes exteriores */
g_linea(x0+1,y0+1,x0+1,y1-1,ventana->clr_s1,G_NORM);
g_linea(x0+1,y0+1,x1-1,y0+1,ventana->clr_s1,G_NORM);
g_linea(x1-1,y0+2,x1-1,y1-1,ventana->clr_s2,G_NORM);
g_linea(x0+2,y1-1,x1-1,y1-1,ventana->clr_s2,G_NORM);
g_linea(x0+2,y0+2,x1-2,y0+2,ventana->clr_s1,G_NORM);
g_linea(x0+2,y0+2,x0+2,y1-2,ventana->clr_s1,G_NORM);
g_linea(x0+3,y1-2,x1-2,y1-2,ventana->clr_s2,G_NORM);
g_linea(x1-2,y0+3,x1-2,y1-2,ventana->clr_s2,G_NORM);

/* espacio entre bordes */
for(i=3; i<=4; i++) g_rectangulo(x0+i,y0+i,x1-i,y1-i,ventana->clr_fondo,
  G_NORM,0);

/* bordes interiores */
if(ventana->borde==VG_BORDE1) {
	g_linea(x0+5,y0+5,x1-6,y0+5,ventana->clr_s2,G_NORM);
	g_linea(x0+5,y0+5,x0+5,y1-6,ventana->clr_s2,G_NORM);
	g_linea(x1-5,y0+5,x1-5,y1-5,ventana->clr_s1,G_NORM);
	g_linea(x0+5,y1-5,x1-5,y1-5,ventana->clr_s1,G_NORM);
}
else g_rectangulo(x0+5,y0+5,x1-5,y1-5,ventana->clr_fondo,G_NORM,0);

/* rellena interior */
if(rellena) {
	xr0=x0+6;
	xr1=x1-6;
	g_rectangulo(x0+6,y0+6,x1-6,y1-6,ventana->clr_fondo,G_NORM,1);
}

/* imprime encabezamiento */
col=ventana->col;
if(ventana->titulo!=NULL) {
	lng=strlen(ventana->titulo);
	col+=((ventana->ancho-lng)/2);
	if(col<=ventana->col) col=ventana->col+1;

	/* dibuja encabezamiento de ventana */
	xr0=x0+2;
	xr1=x1-2;
	y=y0+chr_alt-1;
	g_rectangulo(xr0,y0+2,xr1,y,ventana->clr_fondo,G_NORM,1);
	if(ventana->borde==VG_BORDE0) g_linea(x0,y,x1,y,ventana->clr_s2,G_NORM);
	else g_linea(xr0,y,xr1,y,ventana->clr_s2,G_NORM);
	xr0=(col*8)-1;
	xr1=(col+lng)*8;
	if(xr1>(x1-7)) xr1=x1-7;
	g_linea(xr0,y0,xr0,y,ventana->clr_s2,G_NORM);
	g_linea(xr1,y0,xr1,y,ventana->clr_s1,G_NORM);

	for(t=ventana->titulo; *t; t++, col++) {
		if(col<(ventana->col+ventana->ancho-1))
		  vg_impcar(ventana->fil,col,*t,ventana->clr_fondo,
		    ventana->clr_pplano);
	}
}

/* l¡mite exterior */
g_rectangulo(x0,y0,x1,y1,0,G_NORM,0);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VG_ABRE: abre una ventana.
	  Entrada:      'ventana' puntero a estructura con datos de ventana
****************************************************************************/
void vg_abre(STC_VENTANAG *ventana)
{
int x0, y0, x1, y1;

rg_puntero(RG_OCULTA);

x0=ventana->col*8;
y0=ventana->fil*chr_altura();
x1=x0+(ventana->ancho*8)-1;
y1=y0+(ventana->alto*chr_altura())-1;

/* reserva memoria y guarda el fondo */
ventana->fondo=(BYTE far *)farmalloc(blq_tam(x0,y0,x1,y1));
if(ventana->fondo!=NULL) blq_coge(x0,y0,x1,y1,ventana->fondo);

/* dibuja la ventana */
vg_dibuja(ventana,1);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VG_CIERRA: cierra una ventana.
	  Entrada:      'ventana' puntero a estructura con datos de ventana
****************************************************************************/
void vg_cierra(STC_VENTANAG *ventana)
{

rg_puntero(RG_OCULTA);

/* si tiene fondo guardado lo recupera y libera memoria */
if(ventana->fondo!=NULL) {
	blq_pon(ventana->col*8,ventana->fil*chr_altura(),ventana->fondo);
	farfree(ventana->fondo);
	ventana->fondo=NULL;
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VG_PON_CURSOR: cambia la posici¢n de impresi¢n del texto dentro de
	  una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'fil', 'col' posici¢n relativa dentro de la
			ventana:
				'fil' = 0 .. (alto-3)
				'col' = 0 .. (ancho-3)
****************************************************************************/
void vg_pon_cursor(STC_VENTANAG *ventana, int fil, int col)
{

ventana->filc=fil;
ventana->colc=col;

}

/****************************************************************************
	VG_IMPC: imprime un car cter dentro de una ventana, en la posici¢n
	  actual de impresi¢n.
	  Entrada:      'ventana' puntero a estructura de ventana
			'car' car cter a imprimir
	  NOTA: si el car cter cae fuera de la ventana, no lo imprime
****************************************************************************/
void vg_impc(STC_VENTANAG *ventana, char car)
{
STC_RATONG r;
int sobre_car=0, maxfil, maxcol, fil, col;

/* calcula m xima fila y columna */
maxfil=ventana->alto-3;
maxcol=ventana->ancho-3;

/* si el car cter est  fuera de la ventana, sale */
if((ventana->filc>maxfil) | (ventana->colc>maxcol)) return;

/* posici¢n del car cter en pantalla */
fil=ventana->fil+ventana->filc+1;
col=ventana->col+ventana->colc+1;

/* si el puntero del rat¢n est  sobre el car cter lo oculta */
rg_estado(&r);
if((fil>=r.fil) && (fil<=(r.fil+2)) && (col>=r.col) && (col<=(r.col+3))) {
	rg_puntero(RG_OCULTA);
	sobre_car=1;
}

vg_impcar(fil,col,(BYTE)car,ventana->clr_textof,ventana->clr_textop);

/* siguiente columna */
ventana->colc++;

if(sobre_car) rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VG_IMPCAD: imprime una cadena dentro de una ventana, en la posici¢n
	  actual de impresi¢n.
	  Entrada:      'ventana' puntero a estructura de ventana
			'cad' cadena a imprimir
			'rellena' VG_RELLENA si se quiere rellenar hasta
			el final de la ventana con espacios, VG_NORELLENA
			en otro caso
****************************************************************************/
void vg_impcad(STC_VENTANAG *ventana, char *cad, int rellena)
{

if(ventana->filc>=(ventana->alto-2)) return;

rg_puntero(RG_OCULTA);

while(*cad) {
	if(*cad=='\n') {
		/* rellena con espacios */
		if(rellena==VG_RELLENA) {
			for(; ventana->colc<(ventana->ancho-2); ventana->colc++)
			  vg_impcar(ventana->fil+ventana->filc+1,
			    ventana->col+ventana->colc+1,' ',
			    ventana->clr_textof,ventana->clr_textop);
		}
		ventana->colc=0;
		ventana->filc++;
		if(ventana->filc>=(ventana->alto-2)) {
			rg_puntero(RG_MUESTRA);
			return;
		}
	}
	else {
		if(ventana->colc<(ventana->ancho-2))
		  vg_impcar(ventana->fil+ventana->filc+1,
		    ventana->col+ventana->colc+1,*cad,ventana->clr_textof,
		    ventana->clr_textop);
		else if(ventana->modo_texto==VG_PASA_LINEA) {
			ventana->colc=0;
			ventana->filc++;
			vg_impcar(ventana->fil+ventana->filc+1,
			  ventana->col+ventana->colc+1,*cad,
			  ventana->clr_textof,
			  ventana->clr_textop);
		}
		ventana->colc++;
	}
	cad++;
}

/* rellena con espacios */
if((*(cad-1)!='\n') && (rellena==VG_RELLENA)) {
	for(; ventana->colc<(ventana->ancho-2); ventana->colc++)
	  vg_impcar(ventana->fil+ventana->filc+1,ventana->col+ventana->colc+1,
	    ' ',ventana->clr_textof,ventana->clr_textop);
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VG_MODO_TEXTO:  cambia el modo de impresi¢n del texto dentro de
	  una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'modo' modo de impresi¢n:
				VG_LINEA_LINEA imprime una l¡nea de texto en
				  cada l¡nea de la ventana, si las l¡neas
				  de texto son m s largas que la ventana
				  quedan recortadas
				VG_PASA_LINEA si una l¡nea de texto es m s
				  larga que la ventana, la pasa a la
				  siguiente
****************************************************************************/
void vg_modo_texto(STC_VENTANAG *ventana, int modo)
{

ventana->modo_texto=modo;

}

/****************************************************************************
	VG_BORRA: borra el interior de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
****************************************************************************/
void vg_borra(STC_VENTANAG *ventana)
{

/* coloca posici¢n de impresi¢n en origen */
ventana->filc=0;
ventana->colc=0;

/* restaura color del texto */
ventana->clr_textof=ventana->clr_fondo;
ventana->clr_textop=ventana->clr_pplano;

vg_dibuja(ventana,1);

}

/****************************************************************************
	VG_COLOR: cambia el color de impresi¢n del texto en la ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'fondo', 'pplano' nuevos colores para el texto
****************************************************************************/
void vg_color(STC_VENTANAG *ventana, BYTE fondo, BYTE pplano)
{

ventana->clr_textof=fondo;
ventana->clr_textop=pplano;

}

/****************************************************************************
	VG_BORDE: cambia el tipo de borde de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
			'borde' nuevo tipo de borde (VG_BORDE...)
****************************************************************************/
void vg_borde(STC_VENTANAG *ventana, int borde)
{

ventana->borde=borde;

}

/****************************************************************************
	VG_SCROLL_ARR: realiza scroll hacia arriba de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
****************************************************************************/
void vg_scroll_arr(STC_VENTANAG *ventana)
{

rg_puntero(RG_OCULTA);
g_scroll_arr(ventana->fil+1,ventana->col+1,ventana->ancho-2,ventana->alto-2,
  ventana->clr_fondo);
rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	VG_SCROLL_ABJ: realiza scroll hacia abajo de una ventana.
	  Entrada:      'ventana' puntero a estructura de ventana
****************************************************************************/
void vg_scroll_abj(STC_VENTANAG *ventana)
{

rg_puntero(RG_OCULTA);
g_scroll_abj(ventana->fil+1,ventana->col+1,ventana->ancho-2,ventana->alto-2,
  ventana->clr_fondo);
rg_puntero(RG_MUESTRA);

}
