/****************************************************************************
			  GENERADOR DE MUSICA SINTAC
			    (c)1995 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <dos.h>
#include <bios.h>
#include "graf.h"
#include "rutvga.h"
#include "ventana.h"
#include "raton.h"
#include "cuadro.h"
#include "sintac.h"
#include "color.h"
#include "sonido.h"
#include "gmuss.h"

/*** Variables externas ***/
extern char *c_Borde_Sel;
extern char *c_Esp;

/*** Variables globales ***/
STC_CFG cfg;

/* nombre de fichero de entrada */
char nf_snd[MAXPATH];

/* nombres de notas */
char *nm_notas[13]={
	"DO",
	"DO#",
	"RE",
	"RE#",
	"MI",
	"FA",
	"FA#",
	"SOL",
	"SOL#",
	"LA",
	"LA#",
	"SI",
	"<..>"
};

/* tabla de notas */
STC_SND notas[MAX_NOTAS];

/*** Programa principal ***/
void main(int argc, char *argv[])
{
STC_CABSND cabsnd;
STC_SND snd;
STC_CUADRO csnd;
STC_ELEM *e;
STC_ELEM_LISTA *lst;
FILE *fsnd;
char buff[81], *cab=" GENERADOR DE MUSICA SINTAC versi¢n "GMUSS_VERSION
  "  (c)1995 JSJ Soft Ltd. ";
int i, n, nota, octava, n_notas=0, salir=0, errfich=0, lstp=0;

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

/* analiza par metros de entrada */
analiza_args(argc,argv);

/* lee fichero de configuraci¢n */
lee_cfg(argv[0]);

/* si no pudo establecer modo de v¡deo */
if(!g_modovideo(G_MV_T80C)) {
	printf("\nEste programa requiere tarjeta CGA o mejor.\n");
	exit(1);
}
activa_vga();

if(!*nf_snd) c_selecc_ficheros(C_CENT,C_CENT," Fichero SND ",cfg.color_cs,
  cfg.color_css1,cfg.color_css2,cfg.color_csboton,cfg.color_csinput,
  cfg.color_cstec,cfg.color_cstecboton,cfg.color_cssel,"","*.*",nf_snd);

esconde_cursor();

if(*nf_snd) {
	if((fsnd=fopen(nf_snd,"rb+"))==NULL) {
		if(pregunta_siono(" Fichero SND no existe, ¨crearlo? ")) {
			if((fsnd=fopen(nf_snd,"wb+"))==NULL) {
				imprime_error(" No se puede crear fichero SND ");
				salir=1;
				errfich=1;
			}
			else {
				strcpy(cabsnd.id,SND_ID);
				cabsnd.n_snd=0;
				fwrite(&cabsnd,sizeof(STC_CABSND),1,fsnd);
			}
		}
		else salir=1;
	}
	else {
		/* comprueba si es fichero SND */
		fread(&cabsnd,sizeof(STC_CABSND),1,fsnd);
		if(strcmp(cabsnd.id,SND_ID)) {
			imprime_error(" Fichero SND no v lido ");
			salir=1;
			errfich=1;
		}
	}
	fclose(fsnd);
}
else {
	salir=1;
	errfich=1;
}

c_crea_cuadro(&csnd,cab,C_CENT,C_CENT,GMUSS_ANCHO,GMUSS_ALTO,cfg.color_cs,
  cfg.color_css1,cfg.color_css2,cfg.color_csboton,cfg.color_csinput,
  cfg.color_cssel,cfg.color_cstec,cfg.color_cstecboton);
c_crea_elemento(&csnd,C_ELEM_BOTON,1,GMUSS_ANCHO-17,"^A¤adir",13);
c_crea_elemento(&csnd,C_ELEM_BOTON,3,GMUSS_ANCHO-17,"^Teclado",13);
c_crea_elemento(&csnd,C_ELEM_BOTON,5,GMUSS_ANCHO-17,"^Insertar",13);
c_crea_elemento(&csnd,C_ELEM_BOTON,7,GMUSS_ANCHO-17,"^Modificar",13);
c_crea_elemento(&csnd,C_ELEM_BOTON,9,GMUSS_ANCHO-17,"^Borrar",13);
c_crea_elemento(&csnd,C_ELEM_BOTON,11,GMUSS_ANCHO-17,"^Reproducir",13);
c_crea_elemento(&csnd,C_ELEM_BOTON,13,GMUSS_ANCHO-17,"Borrar t^odo",13);
c_crea_elemento(&csnd,C_ELEM_BOTON,21,0,"^+ Duraci¢n",14);
c_crea_elemento(&csnd,C_ELEM_BOTON,21,16,"^- Duraci¢n",14);
c_crea_elemento(&csnd,C_ELEM_BOTON,21,32,"^Salir",9);
e=c_crea_elemento(&csnd,C_ELEM_LISTA,0,0," ^Notas ",GMUSS_ANCHO-18,
  GMUSS_ALTO-5,C_LSTNORMAL,C_LSTSINORDEN);
lst=(STC_ELEM_LISTA *)(e->info);

if(!salir) c_abre(&csnd);

if(!errfich) {
	/* lee fichero */
	if((fsnd=fopen(nf_snd,"rb"))!=NULL) {
		fread(&cabsnd,sizeof(STC_CABSND),1,fsnd);
		n_notas=cabsnd.n_snd;
		for(i=0; i<n_notas; i++) fread(&notas[i],sizeof(STC_SND),1,fsnd);
	}
	else {
		imprime_error(" Error de apertura de fichero SND ");
		salir=1;
		errfich=1;
	}
}

while(!salir) {
	c_borra_lista(lst);
	for(i=0; i<n_notas; i++) {
		if(notas[i].nota==SND_SILENCIO) {
			nota=12;
			octava=0;
		}
		else {
			nota=notas[i].nota%12;
			octava=notas[i].nota/12;
		}
		sprintf(buff,"%4s %1i %5u",nm_notas[nota],octava,
		  notas[i].duracion);
		c_mete_en_lista(lst,buff);
	}
	if(lstp) {
		lst->elemento_sel=lstp;
		if((lst->elemento_sel-lst->elemento_pr)>(GMUSS_ALTO-7))
		  lst->elemento_pr=lst->elemento_sel-(GMUSS_ALTO-8);
	}
	c_dibuja_elemento(&csnd,e);

	i=c_gestiona(&csnd);
	switch(i) {
		case 0 :	/* a¤adir */
			if(n_notas>=(MAX_NOTAS-1)) break;
			elige_nota(&snd);
			if(snd.nota==-1) break;
			notas[n_notas].nota=snd.nota;
			notas[n_notas].duracion=snd.duracion;
			lstp=n_notas;
			n_notas++;
			break;
		case 1 :	/* teclado */
			while(1) {
				if(n_notas>=(MAX_NOTAS-1)) break;
				elige_nota(&snd);
				if(snd.nota==-1) break;
				notas[n_notas].nota=snd.nota;
				notas[n_notas].duracion=snd.duracion;
				n_notas++;
			}
			lstp=n_notas-1;
			break;
		case 2 :	/* insertar */
			if(n_notas>=(MAX_NOTAS-1)) break;
			elige_nota(&snd);
			if(snd.nota==-1) break;
			for(n=n_notas; n>lst->elemento_sel; n--)
			  notas[n]=notas[n-1];
			notas[lst->elemento_sel].nota=snd.nota;
			notas[lst->elemento_sel].duracion=snd.duracion;
			lstp=lst->elemento_sel;
			n_notas++;
			break;
		case 3 :	/* modificar */
			if(!n_notas) break;
			elige_nota(&snd);
			if(snd.nota==-1) break;
			lstp=lst->elemento_sel;
			notas[lstp].nota=snd.nota;
			notas[lstp].duracion=snd.duracion;
			break;
		case 4 :	/* borrar */
			if(!n_notas) break;
			for(n=lst->elemento_sel; n<n_notas; n++) {
				notas[n].nota=notas[n+1].nota;
				notas[n].duracion=notas[n+1].duracion;
			}
			n_notas--;
			lstp=lst->elemento_sel;
			if(lstp>(n_notas-1)) lstp=n_notas-1;
			break;
		case 5 :      	/* reproducir */
			if(!n_notas) break;
			reproduce(lst->elemento_sel,n_notas);
			lstp=lst->elemento_sel;
			break;
		case 6 :	/* borrar todo */
			if(pregunta_siono(" ¨Borrar todo? ")) {
				n_notas=0;
				lstp=0;
			}
			break;
		case 7 :	/* + duraci¢n */
			lstp=lst->elemento_sel;
			if(notas[lstp].duracion<10000) notas[lstp].duracion++;
			break;
		case 8 :	/* - duraci¢n */
			lstp=lst->elemento_sel;
			if(notas[lstp].duracion>0) notas[lstp].duracion--;
			break;
		case 9 :	/* salir */
		case -1:
			salir=1;
			break;
	}
}

/* graba fichero */
if(!errfich) {
	if((fsnd=fopen(nf_snd,"wb"))!=NULL) {
		strcpy(cabsnd.id,SND_ID);
		cabsnd.n_snd=n_notas;
		fwrite(&cabsnd,sizeof(STC_CABSND),1,fsnd);
		for(i=0; i<n_notas; i++) fwrite(&notas[i],sizeof(STC_SND),1,fsnd);
	}
	else imprime_error(" Error de apertura de fichero SND ");
}

c_borra_lista(lst);
c_cierra(&csnd);
c_elimina(&csnd);
r_puntero(R_OCULTA);

vga_activa_fuente(NULL);
vga_parpadeo(1);
g_modovideo(G_MV_T80C);

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
	ANALIZA_ARGS: analiza los argumentos de la l¡nea de llamada al
	  programa.
	  Entrada:      'argc' n£mero de argumentos en la l¡nea de llamada
			'argv' matriz de punteros a los argumentos, el primero
			ser  siempre el nombre del programa
	  Salida:       1 si error, 0 si no
		      variables globales:-
			'nf_snd' nombre de fichero de entrada
			indicadores correspondientes actualizados
****************************************************************************/
int analiza_args(int argc, char *argv[])
{
int ppar=1;

*nf_snd='\0';

while(ppar<argc) {
	/* si no empieza por '/' o '-' */
	if((*argv[ppar]!='/') && (*argv[ppar]!='-')) {
		if(*nf_snd=='\0') {
			strcpy(nf_snd,argv[ppar]);
			strupr(nf_snd);
		}
	}

	ppar++;
}

return(0);
}

/****************************************************************************
	ACTIVA_VGA: rutina de activaci¢n de caracter¡sticas especiales
	  para la VGA (redefinici¢n de caracteres, etc...)
****************************************************************************/
void activa_vga(void)
{
char nf_fuente[MAXPATH];

/* ruta al fichero de juego de caracteres */
strcpy(nf_fuente,cfg.dir_sintac);
strcat(nf_fuente,"EDS.FNT");

g_modovideo(G_MV_T80C);

if(vga_activa_fuente(nf_fuente)) {
        v_borde_def("ÚÄ¿³´ÀÅµ");
        c_Borde_Sel="ÉÍ»º¹ÈÎ¼";
	c_Esp="íîï¶ÁÂ\x07ÃÆ";
}
else {
        v_borde_def("ÉÍ¸º³ÓÄÙ");
        c_Borde_Sel="ÉÍ»ººÈÍ¼";
	c_Esp="\x18\x19±þ[]\x07X ";
}

vga_parpadeo(0);

}

/****************************************************************************
	LEE_CFG: lee fichero de configuraci¢n si existe, si no asigna
	  valores por defecto.
	  Entrada:      'argv0' ruta y nombre del programa (normalmente
			contenido en argv[0])
****************************************************************************/
void lee_cfg(char *argv0)
{
char ruta[MAXPATH], drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILE],
  ext[MAXEXT], nf_cfg[MAXPATH];
FILE *fcfg;

/* coge ruta de programa */
fnsplit(argv0,drive,dir,fname,ext);
strcpy(ruta,drive);
strcat(ruta,dir);
strupr(ruta);

/* valores de configuraci¢n por defecto */
strcpy(cfg.dir_sintac,ruta);

cfg.color_cs=COLOR_CS;
cfg.color_css1=COLOR_CSS1;
cfg.color_css2=COLOR_CSS2;
cfg.color_csboton=COLOR_CSBOTON;
cfg.color_csinput=COLOR_CSINPUT;
cfg.color_cssel=COLOR_CSSEL;
cfg.color_cstec=COLOR_CSTEC;
cfg.color_cstecboton=COLOR_CSTECBOTON;

cfg.color_err=COLOR_ERR;
cfg.color_errs1=COLOR_ERRS1;
cfg.color_errs2=COLOR_ERRS2;
cfg.color_errboton=COLOR_ERRBOTON;
cfg.color_errinput=COLOR_ERRINPUT;
cfg.color_errsel=COLOR_ERRSEL;
cfg.color_errtec=COLOR_ERRTEC;
cfg.color_errtecboton=COLOR_ERRTECBOTON;

/* lee fichero de configuraci¢n, si existe */
strcpy(nf_cfg,ruta);
strcat(nf_cfg,"..\\");
strcat(nf_cfg,NF_CFG);
if((fcfg=fopen(nf_cfg,"rb"))==NULL) return;
if(fread(&cfg,sizeof(STC_CFG),1,fcfg)!=1) {
	fclose(fcfg);
	return;
}

}

