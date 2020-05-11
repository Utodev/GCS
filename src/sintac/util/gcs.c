/****************************************************************************
		       GENERADOR DE CARACTERES SINTAC
			    (c)1995 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <dir.h>
#include <dos.h>
#include <bios.h>
#include <io.h>
#include <graphics.h>
#include "graf.h"
#include "ventanag.h"
#include "menug.h"
#include "cuadrog.h"
#include "botong.h"
#include "sintac.h"
#include "gcs.h"

/*** Variables globales ***/
/* tablas donde se almacenar n las definiciones de los caracteres 8x16 y 8x8 */
BYTE tabla_ascii8x16[256][16];
BYTE tabla_ascii8x8[256][8];
/* tablas para guardar anchuras de caracteres */
BYTE tabla_anch8x16[256];
BYTE tabla_anch8x8[256];

/* ventanas */
STC_VENTANAG vcuad;		/* cuadr¡cula de dibujo */
STC_VENTANAG vtabla;		/* tabla de caracteres ASCII */
STC_VENTANAG vinfo;		/* ventana de informaci¢n */

/* cuadro de botones */
STC_CUAD_BOTONG cbot;
STC_ICONO icn_cbot[NBOT];

BYTE *tabla_ascii;              /* puntero a tabla definiciones caracteres */
int altura_cuad=16;             /* altura de caracteres actual */
BYTE caracter[16];              /* definici¢n car cter actual */
int ascii=0;                    /* c¢digo ASCII de car cter actual */
int car_modificado=0;           /* indicador de modificaci¢n de caracteres */

/*** Programa principal ***/
#pragma warn -par
void main(int argc, char *argv[])
{
STC_RATONG r;
STC_MENUG *m0, *m1, *m2, *m3;
char drive[MAXDRIVE], dir[MAXDIR], file[MAXFILE], ext[MAXEXT], nf_icn[MAXPATH];
char *s, *cab="GENERADOR DE CARACTERES SINTAC versi¢n "GCS_VERSION
  "  (c)1995 JSJ Soft Ltd.";
int mvid, tecla, cuad_x=0, cuad_y=0, i, j;

/* recoge directorio d¢nde est  el programa */
fnsplit(argv[0],drive,dir,file,ext);
fnmerge(nf_icn,drive,dir,"GCS",".ICN");

/* coge modo de v¡deo actual */
mvid=g_coge_modovideo();

/* selecciona modo gr fico */
if(!g_modovideo(G_MV_G2C16)) {
	printf("\nEste programa requiere tarjeta gr fica VGA.\n");
	exit(1);
}

rg_inicializa();

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

/* copia definiciones de tabla ROM de caracteres 8x16 y 8x8 en RAM */
copia_tabla_rom(TAB_ROM8x16,&tabla_ascii8x16[0][0],16);
copia_tabla_rom(TAB_ROM8x8,&tabla_ascii8x8[0][0],8);
for(i=0; i<256; i++) {
	tabla_anch8x16[i]=8;
	tabla_anch8x8[i]=8;
}

/* inicializa puntero a definiciones de caracteres */
tabla_ascii=&tabla_ascii8x16[0][0];

/* rellena pantalla */
rg_puntero(RG_OCULTA);
g_rectangulo(0,0,639,349,COLOR_FONDO,G_NORM,1);
rg_puntero(RG_MUESTRA);
imp_chr_pos(56,0);
for(s=cab; *s; s++) imp_chr(*s,COLOR_FONDO,COLOR_PPLANO,CHR_NORM);
g_linea(0,33,639,33,COLOR_PPLANO,G_NORM);

/* crea men£s */
mg_color(COLOR_FONDO,COLOR_PPLANO,COLOR_S1,COLOR_S2,COLOR_TECLA);
m0=mg_crea(MENUG_HORZ | MENUG_FIJO,NULL,
  "\xff^Fichero\xff:\xff^Editar\xff:""\xff""Efec^tos\xff"
  "                                                  ",
  1,0,1);
m1=mg_crea(MENUG_VERT,NULL," ^Cargar fuente : ^Grabar fuente| ^Salir",
  2,1,0);
m2=mg_crea(MENUG_VERT,NULL," Fuente 8x1^6 : Fuente ^8x8| ^Prueba",
  2,11,0);
m3=mg_crea(MENUG_VERT,NULL," I^nvertir: Rayado ^horizontal : Rayado ^vertical:"
  " Cursiva ^izquierda: Cursiva ^derecha: Ne^grita",2,20,0);

mg_abre(m0);

/* crea cuadro de botones */
for(i=0; i<NBOT; i++) {
	icn_cbot[i].icn=NULL;
	icn_cbot[i].x=2;
	icn_cbot[i].y=2;
}
carga_iconos(nf_icn,NBOT,icn_cbot);
bg_crea(&cbot,3,60,3,6,4,2,COLOR_FONDO,COLOR_PPLANO,COLOR_S1,COLOR_S2," FX ",
  icn_cbot);
bg_dibuja(&cbot);

/* cuadricula zona de dibujo */
vg_crea(&vcuad,3,45,14,ALT_C8X16,COLOR_FONDO,COLOR_PPLANO,COLOR_S1,COLOR_S2,
  " Car cter ");
vg_dibuja(&vcuad,1);
dibuja_cuadricula((vcuad.col+1)*8,(vcuad.fil+1)*16);
dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,(vcuad.fil+1)*16,1);
dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);

/* tabla ASCII */
vg_crea(&vtabla,3,1,35,ALT_T8X16,COLOR_FONDO,COLOR_PPLANO,COLOR_S1,COLOR_S2,
  " Tabla ASCII ");
vg_dibuja(&vtabla,1);
dibuja_tabla_ascii((vtabla.col+1)*8,(vtabla.fil+1)*16);

/* dibuja marcador en el car cter actual de la tabla */
dibuja_marcador_ascii(ascii);

/* c¢digo ASCII del car cter actual */
vg_crea(&vinfo,6,38,5,3,COLOR_FONDO,COLOR_PPLANO,COLOR_S2,COLOR_S1,NULL);
vg_dibuja(&vinfo,1);
imp_ascii_car();

/* borra la definici¢n del car cter y lo dibuja en pantalla */
limpia_caracter(caracter,0);
g_rectangulo(DEFCAR_X,DEFCAR_Y,DEFCAR_X+DEFCAR_ANCHO-1,DEFCAR_Y+DEFCAR_ALTO-1,
  0,G_NORM,1);
dibuja_caracter();

