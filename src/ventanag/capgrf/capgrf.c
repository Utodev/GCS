/****************************************************************************
			    CAPTURADOR DE GRAFICOS
			     (c)1995 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <alloc.h>
#include <string.h>
#include <bios.h>
#include <dos.h>
#include "graf.h"
#include "ventanag.h"
#include "menug.h"
#include "cuadrog.h"
#include "capgrf.h"

/*** Variables globales ***/
/* colores */
unsigned char clr_fondo=7;
unsigned char clr_pplano=0;
unsigned char clr_s1=15;
unsigned char clr_s2=8;
unsigned char clr_tecla=4;
unsigned char clr_errfondo=8;
unsigned char clr_errpplano=14;
unsigned char clr_errs1=7;
unsigned char clr_errs2=0;
unsigned char clr_errtecla=12;

/* men£s de opciones */
STC_MENUG *m0, *m1, *m2;

/* nombre de £ltimo fichero PCX cargado */
char npcx[MAXPATH];

/* datos de gr ficos */
STC_GRF grf[MAX_GRF];
int num_grf=0;

/* indicador de si hay modificaciones que grabar */
int modificado=0;

/* l¡mites de tama¤o de caja de captura */
int min_ancho=MIN_ANCHO;
int max_ancho=MAX_ANCHO;
int min_alto=MIN_ALTO;
int max_alto=MAX_ALTO;

/* mensajes de error */
char *Merr_Aper="          Error de apertura.";
char *Merr_Lect="           Error de lectura.";
char *Merr_Form="          Formato incorrecto.";
char *Merr_Escr="          Error de escritura.";
char *Merr_Mem="      No hay memoria suficiente.";

void main(void)
{
int mvid, i, j, k;

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

/* inicializa nombre de fichero PCX cargado */
*npcx='\0';

/* coge modo de v¡deo actual */
mvid=g_coge_modovideo();

if(!g_modovideo(G_MV_G3C16)) {
	printf("\nEste programa requiere VGA.\n");
	exit(1);
}

if(!rg_inicializa()) {
	g_modovideo(mvid);
	printf("\nEste programa requiere rat¢n.\n");
	exit(1);
}

mg_color(clr_fondo,clr_pplano,clr_s1,clr_s2,clr_tecla);
m0=mg_crea(MENUG_VERT," CAPGRF "VERSION" "," ^Fichero: ^Captura: ^Opciones   ",
  0,0,1);
m1=mg_crea(MENUG_VERT,NULL,
  " ^Cargar fichero PCX | Cargar ^gr ficos: ^Grabar gr ficos| ^Salir",1,13,0);
m2=mg_crea(MENUG_VERT,NULL,
  " Modo ^1 (640x480) : Modo ^2 (640x350)| ^Borrar gr ficos : ^Ver gr ficos",
  3,13,0);

mg_abre(m0);

while(1) {
	i=mg_elige_opcion(m0);

	switch(i) {
		case 0 :
			mg_abre(m1);
			j=mg_elige_opcion(m1);
			mg_cierra(m1);

			switch(j) {
				case 0 :
					cargar_pcx();
					break;
				case 2 :
					cargar_grf();
					break;
				case 3 :
					grabar_grf();
					break;
				case 5 :
					if(modificado) {
						k=cuadro_siono(
						  "   ¨Grabar gr ficos?");
						if(k==1) grabar_grf();
					}
					libera_memgrf();
					mg_elimina(m0);
					mg_elimina(m1);
					mg_elimina(m2);
					g_modovideo(mvid);
					exit(0);
			}
			break;
		case 1 :
			captura_grf();
			break;
		case 2 :
			mg_abre(m2);
			j=mg_elige_opcion(m2);
			mg_cierra(m2);

			switch(j) {
				case 0 :
					if(g_coge_modovideo()!=G_MV_G3C16) {
						mg_cierra(m0);
						rg_puntero(RG_OCULTA);
						g_modovideo(G_MV_G3C16);
						restaura_pcx();
						rg_inicializa();
						mg_abre(m0);
					}
					break;
				case 1 :
					if(g_coge_modovideo()!=G_MV_G2C16) {
						mg_cierra(m0);
						rg_puntero(RG_OCULTA);
						g_modovideo(G_MV_G2C16);
						restaura_pcx();
						rg_inicializa();
						mg_abre(m0);
					}
					break;
				case 3 :
					k=cuadro_siono("  ¨Borrar gr ficos?");
					if(k==1) libera_memgrf();
					break;
				case 4 :
					mg_cierra(m0);
					rg_puntero(RG_OCULTA);
					ver_grf();
					mg_abre(m0);
					rg_puntero(RG_MUESTRA);
					break;
			}
			break;
	}
}

}

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
	BEEP: produce un pitido por el altavoz.
