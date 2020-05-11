/****************************************************************************
			       LINKADOR SINTAC
			    (c)1995 JSJ Soft Ltd.
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <alloc.h>
#include <string.h>
#include <dir.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>
#include "graf.h"
#include "rutvga.h"
#include "ventana.h"
#include "raton.h"
#include "cuadro.h"
#include "sintac.h"
#include "version.h"
#include "color.h"
#include "lks.h"

/*** Variables externas ***/
extern char *c_Borde_Sel;
extern char *c_Esp;

/*** Variables globales ***/
/* tama§o del STACK */
unsigned _stklen=8192;

/* configuraci¢n */
STC_CFG cfg;

/* indicador de si se ejecuta desde entorno */
BOOLEAN lks_eds=FALSE;

/* nombres de fichero de base de datos y ejecutable */
char nf_bd[MAXPATH];
char nf_exe[MAXPATH];

/* mensajes de error */
char *MsgErr_Rtime=" Error de apertura de m¢dulo 'runtime'";
char *MsgErr_Exe="   Error de apertura de fichero EXE";
char *MsgErr_Bd="  Error de apertura de base de datos";
char *MsgErr_Lect="           Error de lectura";
char *MsgErr_Escr="          Error de escritura";
char *MsgErr_Nobd="  Fichero de base de datos no v†lido";

void main(int argc, char *argv[])
{
STC_CUADRO c;
int i;

/* instala 'handler' de errores cr°ticos */
harderr(int24_hnd);

/* analiza par†metros de entrada */
analiza_args(argc,argv);

if(lks_eds==FALSE) {
	/* si no pudo establecer modo de v°deo */
	if(!g_modovideo(G_MV_T80C)) {
		printf("\nEste programa requiere tarjeta CGA o mejor.\n");
		exit(1);
	}
}

if(vga_activa_fuente("EDS.FNT")) {
        v_borde_def("⁄ƒø≥¥¿≈µ");
        c_Borde_Sel="…Õª∫π»Œº";
	c_Esp="ÌÓÔ∂¡¬\x07√∆";
}
else {
        v_borde_def("…Õ∏∫≥”ƒŸ");
        c_Borde_Sel="…Õª∫∫»Õº";
	c_Esp="\x18\x19±˛[]\x07X ";
}
vga_parpadeo(0);

lee_cfg(argv[0]);

c_crea_cuadro(&c," LINKADOR "COPYRIGHT" versi¢n "VERSION" ",C_CENT,C_CENT,
  64,9,cfg.color_cs,cfg.color_css1,cfg.color_css2,cfg.color_csboton,
  cfg.color_csinput,cfg.color_cssel,cfg.color_cstec,cfg.color_cstecboton);
c_crea_elemento(&c,C_ELEM_INPUT,1,26,"^Fichero de base de datos",35,nf_bd,
  MAXPATH-1);
c_crea_elemento(&c,C_ELEM_INPUT,3,26,"Fichero ^EXE",35,nf_exe,MAXPATH-1);
if(lks_eds==FALSE) {
	c_crea_elemento(&c,C_ELEM_BOTON,5,20,"^Linkar",10);
	c_crea_elemento(&c,C_ELEM_BOTON,5,32,"^Salir",10);
}

c_abre(&c);

if(lks_eds==FALSE) {
	i=c_gestiona(&c);
	if(i==2) linkar();
}
else linkar();

c_cierra(&c);
c_elimina(&c);

r_puntero(R_OCULTA);

if(lks_eds==FALSE) {
	g_modovideo(G_MV_T80C);
	vga_activa_fuente(NULL);
	vga_parpadeo(1);
}

}

#pragma warn -par
/****************************************************************************
	INT24_HND: rutina de manejo de errores cr°ticos de hardware.
****************************************************************************/
int int24_hnd(int errval, int ax, int bp, int si)
{

hardretn(2);

return(2);
}
#pragma warn +par