/****************************************************************************
	ESCONDE_CURSOR: oculta el cursor.
****************************************************************************/
void esconde_cursor(void)
{

asm {
	mov ah,02h              // funci¢n definir posici¢n del cursor
	mov bh,0                // supone p gina 0
	mov dh,25               // DH = fila del cursor
	mov dl,0                // DL = columna del cursor
	int 10h
}

}

/****************************************************************************
	IMPRIME_ERROR: imprime mensaje de error.
	  Entrada:      'msg' mensaje de error
****************************************************************************/
void imprime_error(char *msg)
{
STC_CUADRO c;
int i, ancho;

ancho=strlen(msg)+4;

c_crea_cuadro(&c," ERROR ",C_CENT,C_CENT,ancho,7,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,cfg.color_errboton,cfg.color_errinput,
  cfg.color_errsel,cfg.color_errtec,cfg.color_errtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,3,(ancho-10)/2,"^Vale",8);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,msg,ancho-2,3,C_TXTLINEA,C_TXTNOBORDE);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);
c_elimina(&c);

}

/****************************************************************************
	PREGUNTA_SIONO: presenta un cuadro de di logo con una pregunta
	  y dos opciones (si o no).
	  Entrada:      'preg' texto de la pregunta
	  Salida:       1 si contest¢ si, 0 si no
****************************************************************************/
int pregunta_siono(char *preg)
{
STC_CUADRO c;
int i, ancho, col;

ancho=strlen(preg)+4;
col=(ancho-15)/2;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,ancho,8,cfg.color_err,cfg.color_errs1,
  cfg.color_errs2,cfg.color_errboton,cfg.color_errinput,cfg.color_errsel,
  cfg.color_errtec,cfg.color_errtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,4,col-1,"^Si",6);