****************************************************************************/
void beep(void)
{

asm {
	sub bx,bx               // p gina 0
	mov ax,0E07h            // escribe el car cter de alarma
	int 10h
}

}

/****************************************************************************
	MSG_ERROR: imprime un mensaje de error.
****************************************************************************/
void msg_error(char *msg)
{
STC_CUADROG cerr;

cg_crea_cuadro(&cerr," ERROR ",CG_CENT,CG_CENT,42,8,clr_errfondo,clr_errpplano,
  clr_errs1,clr_errs2,clr_errfondo,clr_errtecla);
cg_crea_elemento(&cerr,CG_ELEM_BOTON,5,16,"^Vale",8);
cg_crea_elemento(&cerr,CG_ELEM_TEXTO,0,0,msg,40,4,CG_TXTLINEA,CG_TXTBORDE);

cg_abre(&cerr);
cg_gestiona(&cerr);
cg_cierra(&cerr);
cg_elimina(&cerr);

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

cg_crea_cuadro(&csiono," AVISO ",CG_CENT,CG_CENT,26,7,clr_fondo,clr_pplano,
  clr_s1,clr_s2,clr_fondo,clr_tecla);
cg_crea_elemento(&csiono,CG_ELEM_BOTON,4,6,"^Si",6);
cg_crea_elemento(&csiono,CG_ELEM_BOTON,4,13,"^No",6);
cg_crea_elemento(&csiono,CG_ELEM_TEXTO,0,0,msg,24,3,CG_TXTLINEA,CG_TXTBORDE);

cg_abre(&csiono);
i=cg_gestiona(&csiono);
cg_cierra(&csiono);

if(i==0) return(1);
else if(i==1) return(0);

return(-1);
}

/****************************************************************************
	CARGAR_PCX: carga fichero PCX con los gr ficos a capturar.
****************************************************************************/
void cargar_pcx(void)
{
char nf_pcx[MAXPATH];
int err;

cg_selecc_ficheros(CG_CENT,CG_CENT," Cargar PCX ",clr_fondo,clr_pplano,clr_s1,
  clr_s2,clr_fondo,clr_tecla,"","*.PCX",nf_pcx);

if(!*nf_pcx) return;

mg_cierra(m0);
rg_puntero(RG_OCULTA);
err=pcx_visualiza(nf_pcx,0,0,0,0,0);
rg_puntero(RG_MUESTRA);
mg_abre(m0);

if(err) {
	*npcx='\0';
	switch(err) {
		case E_PCX_APER :
			msg_error(Merr_Aper);
			break;
		case E_PCX_LECT :
			msg_error(Merr_Lect);
			break;
		case E_PCX_FORM :
			msg_error(Merr_Form);
			break;
	}
}
else strcpy(npcx,nf_pcx);

}

/****************************************************************************
	RESTAURA_PCX: vuelve a cargar £ltimo fichero PCX que fue cargado.
****************************************************************************/
void restaura_pcx(void)
{

if(*npcx) pcx_visualiza(npcx,0,0,0,0,0);

}