/****************************************************************************
	LEE_CFG: lee fichero de configuraci¢n si existe, si no asigna
	  valores por defecto.
	  Entrada:      'argv0' ruta y nombre del programa (normalmente
			contenido en argv[0])
****************************************************************************/
void lee_cfg(char *argv0)
{
char ruta[MAXPATH], drive[MAXDRIVE], dir[MAXDIR], fname[MAXFILE], ext[MAXEXT],
  nf_cfg[MAXPATH];
FILE *fcfg;

/* coge ruta de programa */
fnsplit(argv0,drive,dir,fname,ext);
strcpy(ruta,drive);
strcat(ruta,dir);
strupr(ruta);

/* valores de configuraci¢n por defecto */
strcpy(cfg.dir_sintac,ruta);

cfg.color_err=COLOR_ERR;
cfg.color_errs1=COLOR_ERRS1;
cfg.color_errs2=COLOR_ERRS2;
cfg.color_errboton=COLOR_ERRBOTON;
cfg.color_errinput=COLOR_ERRINPUT;
cfg.color_errsel=COLOR_ERRSEL;
cfg.color_errtec=COLOR_ERRTEC;
cfg.color_errtecboton=COLOR_ERRTECBOTON;

cfg.color_cs=COLOR_CS;
cfg.color_css1=COLOR_CSS1;
cfg.color_css2=COLOR_CSS2;
cfg.color_csboton=COLOR_CSBOTON;
cfg.color_csinput=COLOR_CSINPUT;
cfg.color_cssel=COLOR_CSSEL;
cfg.color_cstec=COLOR_CSTEC;
cfg.color_cstecboton=COLOR_CSTECBOTON;

/* lee fichero de configuraci¢n, si existe */
strcpy(nf_cfg,ruta);
strcat(nf_cfg,NF_CFG);
if((fcfg=fopen(nf_cfg,"rb"))!=NULL) {
	fread(&cfg,sizeof(STC_CFG),1,fcfg);
	fclose(fcfg);
}

}

/****************************************************************************
	ESCONDE_CURSOR: oculta el cursor.
****************************************************************************/
void esconde_cursor(void)
{

asm {
	mov ah,02h              // funci¢n definir posici¢n del cursor
	mov bh,0                // supone p†gina 0
	mov dh,25               // DH = fila del cursor
	mov dl,0                // DL = columna del cursor
	int 10h
}

}

/****************************************************************************
	ANALIZA_ARGS: analiza los argumentos de la l°nea de llamada al
	  programa.
	  Entrada:      'argc' n£mero de argumentos en la l°nea de llamada
			'argv' matriz de punteros a los argumentos, el primero
			ser† siempre el nombre del programa
	  Salida:       1 si error, 0 si no
		      variables globales:-
			'nf_bd', 'nf_exe' nombres de ficheros de entrada y
			salida e indicadores corrrespondientes actualizados
****************************************************************************/
int analiza_args(int argc, char *argv[])
{
int ppar=1;
char par[129];

*nf_bd='\0';
*nf_exe='\0';

while(ppar<argc) {
	/* comprueba si empieza por '/' o '-' */
	if((*argv[ppar]=='/') || (*argv[ppar]=='-')) {
		/* copia argumento sin car†cter inicial y pasa a may£sculas */
		strcpy(par,argv[ppar]+1);
		strupr(par);

		/* mira si es alguno de los par†metros v†lidos */
		if(!strcmp(par,"E")) lks_eds=TRUE;
	}
	else {
		if(*nf_bd=='\0') {
			strcpy(nf_bd,argv[ppar]);
			strupr(nf_bd);
		}
		else if(*nf_exe=='\0') {
			strcpy(nf_exe,argv[ppar]);
			strupr(nf_exe);
		}
	}

	ppar++;
}

if(*nf_exe=='\0') construye_nfexe();

return(0);
}

/****************************************************************************
	CONSTRUYE_NFEXE: construye nombre de fichero de salida a partir
	  del de la base de datos.
	  Entrada:    variables globales:-
			'nfbd' nombre de fichero de base de datos
	  Salida:     variables globales:-
			'nfexe' nombre de fichero ejecutable
****************************************************************************/
void construye_nfexe(void)
{
char drive_n[MAXDRIVE], dir_n[MAXDIR], fname_n[MAXFILE], ext_n[MAXEXT];

if(*nf_bd=='\0') {
	*nf_exe='\0';
	return;
}

fnsplit(nf_bd,drive_n,dir_n,fname_n,ext_n);
strcpy(nf_exe,drive_n);
strcat(nf_exe,dir_n);
strcat(nf_exe,fname_n);
strcat(nf_exe,".EXE");
strupr(nf_exe);

}

/****************************************************************************
	IMPRIME_ERROR: imprime un mensaje de error y sale al sistema
	  operativo.
	  Entrada:      'msg' mensaje
****************************************************************************/
void imprime_error(char *msg)
{
STC_CUADRO c;
int i;

c_crea_cuadro(&c," ERROR ",C_CENT,C_CENT,ERR_ANCHO,ERR_ALTO,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,cfg.color_errboton,cfg.color_errinput,
  cfg.color_errsel,cfg.color_errtec,cfg.color_errtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,ERR_ALTO-4,(ERR_ANCHO-10)/2,"^Vale",8);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,msg,ERR_ANCHO-2,5,C_TXTLINEA,
  C_TXTNOBORDE);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);
