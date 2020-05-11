/****************************************************************************
				   BOTONG.C

	Biblioteca de funciones para gestionar cuadros de botones.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- bg_crea: crea un cuadro de botones
		- bg_dibuja: dibuja un cuadro de botones
		- bg_abre: abre un cuadro de botones
		- bg_cierra: cierra un cuadro de botones
		- bg_pulsado: comprueba si se ha pulsado uno de los botones
		  de un cuadro de botones

	Las siguientes estructuras est n definidas en BOTONG.H:
		STC_ICONO: definici¢n de icono de bot¢n
		STC_BOTONG: definici¢n de un bot¢n
		STC_CUAD_BOTONG: definici¢n de un cuadro de botones
****************************************************************************/

#include <stddef.h>
#include "graf.h"
#include "ventanag.h"
#include "botong.h"

/*** Prototipos de funciones internas ***/
static void dibuja_boton(int fil, int col, int ancho, int alto, int xicn,
  int yicn, unsigned char *icn, BYTE clr_fondo, BYTE clr_s1, BYTE clr_s2);

/****************************************************************************
	DIBUJA_BOTON: dibuja un bot¢n.
	  Entrada:	'fil', 'col' posici¢n del bot¢n en pantalla
			'ancho', 'alto' dimensiones del bot¢n
			'xicn', 'yicn' coordenadas relativas del icono
			respecto al origen del bot¢n
			'icn' puntero al gr fico del icono
			'clr_fondo' color de fondo del bot¢n
			'clr_s1', 'clr_s2' colores de sombra
****************************************************************************/
void dibuja_boton(int fil, int col, int ancho, int alto, int xicn, int yicn,
  unsigned char *icn, BYTE clr_fondo, BYTE clr_s1, BYTE clr_s2)
{
int chr_alt, x0, y0, x1, y1;

rg_puntero(RG_OCULTA);

chr_alt=chr_altura();
x0=col*8;
y0=fil*chr_alt;
x1=x0+(ancho*8)-1;
y1=y0+(alto*chr_alt)-1;

/* rellena interior */
g_rectangulo(x0,y0,x1,y1,clr_fondo,G_NORM,1);

/* dibuja icono */
if(icn!=NULL) blq_pon(x0+xicn,y0+yicn,icn);

/* bordes */
g_linea(x0+1,y0+1,x0+1,y1-1,clr_s1,G_NORM);
g_linea(x0+1,y0+1,x1-1,y0+1,clr_s1,G_NORM);
g_linea(x1-1,y0+2,x1-1,y1-1,clr_s2,G_NORM);
g_linea(x0+2,y1-1,x1-1,y1-1,clr_s2,G_NORM);
g_linea(x0+2,y0+2,x1-2,y0+2,clr_s1,G_NORM);
g_linea(x0+2,y0+2,x0+2,y1-2,clr_s1,G_NORM);
g_linea(x0+3,y1-2,x1-2,y1-2,clr_s2,G_NORM);
g_linea(x1-2,y0+3,x1-2,y1-2,clr_s2,G_NORM);


/* l¡mite exterior */
g_rectangulo(x0,y0,x1,y1,0,G_NORM,0);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	BG_CREA: crea un cuadro de botones.
	  Entrada:	'cbot' puntero a cuadro de botones
			'fil', 'col' posici¢n del cuadro (introducir BG_CENT
			para centrar)
			'nbot_horz' n£mero de botones en horizontal (columnas)
			'nbot_vert' n£mero de botones en vertical (filas)
			'ancho_bot', 'alto_bot' dimensiones de los botones
			'clr_fondo' color de fondo de la ventana
			'clr_pplano' color de primer plano
			'clr_s1', 'clr_s2' colores para sombras
			'titulo' encabezamiento de cuadro
			'dat_icn' matriz de tripletas con datos de iconos de
			los botones (tripletas de la forma {icn, x, y} donde
			"icn" es el puntero (unsigned char) al gr fico
			del icono "x" e "y" la posici¢n	del icono relativa al
			origen del bot¢n; debe haber tantas tripletas como
			botones)
****************************************************************************/
void bg_crea(STC_CUAD_BOTONG *cbot, int fil, int col, int nbot_horz,
  int nbot_vert, int ancho_bot, int alto_bot, BYTE clr_fondo, BYTE clr_pplano,
  BYTE clr_s1, BYTE clr_s2, char *titulo, STC_ICONO dat_icn[])
{
int i, j, k, ancho, alto, filbot, colbot;

/* calcula dimensiones del cuadro */
ancho=nbot_horz*ancho_bot;
alto=(nbot_vert*alto_bot)+1;

/* comprueba si hay que centrar el cuadro */
if(fil==BG_CENT) fil=(chr_maxfil()-alto)/2;
if(col==BG_CENT) col=(chr_maxcol()-ancho)/2;

vg_crea(&cbot->v,fil,col,ancho,alto,clr_fondo,clr_pplano,clr_s1,clr_s2,titulo);

cbot->nbot_horz=nbot_horz;
cbot->nbot_vert=nbot_vert;

k=0;
filbot=1;
colbot=0;
for(i=0; i<nbot_vert; i++) {
	for(j=0; j<nbot_horz; j++) {
		cbot->bot[k].fil=filbot;
		cbot->bot[k].col=colbot;
		cbot->bot[k].ancho=ancho_bot;
		cbot->bot[k].alto=alto_bot;
		cbot->bot[k].icono.icn=dat_icn[k].icn;
		cbot->bot[k].icono.x=dat_icn[k].x;
		cbot->bot[k].icono.y=dat_icn[k].y;
		k++;
		colbot+=ancho_bot;
	}
	colbot=0;
	filbot+=alto_bot;
}

}

/****************************************************************************
	BG_DIBUJA: dibuja un cuadro de botones.
	  Entrada:	'cbot' puntero a cuadro de botones
****************************************************************************/
void bg_dibuja(STC_CUAD_BOTONG *cbot)
{
int i, j, k;

vg_dibuja(&cbot->v,1);

k=0;
for(i=0; i<cbot->nbot_vert; i++) {
	for(j=0; j<cbot->nbot_horz; j++) {
		dibuja_boton(cbot->v.fil+cbot->bot[k].fil,cbot->v.col+
		  cbot->bot[k].col,cbot->bot[k].ancho,cbot->bot[k].alto,
		  cbot->bot[k].icono.x,cbot->bot[k].icono.y,
		  cbot->bot[k].icono.icn,cbot->v.clr_fondo,cbot->v.clr_s1,
		  cbot->v.clr_s2);
		k++;
	}
}

}

/****************************************************************************
	BG_ABRE: abre un cuadro de botones.
	  Entrada:	'cbot' puntero a cuadro de botones
****************************************************************************/
void bg_abre(STC_CUAD_BOTONG *cbot)
{
int i, j, k;

vg_abre(&cbot->v);

k=0;
for(i=0; i<cbot->nbot_vert; i++) {
	for(j=0; j<cbot->nbot_horz; j++) {
		dibuja_boton(cbot->v.fil+cbot->bot[k].fil,cbot->v.col+
		  cbot->bot[k].col,cbot->bot[k].ancho,cbot->bot[k].alto,
		  cbot->bot[k].icono.x,cbot->bot[k].icono.y,
		  cbot->bot[k].icono.icn,cbot->v.clr_fondo,cbot->v.clr_s1,
		  cbot->v.clr_s2);
		k++;
	}
}

}

/****************************************************************************
	BG_CIERRA: cierra un cuadro de botones.
	  Entrada:	'cbot' puntero a cuadro de botones
****************************************************************************/
void bg_cierra(STC_CUAD_BOTONG *cbot)
{

vg_cierra(&cbot->v);

}

/****************************************************************************
	BG_PULSADO: comprueba si se ha pulsado uno de los botones de un
	  cuadro de botones.
	  Entrada:	'cbot' puntero a cuadro de botones
			'x', 'y' coordenadas de pantalla
	  Salida:	n£mero de bot¢n correspondiente a las coordenadas
			dadas (se cuentan de izquierda a derecha y de
			arriba a abajo), -1 si ninguno
****************************************************************************/
int bg_pulsado(STC_CUAD_BOTONG *cbot, int x, int y)
{
STC_RATONG r;
int i, j, k, chr_alt, x0, y0, x1, y1;

chr_alt=chr_altura();

k=0;
for(i=0; i<cbot->nbot_vert; i++) {
	for(j=0; j<cbot->nbot_horz; j++) {
		x0=(cbot->v.col+cbot->bot[k].col)*8;
		y0=(cbot->v.fil+cbot->bot[k].fil)*chr_alt;
		x1=x0+(cbot->bot[k].ancho*8)-1;
		y1=y0+(cbot->bot[k].alto*chr_alt)-1;

		if((x>=x0) && (x<=x1) && (y>=y0) && (y<=y1)) {
			/* si hay rat¢n conectado y est  pulsado */
			/* bot¢n, dibuja pulsaci¢n de bot¢n */
			if(rg_estado(&r)) {
				if(r.boton1 || r.boton2) {
					dibuja_boton(cbot->v.fil+
					  cbot->bot[k].fil,cbot->v.col+
					  cbot->bot[k].col,cbot->bot[k].ancho,
					  cbot->bot[k].alto,
					  cbot->bot[k].icono.x,
					  cbot->bot[k].icono.y,
					  cbot->bot[k].icono.icn,
					  cbot->v.clr_fondo,
					  cbot->v.clr_s2,cbot->v.clr_s1);
					do {
						rg_estado(&r);
					} while(r.boton1 || r.boton2);
					dibuja_boton(cbot->v.fil+
					  cbot->bot[k].fil,cbot->v.col+
					  cbot->bot[k].col,cbot->bot[k].ancho,
					  cbot->bot[k].alto,
					  cbot->bot[k].icono.x,
					  cbot->bot[k].icono.y,
					  cbot->bot[k].icono.icn,
					  cbot->v.clr_fondo,
					  cbot->v.clr_s1,cbot->v.clr_s2);
				}
			}
			return(k);
		}

		k++;
	}
}

return(-1);
}