while(1) {
	/* espera a pulsaci¢n de tecla o bot¢n de rat¢n */
	do {
		tecla=bioskey(1);
		rg_estado(&r);
	} while(!tecla && !r.boton1 && !r.boton2);

	/* saca tecla de buffer de teclado y coge c¢digo de 'scan' */
	if(tecla) {
		bioskey(0);
		tecla=(tecla >> 8) & 0x00ff;
	}

	/* comprueba si puls¢ sobre men£ de opciones */
	if((tecla==T_ESC) | ((r.fil>=m0->v.fil) &&
	  (r.fil<=(m0->v.fil+m0->v.alto-1)) && (r.col>=m0->v.col) &&
	  (r.col<=(m0->v.col+m0->v.ancho-1)) && r.boton1)) {
		i=mg_elige_opcion(m0);

		switch(i) {
			case 0 :
				mg_abre(m1);
				j=mg_elige_opcion(m1);
				mg_cierra(m1);

				switch(j) {
					case 0 :
						tablacar_modificada();
						dibuja_marc_ancho(
						  (vcuad.col+1)*8,
						  (vcuad.fil+1)*16,
						  ascii);
						cargar_fuente();
						dibuja_marc_ancho(
						  (vcuad.col+1)*8,
						  (vcuad.fil+1)*16,
						  ascii);
						break;
					case 1 :
						grabar_fuente();
						break;
					case 3 :
						tablacar_modificada();
						mg_elimina(m3);
						mg_elimina(m2);
						mg_elimina(m1);
						mg_elimina(m0);
						for(i=0; i<NBOT; i++)
						  free(icn_cbot[i].icn);
						g_modovideo(mvid);
						exit(0);
				}
				break;
			case 1 :
				mg_abre(m2);
				j=mg_elige_opcion(m2);
				mg_cierra(m2);

				switch(j) {
					case 0 :
						/* si no est  en 8x16 */
						if(altura_cuad!=16) {
							pasa_a8x16();
							cuad_x=0;
							cuad_y=0;
							dibuja_marc_cuadricula(
							  cuad_x,cuad_y,
							  (vcuad.col+1)*8,
							  (vcuad.fil+1)*16,1);
							dibuja_marc_ancho(
							  (vcuad.col+1)*8,
							  (vcuad.fil+1)*16,
							  ascii);
						}
						break;
					case 1 :
						/* si no est  en 8x8 */
						if(altura_cuad!=8) {
							pasa_a8x8();
							cuad_x=0;
							cuad_y=0;
							dibuja_marc_cuadricula(
							  cuad_x,cuad_y,
							  (vcuad.col+1)*8,
							  (vcuad.fil+1)*16,1);
							dibuja_marc_ancho(
							  (vcuad.col+1)*8,
							  (vcuad.fil+1)*16,
							  ascii);
						}
						break;
					case 3 :
						prueba_car();
						break;
				}
				break;
			case 2 :
				mg_abre(m3);
				j=mg_elige_opcion(m3);
				mg_cierra(m3);

				switch(j) {
					case 0 :
						invertir_def();
						break;
					case 1 :
						rayado_horizontal();
						break;
					case 2 :
						rayado_vertical();
						break;
					case 3 :
						cursiva_izquierda();
						break;
					case 4 :
						cursiva_derecha();
						break;
					case 5 :
						negrita();
						break;
				}
				break;
		}
	}
	/* comprueba si puls¢ sobre cuadr¡cula */
	else if((r.fil>=(vcuad.fil+1)) && (r.fil<=(vcuad.fil+vcuad.alto-2)) &&
	  (r.col>=(vcuad.col+1)) && (r.col<=(vcuad.col+vcuad.ancho-2)) &&
	  (r.boton1 || r.boton2)) cuadricula(r.x-((vcuad.col+1)*8),
	  r.y-((vcuad.fil+1)*16));
	/* comprueba si se puls¢ en tabla ASCII */
	else if((r.fil>=(vtabla.fil+1)) &&
	  (r.fil<=(vtabla.fil+vtabla.alto-2)) && (r.col>=(vtabla.col+1)) &&
	  (r.col<=(vtabla.col+vtabla.ancho-2)) && r.boton1)
	  tab_ascii(r.x-((vtabla.col+1)*8),r.y-((vtabla.fil+1)*16));
	/* comprueba si puls¢ sobre definici¢n de car cter */
	else if((r.x>=DEFCAR_X) && (r.x<=(DEFCAR_X+DEFCAR_ANCHO-1)) &&
	  (r.y>=DEFCAR_Y) && (r.y<=(DEFCAR_Y+DEFCAR_ALTO-1))) def_caracter();
	/* comprueba funciones de teclado */
	else if(tecla==T_IZQ) {
		/* si est  pulsado SHIFT mueve marcador tabla ASCII */
		if(bioskey(2) & 0x0003) {
			if(ascii>0) {
				dibuja_marcador_ascii(ascii);
				dibuja_marc_ancho((vcuad.col+1)*8,
				  (vcuad.fil+1)*16,ascii);
				ascii--;
				imp_ascii_car();
				dibuja_marcador_ascii(ascii);
				dibuja_marc_ancho((vcuad.col+1)*8,
				  (vcuad.fil+1)*16,ascii);
				copia_def_car(tabla_ascii+(ascii*altura_cuad),
				  caracter);
				dibuja_en_cuadricula();
				dibuja_caracter();
			}
		}
		else if(cuad_x>0) {
			dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
			  (vcuad.fil+1)*16,0);
			cuad_x--;
			dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
			  (vcuad.fil+1)*16,1);
		}
	}
	else if(tecla==T_DER) {
		/* si est  pulsado SHIFT mueve marcador tabla ASCII */
		if(bioskey(2) & 0x0003) {
			if(ascii<255) {
				dibuja_marcador_ascii(ascii);
				dibuja_marc_ancho((vcuad.col+1)*8,
				  (vcuad.fil+1)*16,ascii);
				ascii++;
				imp_ascii_car();
				dibuja_marcador_ascii(ascii);
				dibuja_marc_ancho((vcuad.col+1)*8,
				  (vcuad.fil+1)*16,ascii);
				copia_def_car(tabla_ascii+(ascii*altura_cuad),
				  caracter);
				dibuja_en_cuadricula();
				dibuja_caracter();
			}
		}
		else if(cuad_x<7) {
			dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
			  (vcuad.fil+1)*16,0);
			cuad_x++;
			dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
			  (vcuad.fil+1)*16,1);
		}
	}
	else if(tecla==T_ARR) {
		/* si est  pulsado SHIFT mueve marcador tabla ASCII */
		if(bioskey(2) & 0x0003) {
			if(ascii>15) {
				dibuja_marcador_ascii(ascii);
				dibuja_marc_ancho((vcuad.col+1)*8,
				  (vcuad.fil+1)*16,ascii);
				ascii-=16;
				imp_ascii_car();
				dibuja_marcador_ascii(ascii);
				dibuja_marc_ancho((vcuad.col+1)*8,
				  (vcuad.fil+1)*16,ascii);
				copia_def_car(tabla_ascii+(ascii*altura_cuad),
				  caracter);
				dibuja_en_cuadricula();
				dibuja_caracter();
			}
		}
		else if(cuad_y>0) {
			dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
			  (vcuad.fil+1)*16,0);
			cuad_y--;
			dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
			  (vcuad.fil+1)*16,1);
		}
	}
	else if(tecla==T_ABJ) {
		/* si est  pulsado SHIFT mueve marcador tabla ASCII */
		if(bioskey(2) & 0x0003) {
			if(ascii<240) {
				dibuja_marcador_ascii(ascii);
				dibuja_marc_ancho((vcuad.col+1)*8,
				  (vcuad.fil+1)*16,ascii);
				ascii+=16;
				imp_ascii_car();
				dibuja_marcador_ascii(ascii);
				dibuja_marc_ancho((vcuad.col+1)*8,
				  (vcuad.fil+1)*16,ascii);
				copia_def_car(tabla_ascii+(ascii*altura_cuad),
				  caracter);
				dibuja_en_cuadricula();
				dibuja_caracter();
			}
		}
		else if(cuad_y<(altura_cuad-1)) {
			dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
			  (vcuad.fil+1)*16,0);
			cuad_y++;
			dibuja_marc_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
			  (vcuad.fil+1)*16,1);
		}
	}
	else if(tecla==T_CTRLIZQ) dec_ancho();
	else if(tecla==T_CTRLDER) inc_ancho();
	else if(tecla==T_ESP) {
		cambia_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
		  (vcuad.fil+1)*16,1);
		cambia_caracter(caracter,cuad_x,cuad_y,1);
		dibuja_caracter();
	}
	else if(tecla==T_BORR) {
		cambia_cuadricula(cuad_x,cuad_y,(vcuad.col+1)*8,
		  (vcuad.fil+1)*16,0);
		cambia_caracter(caracter,cuad_x,cuad_y,0);
		dibuja_caracter();
	}
	else if(tecla==T_F1) almacena_definicion();
	else if(tecla==T_F2) borra_rejilla();
	else if(tecla==T_F3) llena_rejilla();
	else if(tecla==T_F4) scroll_def_arr();
	else if(tecla==T_F5) scroll_def_izq();
	else if(tecla==T_F6) scroll_def_der();
	else if(tecla==T_F7) scroll_def_abj();
	else if(tecla==T_F8) espejo_def();
	else if(tecla==T_F9) gira180();
	else if(tecla==T_F10) invertir_def();
	else if(tecla==T_SF1) rayado_horizontal();
	else if(tecla==T_SF2) rayado_vertical();
	else if(tecla==T_SF3) cursiva_izquierda();
	else if(tecla==T_SF4) cursiva_derecha();
	else if(tecla==T_SF5) negrita();
	/* cuadro de botones */
	else if(r.boton1) {
		i=bg_pulsado(&cbot,r.x,r.y);
		switch(i) {
			case 0 :
				almacena_definicion();
				break;
			case 1 :
				borra_rejilla();
				break;
			case 2 :
				llena_rejilla();
				break;
			case 3 :
				scroll_def_arr();
				break;
			case 4 :
				scroll_def_izq();
				break;
			case 5 :
				scroll_def_der();
				break;
			case 6 :
				scroll_def_abj();
				break;
			case 7 :
				espejo_def();
				break;
			case 8 :
				gira180();
				break;
			case 9 :
				invertir_def();
				break;
			case 10 :
				rayado_horizontal();
				break;
			case 11 :
				rayado_vertical();
				break;
			case 12 :
				cursiva_izquierda();
				break;
			case 13 :
				cursiva_derecha();
				break;
			case 14 :
				negrita();
				break;
			case 15 :
				dec_ancho();
				break;
			case 16 :
				inc_ancho();
				break;
			case 17 :
				prueba_car();
				break;
		}

		/* espera hasta que suelte bot¢n del rat¢n */
		do {
			rg_estado(&r);
		} while(r.boton1);
	}
}

}
#pragma warn +par