c_elimina(&c);

vga_activa_fuente(NULL);
vga_parpadeo(1);
if(lks_eds==FALSE) g_modovideo(G_MV_T80C);
remove(nf_exe);
exit(1);

}

/****************************************************************************
	MAX_MEM: comprueba m†xima memoria disponible para funciones de
	  reserva de memoria (malloc).
	  Salida:	m†xima cantidad de memoria disponible
****************************************************************************/
unsigned long max_mem(void)
{
struct heapinfo hi;
unsigned long maxtam=0;

/* si hay errores en memoria, devuelve 0 */
if(heapcheck()<0) return(0);

hi.ptr=NULL;

while(heapwalk(&hi)==_HEAPOK) {
	if((!hi.in_use) && (hi.size>maxtam)) maxtam=hi.size;
}

return(maxtam);
}

/****************************************************************************
	COPIA_FICHERO: copia un fichero en otro.
	  Entrada:      'horg' handle de fichero origen
			'hdest' handle de fichero destino
	  Salida:       1 si pudo copiar, 0 si error
****************************************************************************/
void copia_fichero(int horg, int hdest)
{
char *buf, bufaux[256];
int bufaux_usado=0;
long tam=0xff00L, flng;

flng=filelength(horg);
if(flng<tam) tam=flng;

/* reserva memoria para buffer, si no hay suficiente memoria busca */
/* la m†xima cantidad disponible */
if((buf=(char *)malloc((size_t)tam))==NULL) {
	tam=max_mem();
	/* si a£n asi no pudo reservar memoria, usa buffer auxiliar */
	if((buf=(char *)malloc((size_t)tam))==NULL) {
		buf=bufaux;
		tam=sizeof(bufaux);
		bufaux_usado=1;
	}
}

while(!eof(horg)) {
	if((tam=read(horg,buf,(unsigned)tam))==-1) {
		close(horg);
		close(hdest);
		if(!bufaux_usado) free(buf);
		imprime_error(MsgErr_Lect);
	}
	if(write(hdest,buf,(unsigned)tam)==-1) {
		close(horg);
		close(hdest);
		if(!bufaux_usado) free(buf);
		imprime_error(MsgErr_Escr);
	}
}

if(!bufaux_usado) free(buf);

}

/****************************************************************************
	LINKAR: crea fichero ejecutable a partir de base de datos.
	  Entrada:    variables globales:-
			'nf_bd' nombre de fichero de base de datos
			'nf_exe' nombre de fichero ejecutable
****************************************************************************/
void linkar(void)
{
CAB_SINTAC cab;
char *srecon=SRECON;
char nf_runtime[MAXPATH];
long lng_runtime=0;
int hruntime, hexe, hbd;

esconde_cursor();

/* si no ha dado nombre de base de datos, sale */
if(!*nf_bd) return;

/* si no ha dado nombre de fichero ejecutable lo construye */
if(!*nf_exe) construye_nfexe();

/* nombre de fichero 'runtime' */
strcpy(nf_runtime,cfg.dir_sintac);
strcat(nf_runtime,"SINTAC.RUN");

/* longitud de m¢dulo 'runtime' */
if((hruntime=open(nf_runtime,O_BINARY | O_RDONLY))==-1)
  imprime_error(MsgErr_Rtime);
lng_runtime=filelength(hruntime);

if((hexe=open(nf_exe,O_BINARY | O_WRONLY | O_CREAT | O_TRUNC,
  S_IREAD | S_IWRITE))==-1) imprime_error(MsgErr_Exe);

/* copia m¢dulo 'runtime' en fichero ejecutable */
copia_fichero(hruntime,hexe);
close(hruntime);

/* copia base de datos en fichero ejecutable */
if((hbd=open(nf_bd,O_BINARY | O_RDONLY))==-1) {
	close(hexe);
	imprime_error(MsgErr_Bd);
}

/* lee cabecera */
if(read(hbd,&cab,sizeof(CAB_SINTAC))==-1) {
	close(hbd);
	close(hexe);
	imprime_error(MsgErr_Lect);
}

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1])) {
	close(hbd);
	close(hexe);
	imprime_error(MsgErr_Nobd);
}

lseek(hbd,0,SEEK_SET);
copia_fichero(hbd,hexe);
close(hbd);

/* pone longitud de m¢dulo 'runtime' al final del fichero ejecutable */
if(write(hexe,&lng_runtime,sizeof(long))==-1) {
	close(hexe);
	imprime_error(MsgErr_Escr);
}

close(hexe);

}