/****************************************************************************
	CAPTURA_GRF: captura gr ficos.
****************************************************************************/
void captura_grf(void)
{
STC_RATONG r;
STC_VENTANAG vinfo;
STC_CUADROG cdim;
int x=0, y=0, ancho=MIN_ANCHO, alto=MIN_ALTO, xant, yant, anchoant, altoant,
  salir=0, tecla, ancho0, alto0;
char cancho[4], calto[4], cinfo[81];
unsigned char *pgrf;
unsigned long tam;

vg_crea(&vinfo,0,0,27,5,clr_fondo,clr_pplano,clr_s1,clr_s2," Informaci¢n ");
vg_borde(&vinfo,VG_BORDE1);

cg_crea_cuadro(&cdim," Dimensiones ",CG_CENT,CG_CENT,19,11,clr_fondo,
  clr_pplano,clr_s1,clr_s2,clr_fondo,clr_tecla);
cg_crea_elemento(&cdim,CG_ELEM_INPUT,1,7,"^Ancho",6,cancho,3);
cg_crea_elemento(&cdim,CG_ELEM_INPUT,3,7,"A^lto",6,calto,3);
cg_crea_elemento(&cdim,CG_ELEM_BOTON,6,0,"^Vale",8);
cg_crea_elemento(&cdim,CG_ELEM_BOTON,6,8,"^Salir",9);

rg_puntero(RG_OCULTA);
mg_cierra(m0);

/* dibuja caja */
g_rectangulo(x,y,x+ancho-1,y+alto-1,255,G_XOR,0);

xant=x;
yant=y;
anchoant=ancho;
altoant=alto;

do {
	/* recoge estado del rat¢n */
	rg_estado(&r);

	/* si est  pulsado el bot¢n izquierdo, cambia dimensiones de caja */
	if(r.boton1) {
		rg_pon_puntero(x+ancho-1,y+alto-1);

		do {
			rg_estado(&r);

			ancho=r.x-x+1;
			alto=r.y-y+1;

			/* comprueba dimensiones */
			if((x+ancho)>g_maxx()) ancho=anchoant;
			if(ancho<min_ancho) ancho=min_ancho;
			if(ancho>max_ancho) ancho=max_ancho;
			if((y+alto)>g_maxy()) alto=altoant;
			if(alto<min_alto) alto=min_alto;
			if(alto>max_alto) alto=max_alto;

			/* si ha cambiado dimensiones borra caja anterior y */
			/* dibuja nueva */
			if((ancho!=anchoant) || (alto!=altoant)) {
				g_rectangulo(xant,yant,xant+anchoant-1,
				  yant+altoant-1,255,G_XOR,0);
				g_rectangulo(x,y,x+ancho-1,y+alto-1,255,
				  G_XOR,0);
				anchoant=ancho;
				altoant=alto;
			}
		} while(r.boton1);

		rg_pon_puntero(x,y);
	}
	/* si est  pulsado el bot¢n derecho, captura gr fico */
	else if(r.boton2) {
		if(num_grf<MAX_GRF) {
			tam=blq_tam(x,y,x+ancho-1,y+alto-1);

			/* si pasa de m ximo tama¤o permitido o de la */
			/* memoria disponible, sale */
			if((tam>MAX_TAMBLQ) || (tam>(coreleft()-MEM_RESERVA))) {
				beep();
				beep();
				continue;
			}

			pgrf=(unsigned char *)malloc((size_t)tam);

			/* si hay memoria, captura gr fico */
			if(pgrf!=NULL) {
				/* borra cuadro */
				g_rectangulo(x,y,x+ancho-1,y+alto-1,255,
				  G_XOR,0);

				grf[num_grf].tam=tam;
				blq_coge(x,y,x+ancho-1,y+alto-1,pgrf);
				grf[num_grf].grf=pgrf;
				beep();
				num_grf++;

				/* dibuja cuadro */
				g_rectangulo(x,y,x+ancho-1,y+alto-1,255,
				  G_XOR,0);

				/* espera hasta que suelte bot¢n derecho */
				do {
					rg_estado(&r);
				} while(r.boton2);

				/* indica modificaciones */
				modificado=1;
			}
			else {
				beep();
				beep();
			}
		}
	}
	else {
		x=r.x;
		y=r.y;

		/* comprueba l¡mites */
		if((x+ancho)>g_maxx()) x=g_maxx()-ancho;
		if((y+alto)>g_maxy()) y=g_maxy()-alto;

		/* si ha movido el rat¢n borra caja anterior y dibuja nueva */
		if((x!=xant) || (y!=yant)) {
			g_rectangulo(xant,yant,xant+anchoant-1,yant+altoant-1,
			  255,G_XOR,0);
			g_rectangulo(x,y,x+ancho-1,y+alto-1,255,G_XOR,0);
			xant=x;
			yant=y;
		}
	}

	if(bioskey(1)) {
		tecla=(bioskey(0) >> 8) & 0x00ff;

		switch(tecla) {
			case T_ESC :
				salir=1;
				break;
			case T_F1 :
				vg_abre(&vinfo);
				vg_pon_cursor(&vinfo,0,3);
				sprintf(cinfo,"Memoria libre: %lu",coreleft());
				vg_impcad(&vinfo,cinfo,VG_RELLENA);
				vg_pon_cursor(&vinfo,1,1);
				sprintf(cinfo,"Gr fico (bytes): %lu",
				  blq_tam(x,y,x+ancho-1,y+alto-1));
				vg_impcad(&vinfo,cinfo,VG_RELLENA);
				vg_pon_cursor(&vinfo,2,3);
				sprintf(cinfo,"Num. gr ficos: %i",num_grf);
				vg_impcad(&vinfo,cinfo,VG_RELLENA);
				bioskey(0);
				vg_cierra(&vinfo);
				break;
			case T_F2 :
				/* coge anchura y altura actual */
				itoa(ancho,cancho,10);
				itoa(alto,calto,10);

				cg_abre(&cdim);
				cg_gestiona(&cdim);
				cg_cierra(&cdim);

				ancho0=atoi(cancho);
				alto0=atoi(calto);
				if(((x+ancho0)<=g_maxx()) &&
				  (ancho0>=MIN_ANCHO)) ancho=ancho0;
				if(((y+alto0)<=g_maxy()) &&
				  (alto0>=MIN_ALTO)) alto=alto0;

				/* oculta y restaura puntero a su posici¢n */
				rg_puntero(RG_OCULTA);
				rg_pon_puntero(x,y);

				/* borra caja antigua y dibuja nueva */
				g_rectangulo(x,y,x+anchoant-1,y+altoant-1,255,
				  G_XOR,0);
				g_rectangulo(x,y,x+ancho-1,y+alto-1,255,
				  G_XOR,0);
				anchoant=ancho;
				altoant=alto;
				break;
			case T_F3 :
				g_rectangulo(x,y,x+ancho-1,y+alto-1,255,
				  G_XOR,0);
				ver_grf();
				rg_pon_puntero(x,y);
				g_rectangulo(x,y,x+ancho-1,y+alto-1,255,
				  G_XOR,0);
				break;
		}
	}

} while(!salir);

cg_elimina(&cdim);

/* borra caja */
g_rectangulo(x,y,x+ancho-1,y+alto-1,255,G_XOR,0);

mg_abre(m0);
rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	LIBERA_MEMGRF: libera memoria ocupada por gr ficos capturados.
****************************************************************************/
void libera_memgrf(void)
{
int i;

/* si no hay gr ficos, sale */
if(!num_grf) return;

for(i=0; i<num_grf; i++) {
	if(grf[i].grf!=NULL) {
		free(grf[i].grf);
		grf[i].grf=NULL;
		grf[i].tam=0;
	}
}

num_grf=0;

}

/****************************************************************************
	DIBUJA_FONDO_VER: dibuja fondo de pantalla de visualizaci¢n de
	  gr ficos capturados.
****************************************************************************/
void dibuja_fondo_ver(void)
{
int i;

g_borra_pantalla();

g_linea(0,0,g_maxx()-1,0,clr_s1,G_NORM);
g_linea(0,0,0,g_maxy()-1,clr_s1,G_NORM);
g_linea(1,g_maxy()-1,g_maxx()-1,g_maxy()-1,clr_s2,G_NORM);
g_linea(g_maxx()-1,1,g_maxx()-1,g_maxy()-1,clr_s2,G_NORM);
g_linea(1,1,g_maxx()-2,1,clr_s1,G_NORM);
g_linea(1,1,1,g_maxy()-2,clr_s1,G_NORM);
g_linea(2,g_maxy()-2,g_maxx()-2,g_maxy()-2,clr_s2,G_NORM);
g_linea(g_maxx()-2,2,g_maxx()-2,g_maxy()-2,clr_s2,G_NORM);

for(i=2; i<7; i++) g_rectangulo(i,i,g_maxx()-i-1,g_maxy()-i-1,clr_fondo,
  G_NORM,0);

g_linea(7,7,g_maxx()-8,7,clr_s2,G_NORM);
g_linea(7,7,7,g_maxy()-8,clr_s2,G_NORM);
g_linea(8,g_maxy()-8,g_maxx()-8,g_maxy()-8,clr_s1,G_NORM);
g_linea(g_maxx()-8,8,g_maxx()-8,g_maxy()-8,clr_s1,G_NORM);

}

/****************************************************************************
	VER_GRF: ver gr ficos capturados.
****************************************************************************/
void ver_grf(void)
{
STC_VENTANAG vinfo;
char cinfo[81];
int i, ngrf=0, salir, tecla;

/* si no hay gr ficos, sale */
if(!num_grf) return;

vg_crea(&vinfo,0,80-13,13,3,clr_fondo,clr_pplano,clr_s1,clr_s2," Gr fico ");
vg_borde(&vinfo,VG_BORDE1);

dibuja_fondo_ver();

while(1) {
	blq_pon(8,8,grf[ngrf].grf);
	vg_dibuja(&vinfo,1);
	sprintf(cinfo,"%i/%i",ngrf+1,num_grf);
	vg_pon_cursor(&vinfo,0,(11-strlen(cinfo))/2);
	vg_impcad(&vinfo,cinfo,VG_RELLENA);

	salir=0;
	while(!salir) {
		tecla=(bioskey(0) >> 8) & 0x00ff;
		switch(tecla) {
			case T_DER :
				if(ngrf<(num_grf-1)) {
					ngrf++;
					salir=1;
				}
				break;
			case T_IZQ :
				if(ngrf>0) {
					ngrf--;
					salir=1;
				}
				break;
			case T_ORG :
				if(ngrf) {
					ngrf=0;
					salir=1;
				}
				break;
			case T_FIN :
				if(ngrf!=(num_grf-1)) {
					ngrf=num_grf-1;
					salir=1;
				}
				break;
			case T_BORRAR :
				i=cuadro_siono("¨Borrar este gr fico?");
				rg_puntero(RG_OCULTA);
				if(i==1) {
					borrar_grf(ngrf);

					/* si estaba en el £ltimo gr fico */
					/* y lo borr¢ visualiza anterior */
					if(ngrf>(num_grf-1)) ngrf=num_grf-1;

					/* si ha borrado todos los */
					/* gr ficos, sale */
					if(!num_grf) {
						g_borra_pantalla();
						restaura_pcx();
						return;
					}
				}
				salir=1;
				break;
			case T_ESC :
				g_borra_pantalla();
				restaura_pcx();
				return;
		}
	}
	dibuja_fondo_ver();
}

}

/****************************************************************************
	GRABAR_GRF: grabar gr ficos capturados.
****************************************************************************/
void grabar_grf(void)
{
FILE *f_grf;
char nf_grf[MAXPATH];
int i;

cg_selecc_ficheros(CG_CENT,CG_CENT," Grabar gr ficos ",clr_fondo,clr_pplano,
  clr_s1,clr_s2,clr_fondo,clr_tecla,"","*.GRF",nf_grf);

if(!*nf_grf) return;

if((f_grf=fopen(nf_grf,"wb"))==NULL) {
	msg_error(Merr_Aper);
	return;
}

/* graba n£mero de gr ficos */
if(fwrite(&num_grf,sizeof(int),1,f_grf)!=1) {
	msg_error(Merr_Escr);
	fclose(f_grf);
	return;
}

/* graba tama¤o de gr ficos */
for(i=0; i<num_grf; i++) {
	if(fwrite(&grf[i].tam,sizeof(unsigned long),1,f_grf)!=1) {
		msg_error(Merr_Escr);
		fclose(f_grf);
		return;
	}
}

/* graba gr ficos */
for(i=0; i<num_grf; i++) {
	if(fwrite(grf[i].grf,sizeof(unsigned char),(size_t)grf[i].tam,
	  f_grf)!=(size_t)grf[i].tam) {
		msg_error(Merr_Escr);
		fclose(f_grf);
		return;
	}
}

fclose(f_grf);

modificado=0;

}

/****************************************************************************
	CARGAR_GRF: cargar gr ficos.
****************************************************************************/
void cargar_grf(void)
{
FILE *f_grf;
char nf_grf[MAXPATH];
unsigned char *pgrf;
int i;

/* si hay modificaciones sin grabar, pregunta si quiere grabarlos */
if(modificado) {
	i=cuadro_siono("   ¨Grabar gr ficos?");
	if(i==1) grabar_grf();
	modificado=0;
}

cg_selecc_ficheros(CG_CENT,CG_CENT," Cargar gr ficos ",clr_fondo,clr_pplano,
  clr_s1,clr_s2,clr_fondo,clr_tecla,"","*.GRF",nf_grf);

if(!*nf_grf) return;

/* borra gr ficos existentes */
libera_memgrf();

if((f_grf=fopen(nf_grf,"rb"))==NULL) {
	msg_error(Merr_Aper);
	return;
}

/* carga n£mero de gr ficos */
if(fread(&num_grf,sizeof(int),1,f_grf)!=1) {
	msg_error(Merr_Lect);
	fclose(f_grf);
	return;
}

/* carga tama¤o de gr ficos */
for(i=0; i<num_grf; i++) {
	if(fread(&grf[i].tam,sizeof(unsigned long),1,f_grf)!=1) {
		msg_error(Merr_Lect);
		fclose(f_grf);
		return;
	}
	grf[i].grf=NULL;
}

/* carga gr ficos */
for(i=0; i<num_grf; i++) {
	pgrf=(unsigned char *)malloc((size_t)grf[i].tam);
	if(pgrf==NULL) {
		libera_memgrf();
		msg_error(Merr_Mem);
		return;
	}

	grf[i].grf=pgrf;

	if(fread(grf[i].grf,sizeof(unsigned char),(size_t)grf[i].tam,
	  f_grf)!=(size_t)grf[i].tam) {
		libera_memgrf();
		msg_error(Merr_Lect);
		fclose(f_grf);
		return;
	}
}

fclose(f_grf);

}

/****************************************************************************
	BORRAR_GRF: borra un gr fico.
	  Entrada:	'ngrf' n£mero de gr fico a borrar
****************************************************************************/
void borrar_grf(int ngrf)
{
int i;

/* sale si no hay gr ficos capturados o si no existe gr fico */
if(!num_grf || (ngrf>(num_grf-1))) return;

/* libera memoria ocupada por el gr fico */
free(grf[ngrf].grf);

/* desplaza resto de gr ficos una posici¢n hacia arriba */
for(i=ngrf+1; i<num_grf; i++) {
	grf[i-1].grf=grf[i].grf;
	grf[i-1].tam=grf[i].tam;
}

num_grf--;

/* indica modificaciones */
modificado=1;

}