#pragma warn -par
/****************************************************************************
	INT24_HND: rutina de manejo de errores cr¡ticos de hardware.
****************************************************************************/
int int24_hnd(int errval, int ax, int bp, int si)
{

hardretn(2);

return(2);
}
#pragma warn +par

/****************************************************************************
	CARGA_ICONOS: carga iconos.
	  Entrada:	'nf_icn' nombre del fichero con los iconos
			'nicn' n£mero de iconos
			'icn' tabla con datos de iconos
****************************************************************************/
void carga_iconos(char *nf_icn, int nicn, STC_ICONO icn[])
{
FILE *f_icn;
unsigned long tamgrf[100];
unsigned char *pgrf;
int i, ngrf;

if((f_icn=fopen(nf_icn,"rb"))==NULL) return;

/* carga n£mero de gr ficos */
if(fread(&ngrf,sizeof(int),1,f_icn)!=1) {
	fclose(f_icn);
	return;
}

if(nicn>ngrf) nicn=ngrf;

/* carga tama¤o de gr ficos */
for(i=0; i<ngrf; i++) {
	if(fread(&tamgrf[i],sizeof(unsigned long),1,f_icn)!=1) {
		fclose(f_icn);
		return;
	}
}

/* carga gr ficos de iconos */
for(i=0; i<nicn; i++) {
	pgrf=(unsigned char *)malloc((size_t)tamgrf[i]);
	if(pgrf==NULL) return;

	icn[i].icn=pgrf;

	if(fread(icn[i].icn,sizeof(unsigned char),(size_t)tamgrf[i],
	  f_icn)!=(size_t)tamgrf[i]) {
		free(icn[i].icn);
		icn[i].icn=NULL;
		fclose(f_icn);
		return;
	}
}

fclose(f_icn);

}

/****************************************************************************
	CUADRO_AVISO: presenta un cuadro con un mensaje de aviso.
	  Entrada:	'msg' mensaje a presentar
****************************************************************************/
void cuadro_aviso(char *msg)
{
STC_CUADROG cav;

cg_crea_cuadro(&cav," AVISO ",CG_CENT,CG_CENT,26,9,AVISO_CF,AVISO_CPP,
  AVISO_CS1,AVISO_CS2,AVISO_CF,AVISO_CT);
cg_crea_elemento(&cav,CG_ELEM_BOTON,6,8,"^Vale",8);
cg_crea_elemento(&cav,CG_ELEM_TEXTO,0,0,msg,24,4,CG_TXTLINEA,CG_TXTBORDE);

cg_abre(&cav);
cg_gestiona(&cav);
cg_cierra(&cav);
cg_elimina(&cav);

}

/****************************************************************************
	CUADRO_SIONO: presenta un cuadro de pregunta.
	  Entrada:	'msg' mensaje a imprimir
	  Salida:	1 si eligi¢ SI, 0 si eligi¢ NO, -1 si sali¢ del
			cuadro
****************************************************************************/
int cuadro_siono(char *msg)
{
STC_CUADROG csiono;
int i;

cg_crea_cuadro(&csiono," AVISO ",CG_CENT,CG_CENT,26,10,AVISO_CF,AVISO_CPP,
  AVISO_CS1,AVISO_CS2,AVISO_CF,AVISO_CT);
cg_crea_elemento(&csiono,CG_ELEM_BOTON,7,6,"^Si",6);
cg_crea_elemento(&csiono,CG_ELEM_BOTON,7,13,"^No",6);
cg_crea_elemento(&csiono,CG_ELEM_TEXTO,0,0,msg,24,5,CG_TXTLINEA,CG_TXTBORDE);

cg_abre(&csiono);
i=cg_gestiona(&csiono);
cg_cierra(&csiono);
cg_elimina(&csiono);

if(i==0) return(1);
else if(i==1) return(0);

return(-1);
}