c_crea_elemento(&c,C_ELEM_BOTON,4,col+8,"^No",6);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,preg,ancho-2,3,C_TXTLINEA,C_TXTBORDE);

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);
c_elimina(&c);

if(i==0) return(1);

return(0);
}

/****************************************************************************
	ELIGE_NOTA: cuadro de selecci¢n de notas.
	  Entrada:	'snd' puntero a estructura donde dejar la nota
	  Salida:	'snd' conteniendo datos de la nota seleccionada,
			si 'snd.nota' es -1 no se seleccion¢ ninguna
****************************************************************************/
void elige_nota(STC_SND *snd)
{
STC_CUADRO cnota;
static char octava[2]={'4', '\0'},
  duracion[6]={'1', '0', '\0', '\0', '\0', '\0'};
int i, o, salir=0;

c_crea_cuadro(&cnota," Notas ",C_CENT,C_CENT,42,15,cfg.color_cs,
  cfg.color_css1,cfg.color_css2,cfg.color_csboton,cfg.color_csinput,
  cfg.color_cssel,cfg.color_cstec,cfg.color_cstecboton);
c_crea_elemento(&cnota,C_ELEM_BOTON,1,1,"^1 DO",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,1,11,"^2 DO#",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,3,1,"^3 RE",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,3,11,"^4 RE#",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,5,1,"^5 MI",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,5,11,"^6 FA",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,7,1,"^7 FA#",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,7,11,"^8 SOL",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,9,1,"^9 SOL#",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,9,11,"^0 LA",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,11,1,"^A LA#",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,11,11,"^B SI",8);
c_crea_elemento(&cnota,C_ELEM_BOTON,1,21,"^SILENCIO",10);
c_crea_elemento(&cnota,C_ELEM_INPUT,6,31,"^Octava",4,octava,1);
c_crea_elemento(&cnota,C_ELEM_INPUT,9,31,"^Duraci¢n",8,duracion,5);

c_abre(&cnota);
do {
	i=c_gestiona(&cnota);
	o=atoi(octava);
	if((i!=-1) && ((o<0) || (o>7)))
	  imprime_error(" Octava debe estar entre 0 y 7 ");
	if((o>=0) && (o<=7)) salir=1;
} while((i!=-1) && !salir);
c_cierra(&cnota);
c_elimina(&cnota);

snd->duracion=atoi(duracion);
if(i==-1) snd->nota=-1;
else if(i>11) snd->nota=SND_SILENCIO;
else {
	snd->nota=i+(12*o);
	bpr_nota(snd->nota,snd->duracion);
}

}

/****************************************************************************
	REPRODUCE: reproduce m£sica.
	  Entrada:      'nota_i' nota inicial
			'nota_f' nota final
****************************************************************************/
void reproduce(int nota_i, int nota_f)
{

for(; nota_i<nota_f; nota_i++) {
	if(notas[nota_i].nota==SND_SILENCIO) delay(100*notas[nota_i].duracion);
	else bpr_nota(notas[nota_i].nota,notas[nota_i].duracion);
	if(bioskey(1)) {
		bioskey(0);
		break;
	}
}

}