/****************************************************************************
	DIBUJA_CUADRICULA: cuadricula la zona de dibujo.
	  Entrada:      'x', 'y' origen de la zona de dibujo
	  NOTA: las dimensiones de cada cuadro son CUAD_ANCHO y CUAD_ALTO
****************************************************************************/
void dibuja_cuadricula(int x, int y)
{
int i, j, x0, y0;

rg_puntero(RG_OCULTA);

g_rectangulo(x,y,x+(CUAD_ANCHO*8)-1,y+(CUAD_ALTO*altura_cuad)-1,COLOR_F_CUAD,
  G_NORM,1);

y0=y;
for(i=0; i<altura_cuad; i++, y0+=CUAD_ALTO) {
	x0=x;
	for(j=0; j<8; j++, x0+=CUAD_ANCHO) g_rectangulo(x0,y0,
	  x0+(CUAD_ANCHO-1),y0+(CUAD_ALTO-1),COLOR_B_CUAD,G_NORM,0);
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	DIBUJA_MARC_ANCHO: dibuja marcador de anchura.
	  Entrada:      'x', 'y' origen de la cuadr¡cula
			'c' c¢digo ASCII del car cter
****************************************************************************/
void dibuja_marc_ancho(int x, int y, int c)
{
int x0, y0, ancho;

ancho=car_anch(c);
x0=x+(ancho*CUAD_ANCHO)-1;
y0=y+(altura_cuad*CUAD_ALTO)-1;

rg_puntero(RG_OCULTA);
g_linea(x0,y,x0,y0,COLOR_A_CUAD,G_XOR);
g_linea(x0+1,y,x0+1,y0,COLOR_A_CUAD,G_XOR);
rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	DIBUJA_MARC_CUADRICULA: dibuja marcador en cuadr¡cula de dibujo.
	  Entrada:      'x', 'y' coordenadas del cuadro a modificar ([0..7] y
			[0..(altura_cuad-1)])
			'orgx', 'orgy' origen de la cuadr¡cula
			'modo' 1 dibuja, 0 borra
****************************************************************************/
void dibuja_marc_cuadricula(int x, int y, int orgx, int orgy, int modo)
{
int cuadx, cuady;

/* oculta el puntero durante el cambio de cuadro */
rg_puntero(RG_OCULTA);

dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);

/* comprueba que no se salga de los l¡mites */
x=(x>7) ? 7 : x;
y=(y>(altura_cuad-1)) ? altura_cuad-1 : y;

/* calcula origen del cuadro */
cuadx=orgx+(x*CUAD_ANCHO);
cuady=orgy+(y*CUAD_ALTO);

/* cambia estado de cuadro */
if(!modo) g_rectangulo(cuadx,cuady,cuadx+(CUAD_ANCHO-1),cuady+(CUAD_ALTO-1),
  COLOR_B_CUAD,G_NORM,0);
else g_rectangulo(cuadx,cuady,cuadx+(CUAD_ANCHO-1),cuady+(CUAD_ALTO-1),
  COLOR_M_CUAD,G_NORM,0);

dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);
rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	CAMBIA_CUADRICULA: cambia el estado de un cuadro de la cuadr¡cula.
	  Entrada:      'x', 'y' coordenadas del cuadro a modificar ([0..7] y
			[0..(altura_cuad-1)])
			'orgx', 'orgy' origen de la cuadr¡cula
			'rellena' 0 vac¡a cuadro, 1 lo rellena
****************************************************************************/
void cambia_cuadricula(int x, int y, int orgx, int orgy, int rellena)
{
int cuadx, cuady;

/* oculta el puntero durante el cambio de cuadro */
rg_puntero(RG_OCULTA);

/* comprueba que no se salga de los l¡mites */
x=(x>7) ? 7 : x;
y=(y>(altura_cuad-1)) ? altura_cuad-1 : y;

/* calcula origen del cuadro */
cuadx=orgx+x*CUAD_ANCHO;
cuady=orgy+y*CUAD_ALTO;

/* cambia estado de cuadro */
if(!rellena) g_rectangulo(cuadx+1,cuady+1,cuadx+(CUAD_ANCHO-2),
  cuady+(CUAD_ALTO-2),COLOR_F_CUAD,G_NORM,1);
else g_rectangulo(cuadx+1,cuady+1,cuadx+(CUAD_ANCHO-2),cuady+(CUAD_ALTO-2),
  COLOR_R_CUAD,G_NORM,1);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	IMP_CHRDEF: imprime la definici¢n de un car cter.
	  Entrada:      'pchr' puntero a definici¢n de car cter a imprimir
			'x', 'y' coordenadas donde se imprimir 
			'colorf', 'color' colores de fondo y primer plano
			'alt' altura de la definici¢n del car cter
			'modo' modo de dibujo (C_NORM, C_XOR, C_OR, C_AND)

	  NOTA: esta rutina s¢lo sirve para los modos de 16 colores
****************************************************************************/
void imp_chrdef(BYTE far *pchr, int x, int y, int colorf, int color, int alt,
  BYTE modo)
{
BYTE far *video;
int varshift;

video=(BYTE far *)0xa0000000L+((x/8)+(y*80));

asm {
	push di
	push si
	push ds

	mov cx,x                // CL = byte bajo de coordenada X
	and cl,7
	xor cl,7                // CL = n§ de bits a desplazar izq.
	inc cx
	and cl,7                // CL = n§ bits a despl. para enmascarar
	mov ch,0ffh
	shl ch,cl               // CH = m scara lado derecho de car cter
	mov varshift,cx

	les di,pchr             // ES:DI = dir. definici¢n de car cter
	lds si,video            // DS:SI = dir. en buffer de v¡deo
	mov cx,alt              // CX = altura en pixels del car cter

	// establecer registros del controlador de gr ficos

	mov dx,3ceh             // puerto del registro de direcciones
	mov ax,0a05h            // modo escritura 2, modo lectura 1
	out dx,ax
	mov ah,modo             // modo de dibujo: 18h = XOR, 10h = OR,
				// 08h = AND, 00h = NORM
	mov al,3
	out dx,ax
	mov ax,0007h            // color "don't care" todos los planos
	out dx,ax

	mov bl,byte ptr color   // BL = valor de pixel imagen
	mov bh,byte ptr colorf  // BH = valor de pixel fondo

	cmp byte ptr varshift,0
	jne chr_no_alin         // salta si car cter no alineado

	// rutina para caracteres alineados

	mov al,8                // AL = n§ registro de m scara de bit
}
chr_alin:
asm {
	mov ah,es:[di]          // AH = patr¢n de fila de pixels
	out dx,ax               // lo carga en registro m scara de bit
	and [si],bl             // actualiza pixels de imagen
	not ah
	out dx,ax
	and [si],bh             // actualiza pixels de fondo

	inc di                  // siguiente byte del car cter
	add si,80               // siguiente l¡nea de pantalla

	loop chr_alin
	jmp chr_fin

	// rutina para caracteres no alineados
}
chr_no_alin:
asm {
	push cx                 // guarda contador n§ de l¡neas de car.
	mov cx,varshift         // CH = m scara lado derecho
				// CL = n§ de bits a desplazar
	// dibuja lado izquierdo del car cter
	mov al,es:[di]          // AL = patr¢n de fila de pixels
	xor ah,ah
	shl ax,cl               // AH = patr¢n para lado izquierdo
				// AL = patr¢n para lado derecho
	push ax                 // guarda patrones
	mov al,8                // n§ de registro de m scara de bit
	out dx,ax               // carga patr¢n de lado izquierdo
	and [si],bl             // actualiza pixels de imagen
	not ch                  // CH = m scara para lado izquierdo
	xor ah,ch
	out dx,ax
	and [si],bh             // actualiza pixels de fondo
	// dibuja lado derecho del car cter
	pop ax                  // recupera patrones
	mov ah,al               // AH = patr¢n lado derecho
	mov al,8                // AL = n§ registro de m scara de bit
	out dx,ax               // carga patr¢n
	inc si                  // posici¢n en buffer de v¡deo
	and [si],bl             // actualiza pixels de imagen
	not ch                  // CH = m scara para lado derecho
	xor ah,ch
	out dx,ax
	and [si],bh             // actualiza pixels de fondo

	inc di                  // siguiente byte del car cter
	dec si
	add si,80               // siguiente l¡nea de pantalla
	pop cx
	loop chr_no_alin
}
chr_fin:
asm {
	// restaura registros de controlador a sus valores por defecto
	mov ax,0ff08h           // m scara de bit
	out dx,ax
	mov ax,0005h            // registro de modo
	out dx,ax
	mov ax,0003h            // rotar dato/selecc. funci¢n
	out dx,ax
	mov ax,0f07h            // color "don't care"
	out dx,ax

	pop ds
	pop si
	pop di
}

}

/****************************************************************************
	DIBUJA_TABLA_ASCII: dibuja la tabla de caracteres ASCII en forma
	  de matriz de 16x16.
	  Entrada:      'x', 'y' posici¢n de inicio de la tabla
****************************************************************************/
void dibuja_tabla_ascii(int x, int y)
{
BYTE espacio[16];
int i, j, x1, car=0;

/* esconde puntero mientras dibuja tabla */
rg_puntero(RG_OCULTA);

/* crea una definici¢n de espacio */
for(i=0; i<16; i++) espacio[i]=0;

for(i=0; i<16; i++) {
	/* imprime espacio al inicio de cada l¡nea */
	imp_chrdef(&espacio[0],x,y,0,COLOR_L_ASCII,altura_cuad,C_NORM);
	x1=x+8;

	for(j=0; j<16; j++, car++, x1+=8) {
		imp_chrdef(tabla_ascii+(car*altura_cuad),x1,y,0,COLOR_L_ASCII,
		  altura_cuad,C_NORM);
		x1+=8;
		imp_chrdef(&espacio[0],x1,y,0,COLOR_L_ASCII,altura_cuad,C_NORM);
	}

	y+=altura_cuad;
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	CAMBIA_CARACTER: cambia el estado de un bit de la definici¢n
	  de un car cter.
	  Entrada:      'car' puntero a definici¢n del car cter
			'x', 'y' coordenadas del bit a cambiar
			'bit' estado del bit (0 o 1)
****************************************************************************/
void cambia_caracter(BYTE *car, int x, int y, int bit)
{
BYTE mascara;

mascara=0x80 >> x;
if(bit) car[y] |= mascara;
else {
	mascara ^= 0xff;
	car[y] &= mascara;
}

}

/****************************************************************************
	LIMPIA_CARACTER: rellena la definici¢n de un car cter con un BYTE
	  dado.
	  Entrada:      'car' puntero a definici¢n del car cter
			'byte' byte de relleno
****************************************************************************/
void limpia_caracter(BYTE *car, BYTE byte)
{
int i;

for(i=0; i<altura_cuad; car[i++]=byte);

}

/****************************************************************************
	PULSA_EN_ASCII: devuelve el c¢digo ASCII que corresponde a un
	  car cter situado en la tabla ASCII 16x16.
	  Entrada:      'x', 'y' posici¢n relativa al origen de la tabla
	  Salida:       c¢digo ASCII del car cter cuya posici¢n coincide
			con las coordenadas 'x', 'y'
****************************************************************************/
int pulsa_en_ascii(int x, int y)
{
int x0, y0;

/* transforma en valor 0..15 */
x0=x/16;
y0=y/altura_cuad;

/* comprueba que no est‚ fuera de los l¡mites */
x0=(x0>15) ? 15 : x0;
y0=(y0>15) ? 15 : y0;

return(y0*16+x0);
}

/****************************************************************************
	DIBUJA_MARCADOR_ASCII: dibuja un marcador alrededor de un car cter
	  de la tabla ASCII.
	  Entrada:      'ascii' c¢digo ASCII del car cter
****************************************************************************/
void dibuja_marcador_ascii(int ascii)
{
int orgx, orgy, x, y;

/* origen de la tabla */
orgx=(vtabla.col+1)*8;
orgy=(vtabla.fil+1)*16;

/* coordenadas relativas de un car cter dentro de la tabla 16x16 en */
/* funci¢n de su c¢digo ASCII */
x=(((ascii%16)*2)+1)*8;
y=(ascii/16)*altura_cuad;

/* desactiva puntero mientras dibuja */
rg_puntero(RG_OCULTA);

/* dibuja un rect ngulo alrededor del car cter seleccionado */
g_rectangulo(orgx+x-1,orgy+y-1,orgx+x+8,orgy+y+altura_cuad,COLOR_R_ASCII,
  G_XOR,0);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	COPIA_TABLA_ROM: copia las definiciones de una tabla de caracteres
	  de la ROM en memoria RAM.
	  Entrada:      'tabla' n£mero de la tabla de la ROM
			'tabla_car' puntero a tabla d¢nde se copiar n las
			definiciones de los caracteres
			'alt' altura de caracteres a copiar
****************************************************************************/
void copia_tabla_rom(BYTE tabla, BYTE far *tabla_car, int alt)
{
BYTE far *dir_tabla_rom;
int i;

asm {
	push bp
	mov ah,11h              // interfaz del generador de caracteres
	mov al,30h              // obtener informaci¢n del generador
	mov bh,tabla
	int 10h                 // ES:BP=direcci¢n de tabla en ROM
	mov ax,bp
	pop bp
	mov word ptr (dir_tabla_rom+2),es
	mov word ptr dir_tabla_rom,ax
}

for(i=0; i<256*alt; i++) tabla_car[i]=*dir_tabla_rom++;

}

/****************************************************************************
	COPIA_DEF_CAR: copia la definici¢n de un car cter de un buffer
	  a otro.
	  Entrada:      'origen' puntero a la definici¢n de car cter a
			copiar
			'destino' puntero a d¢nde copiarla
****************************************************************************/
void copia_def_car(BYTE far *origen, BYTE far *destino)
{
int i;

for(i=0; i<altura_cuad; i++) destino[i]=origen[i];

}

/****************************************************************************
	DIBUJA_EN_CUADRICULA: dibuja la definici¢n del car cter actual en la
	  cuadr¡cula de dibujo.
****************************************************************************/
void dibuja_en_cuadricula(void)
{
int i, j, x, y;
BYTE c, mascara;

/* esconde puntero mientras dibuja */
rg_puntero(RG_OCULTA);

y=(vcuad.fil+1)*16;
for(i=0; i<altura_cuad; i++, y+=CUAD_ALTO) {
	c=caracter[i];
	mascara=0x80;
	x=(vcuad.col+1)*8;
	/* dibuja bit por bit */
	for(j=0; j<8; j++, mascara>>=1, x+=CUAD_ANCHO) {
		/* si el bit est  activo dibuja de color de relleno */
		/* si no de color de fondo */
		if(mascara & c) g_rectangulo(x+1,y+1,x+(CUAD_ANCHO-2),
		  y+(CUAD_ALTO-2),COLOR_R_CUAD,G_NORM,1);
		else g_rectangulo(x+1,y+1,x+(CUAD_ANCHO-2),y+(CUAD_ALTO-2),
		  COLOR_F_CUAD,G_NORM,1);
	}
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	ACTUALIZA_TABLA: dibuja una definici¢n de car cter en su posici¢n
	  en la tabla ASCII de pantalla.
	  Entrada:      'x', 'y' origen de la tabla
			'ascii' c¢digo ASCII del car cter
****************************************************************************/
void actualiza_tabla(int x, int y, int ascii)
{
int xcar, ycar;

/* oculta puntero mientras actualiza tabla ASCII */
rg_puntero(RG_OCULTA);

/* calcula posici¢n del car cter de acuerdo a su c¢digo ASCII */
/* teniendo en cuenta que hay un espacio entre cada columna de la tabla */
ycar=(ascii/16)*altura_cuad;
xcar=(((ascii%16)*2)+1)*8;

/* imprime car cter en su posici¢n dentro de la tabla */
imp_chrdef(tabla_ascii+(ascii*altura_cuad),x+xcar,y+ycar,0,COLOR_L_ASCII,
  altura_cuad,C_NORM);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	DIBUJA_CARACTER: dibuja la definici¢n de un car cter a tama¤o real.
****************************************************************************/
void dibuja_caracter(void)
{

/* oculta el puntero mientras dibuja car cter */
rg_puntero(RG_OCULTA);

g_linea(DEFCAR_X,DEFCAR_Y,DEFCAR_X+DEFCAR_ANCHO-1,DEFCAR_Y,COLOR_S2,G_NORM);
g_linea(DEFCAR_X,DEFCAR_Y,DEFCAR_X,DEFCAR_Y+DEFCAR_ALTO-1,COLOR_S2,G_NORM);
g_linea(DEFCAR_X+1,DEFCAR_Y+DEFCAR_ALTO-1,DEFCAR_X+DEFCAR_ANCHO-1,
  DEFCAR_Y+DEFCAR_ALTO-1,COLOR_S1,G_NORM);
g_linea(DEFCAR_X+DEFCAR_ANCHO-1,DEFCAR_Y+1,DEFCAR_X+DEFCAR_ANCHO-1,
  DEFCAR_Y+DEFCAR_ALTO-1,COLOR_S1,G_NORM);

imp_chrdef(caracter,DEFCAR_X+((DEFCAR_ANCHO-8)/2),
  DEFCAR_Y+((DEFCAR_ALTO-altura_cuad)/2),0,COLOR_L_ASCII,altura_cuad,C_NORM);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	GRABA_DEF: graba en un fichero las definiciones de los caracteres.
	  Entrada:      'nombre' nombre con que se grabar  la fuente
	  Salida:       0 si hubo error o un valor distinto de 0 en
			otro caso
****************************************************************************/
int graba_def(char *nombre)
{
FILE *ffuente;
size_t num_bytes;

/* abre el fichero para escritura en modo binario */
ffuente=fopen(nombre,"wb");
/* sale si hubo error */
if(ffuente==NULL) return(0);

/* escribe cadena de reconocimiento */
if(fwrite(RECON_FUENTE,sizeof(char),LONG_RECON_F+1,ffuente)<LONG_RECON_F+1) {
	fclose(ffuente);
	return(0);
}

/* escribe anchuras de caracteres 8x16 */
if(fwrite(tabla_anch8x16,sizeof(BYTE),256,ffuente)<256) {
	fclose(ffuente);
	return(0);
}

/* escribe definiciones de caracteres 8x16 */
num_bytes=(size_t)(256*16);
if(fwrite(tabla_ascii8x16,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

/* escribe anchuras de caracteres 8x8 */
if(fwrite(tabla_anch8x8,sizeof(BYTE),256,ffuente)<256) {
	fclose(ffuente);
	return(0);
}

/* escribe definiciones de caracteres 8x8 */
num_bytes=(size_t)(256*8);
if(fwrite(tabla_ascii8x8,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

car_modificado=0;

return(1);
}

/****************************************************************************
	CARGA_DEF: carga de un fichero las definiciones de los
	  caracteres.
	  Entrada:      'nombre' nombre del fichero
	  Salida:       0 si hubo error o un valor distinto de 0 en
			otro caso
****************************************************************************/
int carga_def(char *nombre)
{
FILE *ffuente;
char cad_recon[LONG_RECON_F+1];
char *recon_fuente=RECON_FUENTE;
size_t num_bytes;
int i, version=0;

/* abre el fichero para lectura */
ffuente=fopen(nombre,"rb");
/* sale si hubo error */
if(ffuente==NULL) return(0);

/* lee cadena de reconocimiento */
if(fread(cad_recon,sizeof(char),LONG_RECON_F+1,ffuente)<LONG_RECON_F+1) {
	fclose(ffuente);
	return(0);
}

/* comprueba la versi¢n del fichero */
/* si es versi¢n 2 lo convierte */
if(cad_recon[LONG_RECON_F-1]=='2') {
	i=cuadro_siono("Formato antiguo fuente.\n     ¨Convertirla?");
	if((i==-1) || !i) return(1);
	else version=1;
}
else if(cad_recon[LONG_RECON_F-1]!=recon_fuente[LONG_RECON_F-1]) {
	fclose(ffuente);
	return(0);
}

/* si la versi¢n ha sido v lida lee las definiciones de los caracteres */
/* anchuras 8x16 */
if(!version) {
	if(fread(tabla_anch8x16,sizeof(BYTE),256,ffuente)<256) {
		fclose(ffuente);
		return(0);
	}
}
else for(i=0; i<256; i++) tabla_anch8x16[i]=8;

/* definiciones 8x16 */
num_bytes=(size_t)(256*16);
if(fread(tabla_ascii8x16,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

/* anchuras 8x8 */
if(!version) {
	if(fread(tabla_anch8x8,sizeof(BYTE),256,ffuente)<256) {
		fclose(ffuente);
		return(0);
	}
}
else for(i=0; i<256; i++) tabla_anch8x8[i]=8;

/* definiciones 8x8 */
num_bytes=(size_t)(256*8);
if(fread(tabla_ascii8x8,sizeof(BYTE),num_bytes,ffuente)<num_bytes) {
	fclose(ffuente);
	return(0);
}

fclose(ffuente);

car_modificado=0;

return(1);
}

/****************************************************************************
	CUADRICULA: rutina de manejo de la cuadr¡cula de dibujo.
	  Entrada:	'x', 'y' coordenadas relativas al interior de la
			cuadr¡cula de dibujo
****************************************************************************/
void cuadricula(int x, int y)
{
STC_RATONG r;
static int ant_cuadrox=-1, ant_cuadroy=-1, ant_boton=-1;
int cuadrox, cuadroy;

rg_estado(&r);

cuadrox=x/CUAD_ANCHO;
cuadroy=y/CUAD_ALTO;

/* si est  pulsado el bot¢n izquierdo del rat¢n, rellena */
if(r.boton1 && ((cuadrox!=ant_cuadrox) || (cuadroy!=ant_cuadroy) ||
  (ant_boton!=1))) {
	cambia_cuadricula(cuadrox,cuadroy,(vcuad.col+1)*8,(vcuad.fil+1)*16,1);
	cambia_caracter(caracter,cuadrox,cuadroy,1);
	dibuja_caracter();
	ant_cuadrox=cuadrox;
	ant_cuadroy=cuadroy;
	ant_boton=1;
}
/* si est  pulsado el bot¢n derecho del rat¢n, vac¡a */
else if(r.boton2 && ((cuadrox!=ant_cuadrox) || (cuadroy!=ant_cuadroy) ||
  (ant_boton!=2))) {
	cambia_cuadricula(cuadrox,cuadroy,(vcuad.col+1)*8,(vcuad.fil+1)*16,0);
	cambia_caracter(caracter,cuadrox,cuadroy,0);
	dibuja_caracter();
	ant_cuadrox=cuadrox;
	ant_cuadroy=cuadroy;
	ant_boton=2;
}

}

/****************************************************************************
	TAB_ASCII: rutina de manejo de la tabla ASCII.
	  Entrada:	'x', 'y' coordenadas relativas al interior de la
			tabla
****************************************************************************/
void tab_ascii(int x, int y)
{

/* borra marcador actual */
dibuja_marcador_ascii(ascii);
dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);

/* coge c¢digo ASCII */
ascii=pulsa_en_ascii(x,y);

/* escribe c¢digo ASCII del car cter actual */
imp_ascii_car();

/* dibuja marcador */
dibuja_marcador_ascii(ascii);
dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);

/* copia definici¢n de caracter en buffer auxiliar */
copia_def_car(tabla_ascii+(ascii*altura_cuad),caracter);

/* dibuja en cuadr¡cula y a tama¤o real */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	DEF_CARACTER: rutina de manejo de pulsaci¢n en cuadro con
	  la definici¢n del car cter a tama¤o real.
****************************************************************************/
void def_caracter(void)
{
STC_RATONG r;
int asc;

/* marca recuadro de bot¢n para indicar que ha sido pulsado */
rg_puntero(RG_OCULTA);
g_linea(DEFCAR_X-1,DEFCAR_Y-1,DEFCAR_X+DEFCAR_ANCHO,DEFCAR_Y-1,COLOR_S2,
  G_NORM);
g_linea(DEFCAR_X-1,DEFCAR_Y-1,DEFCAR_X-1,DEFCAR_Y+DEFCAR_ALTO,COLOR_S2,G_NORM);
g_linea(DEFCAR_X,DEFCAR_Y+20,DEFCAR_X+DEFCAR_ANCHO,DEFCAR_Y+DEFCAR_ALTO,
  COLOR_S1,G_NORM);
g_linea(DEFCAR_X+DEFCAR_ANCHO,DEFCAR_Y,DEFCAR_X+DEFCAR_ANCHO,
  DEFCAR_Y+DEFCAR_ALTO,COLOR_S1,G_NORM);
rg_puntero(RG_MUESTRA);

/* espera a que se suelte el bot¢n del rat¢n */
do {
	rg_estado(&r);
} while(r.boton1);

/* espera hasta que pulse en otro lugar */
do {
	rg_estado(&r);
} while(!r.boton1);

/* restaura recuadro de bot¢n */
rg_puntero(RG_OCULTA);
g_rectangulo(DEFCAR_X-1,DEFCAR_Y-1,DEFCAR_X+DEFCAR_ANCHO,DEFCAR_Y+DEFCAR_ALTO,
  COLOR_FONDO,G_NORM,0);
rg_puntero(RG_MUESTRA);

/* si pulsa en tabla ASCII pasa la definici¢n actual a la */
/* posici¢n de la tabla d¢nde puls¢ */
if((r.fil>=(vtabla.fil+1)) && (r.fil<=(vtabla.fil+vtabla.alto-2)) &&
  (r.col>=(vtabla.col+1)) && (r.col<=(vtabla.col+vtabla.ancho-2))) {
	asc=pulsa_en_ascii(r.x-((vtabla.col+1)*8),r.y-((vtabla.fil+1)*16));

	/* copia definici¢n de car cter en tabla ASCII */
	copia_def_car(caracter,tabla_ascii+(asc*altura_cuad));

	/* reimprime en pantalla */
	actualiza_tabla((vtabla.col+1)*8,(vtabla.fil+1)*16,asc);

	car_modificado=1;
}

}

/****************************************************************************
	ALMACENA_DEFINICION: almacena la definici¢n actual del car cter
	  en la posici¢n actual de la tabla ASCII.
****************************************************************************/
void almacena_definicion(void)
{

/* copia la definici¢n actual en la posici¢n actual de la tabla ASCII */
copia_def_car(caracter,tabla_ascii+(ascii*altura_cuad));

/* y actualiza tabla en pantalla */
actualiza_tabla((vtabla.col+1)*8,(vtabla.fil+1)*16,ascii);

car_modificado=1;

}

/****************************************************************************
	BORRA_REJILLA: borra la rejilla de dibujo y la definici¢n actual de
	  car cter.
****************************************************************************/
void borra_rejilla(void)
{

/* limpia definici¢n del car cter usando 0 */
limpia_caracter(caracter,0);

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	LLENA_REJILLA: llena la rejilla de dibujo y la definici¢n actual de
	  car cter.
****************************************************************************/
void llena_rejilla(void)
{

/* limpia definici¢n del car cter usando 0xFF */
limpia_caracter(caracter,0xff);

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	SCROLL_DEF_ARR: desplaza la definici¢n del car cter una l¡nea
	  hacia arriba, la l¡nea que sale por arriba aparece por abajo.
****************************************************************************/
void scroll_def_arr(void)
{
BYTE c;
int i;

/* guarda la primera l¡nea */
c=caracter[0];

/* desplaza la definici¢n 1 l¡nea hacia arriba */
for(i=1; i<altura_cuad; caracter[i-1]=caracter[i], i++);

/* pone la primera l¡nea en la £ltima */
caracter[altura_cuad-1]=c;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	SCROLL_DEF_ABJ: desplaza la definici¢n del car cter una l¡nea
	  hacia abajo, la l¡nea que sale por abajo aparece por arriba.
****************************************************************************/
void scroll_def_abj(void)
{
BYTE c;
int i;

/* guarda la primera l¡nea */
c=caracter[altura_cuad-1];

/* desplaza la definici¢n 1 l¡nea hacia abajo */
for(i=altura_cuad-1; i>0; caracter[i]=caracter[i-1], i--);

/* pone la £ltima l¡nea en la primera */
caracter[0]=c;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	SCROLL_DEF_IZQ: desplaza la definici¢n del car cter una columna
	  hacia la izquierda, la columna que sale por la izquierda aparece
	  por la derecha.
****************************************************************************/
void scroll_def_izq(void)
{
BYTE c;
int i;

for(i=0; i<altura_cuad; i++) {
	/* coge el £ltimo bit y lo guarda como primer bit */
	c=(BYTE)((caracter[i] & 0x80) >> 7);
	/* desplaza hacia izquierda y mete £ltimo bit en primero */
	caracter[i]=(BYTE)((caracter[i] << 1) | c);
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	SCROLL_DEF_DER: desplaza la definici¢n del car cter una columna
	  hacia la derecha, la columna que sale por la derecha aparece
	  por la izquierda.
****************************************************************************/
void scroll_def_der(void)
{
BYTE c;
int i;

for(i=0; i<altura_cuad; i++) {
	/* coge el primer bit y lo guarda como £ltimo bit */
	c=(BYTE)((caracter[i] & 0x01) << 7);
	/* desplaza hacia derecha y mete primer bit en £ltimo */
	caracter[i]=(BYTE)((caracter[i] >> 1) | c);
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	INVERTIR_DEF: invierte la definici¢n del car cter de la cuadr¡-
	  cula; pone a 1 los bits 0 y a 0 los bits 1.
****************************************************************************/
void invertir_def(void)
{
int i;

for(i=0; i<altura_cuad; i++) caracter[i]=(BYTE)~caracter[i];

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	GIRA180: gira el car cter 180 grados.
****************************************************************************/
void gira180(void)
{
BYTE c;
int i;

for(i=0; i<(altura_cuad/2); i++) {
	/* intercambia caracteres */
	c=caracter[(altura_cuad-1)-i];
	caracter[(altura_cuad-1)-i]=caracter[i];
	caracter[i]=c;
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	ESPEJO_DEF: efecto espejo.
****************************************************************************/
void espejo_def(void)
{
BYTE c, m1, m2;
int i, j;

for(i=0; i<altura_cuad; i++) {
	c=0;
	m1=0x01;
	m2=0x80;
	/* refleja byte */
	for(j=0; j<8; j++) {
		if(caracter[i] & m1) c |= m2;
		m1 <<= 1;
		m2 >>= 1;
	}
	/* almacena byte reflejado */
	caracter[i]=c;
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	CARGAR_FUENTE: rutina para cargar un fichero con definiciones de
	  caracteres.
****************************************************************************/
void cargar_fuente(void)
{
char nfich[MAXPATH];

cg_selecc_ficheros(CG_CENT,CG_CENT," Cargar fuente ",COLOR_FONDO,COLOR_PPLANO,
  COLOR_S1,COLOR_S2,COLOR_FONDO,COLOR_TECLA,"","*.*",
  nfich);

/* sale si no se eligi¢ ning£n fichero */
if(!*nfich) return;

if(!carga_def(nfich)) cuadro_aviso("  Fichero no v lido\n"
  "  o error en lectura.");

/* redibuja la tabla ASCII */
dibuja_marcador_ascii(ascii);
dibuja_tabla_ascii((vtabla.col+1)*8,(vtabla.fil+1)*16);
/* dibuja el marcador */
dibuja_marcador_ascii(ascii);

}

/****************************************************************************
	GRABAR_FUENTE: rutina para grabar un fichero con definiciones de
	  caracteres.
****************************************************************************/
void grabar_fuente(void)
{
char nfich[MAXPATH];
int i;

cg_selecc_ficheros(CG_CENT,CG_CENT," Grabar fuente ",COLOR_FONDO,COLOR_PPLANO,
  COLOR_S1,COLOR_S2,COLOR_FONDO,COLOR_TECLA,"","*.*",
  nfich);

/* sale si no se eligi¢ ning£n fichero */
if(!*nfich) return;

/* comprueba si existe y pide confirmaci¢n */
if(!access(nfich,0)) {
	i=cuadro_siono("Ese fichero ya existe.\n    ¨Reemplazarlo?");
	if((i==-1) || !i) return;
}

if(!graba_def(nfich)) cuadro_aviso("   Error de apertura\n"
  " o error en escritura.");

}

/****************************************************************************
	PASA_A8x16: pasa al modo de 8x16.
****************************************************************************/
void pasa_a8x16(void)
{

rg_puntero(RG_OCULTA);

altura_cuad=16;
tabla_ascii=&tabla_ascii8x16[0][0];

/* borra cuadr¡cula y tabla ASCII */
g_rectangulo(vcuad.col*8,vcuad.fil*16,(vcuad.col+vcuad.ancho)*8,
  (vcuad.fil+vcuad.alto)*16,COLOR_FONDO,G_NORM,1);
g_rectangulo(vtabla.col*8,vtabla.fil*16,(vtabla.col+vtabla.ancho)*8,
  (vtabla.fil+vtabla.alto)*16,COLOR_FONDO,G_NORM,1);

/* cambia altura de cuadr¡cula de dibujo y tabla ASCII */
vcuad.alto=ALT_C8X16;
vtabla.alto=ALT_T8X16;

/* cuadricula zona de dibujo */
vg_dibuja(&vcuad,1);
dibuja_cuadricula((vcuad.col+1)*8,(vcuad.fil+1)*16);

/* tabla ASCII */
vg_dibuja(&vtabla,1);
dibuja_tabla_ascii((vtabla.col+1)*8,(vtabla.fil+1)*16);

/* dibuja marcador en el car cter actual de la tabla */
dibuja_marcador_ascii(ascii);

/* copia definici¢n de caracter en buffer auxiliar */
copia_def_car(tabla_ascii+(ascii*altura_cuad),caracter);

/* dibuja car cter a tama¤o real */
g_rectangulo(DEFCAR_X,DEFCAR_Y,DEFCAR_X+DEFCAR_ANCHO-1,DEFCAR_Y+DEFCAR_ALTO-1,
  0,G_NORM,1);
dibuja_caracter();

/* dibuja car cter en cuadr¡cula */
dibuja_en_cuadricula();

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	PASA_A8x8: pasa al modo de 8x8.
****************************************************************************/
void pasa_a8x8(void)
{

rg_puntero(RG_OCULTA);

altura_cuad=8;
tabla_ascii=&tabla_ascii8x8[0][0];

/* borra cuadr¡cula y tabla ASCII */
g_rectangulo(vcuad.col*8,vcuad.fil*16,(vcuad.col+vcuad.ancho)*8,
  (vcuad.fil+vcuad.alto)*16,COLOR_FONDO,G_NORM,1);
g_rectangulo(vtabla.col*8,vtabla.fil*16,(vtabla.col+vtabla.ancho)*8,
  (vtabla.fil+vtabla.alto)*16,COLOR_FONDO,G_NORM,1);

/* cambia altura de cuadr¡cula de dibujo y tabla ascii */
vcuad.alto=ALT_C8X8;
vtabla.alto=ALT_T8X8;

/* cuadricula zona de dibujo */
vg_dibuja(&vcuad,1);
dibuja_cuadricula((vcuad.col+1)*8,(vcuad.fil+1)*16);

/* tabla ASCII */
vg_dibuja(&vtabla,1);
dibuja_tabla_ascii((vtabla.col+1)*8,(vtabla.fil+1)*16);

/* dibuja marcador en el car cter actual de la tabla */
dibuja_marcador_ascii(ascii);

/* copia definici¢n de caracter en buffer auxiliar */
copia_def_car(tabla_ascii+(ascii*altura_cuad),caracter);

/* dibuja car cter a tama¤o real */
g_rectangulo(DEFCAR_X,DEFCAR_Y,DEFCAR_X+DEFCAR_ANCHO-1,DEFCAR_Y+DEFCAR_ALTO-1,
  0,G_NORM,1);
dibuja_caracter();

/* dibuja car cter en cuadr¡cula */
dibuja_en_cuadricula();

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	PRUEBA_CAR: abre una ventana para comprobar juego de caracteres.
****************************************************************************/
void prueba_car(void)
{
STC_VENTANAG v;
BYTE espacio[16];
int i, tecla, xcar, ycar, ncar=0, car[NPRB_CAR];

/* crea una definici¢n de espacio */
for(i=0; i<16; i++) espacio[i]=0;

rg_puntero(RG_OCULTA);

vg_crea(&v,15,(80-(NPRB_CAR+2))/2,NPRB_CAR+2,6,COLOR_FONDO,COLOR_PPLANO,
  COLOR_S1,COLOR_S2," Prueba de caracteres ");
vg_abre(&v);
vg_impcad(&v,"     Teclea los caracteres y cuando\n"
  "         termines pulsa RETURN.",VG_NORELLENA);

/* zona de prueba */
g_rectangulo((v.col+1)*8,(v.fil+3)*16,(v.col+NPRB_CAR+1)*8,(v.fil+5)*16,
  COLOR_F_PRB,G_NORM,1);

xcar=(v.col+1)*8;
ycar=((v.fil+3)*16)+((32-altura_cuad)/2);

do {
	/* imprime cursor */
	g_linea(xcar,ycar+altura_cuad-1,xcar+7,ycar+altura_cuad-1,COLOR_P_PRB,
	  G_NORM);

	/* coge c¢digo ASCII de tecla pulsada */
	tecla=bioskey(0) & 0x00ff;

	/* si pulsa borrado */
	if(tecla==0x08) {
		/* retrocede una posici¢n si no est  al principio */
		/* e imprime un espacio */
		if(ncar) {
			ncar--;
			xcar-=car_anch(car[ncar]);
			/* borra car cter y cursor */
			imp_chrdef(&espacio[0],xcar,ycar,COLOR_F_PRB,
			  COLOR_P_PRB,altura_cuad,C_NORM);
			imp_chrdef(&espacio[0],xcar+car_anch(car[ncar]),
			  ycar,COLOR_F_PRB,COLOR_P_PRB,altura_cuad,C_NORM);
		}
	}
	else if((ncar<(NPRB_CAR-1)) && (tecla!=0x0d)) {
		/* imprime car cter */
		imp_chrdef(tabla_ascii+(tecla*altura_cuad),xcar,ycar,
		  COLOR_F_PRB,COLOR_P_PRB,altura_cuad,C_NORM);
		car[ncar]=tecla;
		ncar++;
		xcar+=car_anch(tecla);
	}
} while(tecla!=0x0d);

vg_cierra(&v);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	IMP_ASCII_CAR: imprime el c¢digo ASCII del car cter actual.
****************************************************************************/
void imp_ascii_car(void)
{
char cascii[4];

sprintf(cascii,"%03d",ascii);
vg_pon_cursor(&vinfo,0,0);
vg_impcad(&vinfo,cascii,VG_NORELLENA);

}

/****************************************************************************
	TABLACAR_MODIFICADA: comprueba si la tabla de caracteres fue
	  modificada y si es as¡ pregunta si quiere grabarla.
****************************************************************************/
void tablacar_modificada(void)
{
int i;

if(car_modificado) {
	i=cuadro_siono("Juego de caracteres ha\n    sido modificado\n"
	  "      ¨Grabarlo?");
	if(i==1) grabar_fuente();
}

car_modificado=0;

}

/****************************************************************************
	RAYADO_HORIZONTAL: raya con franjas alternativas horizontales el
	  car cter actual.
****************************************************************************/
void rayado_horizontal(void)
{
int i;

for(i=0; i<altura_cuad; i+=2) caracter[i]=0;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	RAYADO_VERTICAL: raya con franjas alternativas verticales el
	  car cter actual.
****************************************************************************/
void rayado_vertical(void)
{
int i;

for(i=0; i<altura_cuad; i++) caracter[i] &= 0x55;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	CURSIVA_IZQUIERDA: crea forma cursiva inclinada hacia la izquierda
	  del car cter actual.
****************************************************************************/
void cursiva_izquierda(void)
{
int i;

for(i=0; i<(altura_cuad/2); i++) caracter[i] <<= 1;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	CURSIVA_DERECHA: crea forma cursiva inclinada hacia la derecha
	  del car cter actual.
****************************************************************************/
void cursiva_derecha(void)
{
int i;

for(i=0; i<(altura_cuad/2); i++) caracter[i] >>= 1;

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	NEGRITA: crea forma negrita del car cter actual.
****************************************************************************/
void negrita(void)
{
int i;
BYTE c;

for(i=0; i<altura_cuad; i++) {
	c=caracter[i] >> 1;
	caracter[i] |= c;
}

/* dibuja definici¢n en cuadr¡cula */
dibuja_en_cuadricula();
dibuja_caracter();

}

/****************************************************************************
	INC_ANCHO: incrementa ancho de car cter actual.
****************************************************************************/
void inc_ancho(void)
{
BYTE ancho;

ancho=car_anch(ascii);

if(ancho<8) {
	/* borra marcador de anchura */
	dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);
	ancho++;
	if(altura_cuad==16) tabla_anch8x16[ascii]=ancho;
	else tabla_anch8x8[ascii]=ancho;
	/* dibuja marcador de anchura */
	dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);
	car_modificado=1;
}

}

/****************************************************************************
	DEC_ANCHO: decrementa ancho de car cter actual.
****************************************************************************/
void dec_ancho(void)
{
BYTE ancho;

ancho=car_anch(ascii);

if(ancho>1) {
	/* borra marcador de anchura */
	dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);
	ancho--;
	if(altura_cuad==16) tabla_anch8x16[ascii]=ancho;
	else tabla_anch8x8[ascii]=ancho;
	/* dibuja marcador de anchura */
	dibuja_marc_ancho((vcuad.col+1)*8,(vcuad.fil+1)*16,ascii);
	car_modificado=1;
}

}

/****************************************************************************
	CAR_ANCH: devuelve la anchura de un car cter.
	  Entrada:	'c' car cter
	  Salida:	anchura del car cter
****************************************************************************/
int car_anch(int c)
{

if(altura_cuad==16) return(tabla_anch8x16[c]);
else return(tabla_anch8x8[c]);

}
