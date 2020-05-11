/****************************************************************************
			CONVERSOR DE GRAFICOS SINTAC
			    (c)1995 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>
#include <io.h>
#include <dir.h>
#include <bios.h>
#include <dos.h>
#include "graf.h"
#include "rutvga.h"
#include "ventana.h"
#include "raton.h"
#include "cuadro.h"
#include "grfpcx.h"
#include "sintac.h"
#include "color.h"
#include "grfs.h"

/*** Variables externas ***/
extern char *c_Borde_Sel;
extern char *c_Esp;

/*** Variables globales ***/
STC_CFG cfg;

/* nombre de fichero de entrada */
char nf_grf[MAXPATH];

/* mensajes de error */
char *err_grf[]={
	"",
	" No existe fichero GRF ",
	" Fichero GRF no v lido ",
	" Error de apertura de fichero PCX ",
	" Error de lectura de fichero PCX ",
	" Fichero PCX no v lido ",
	" Error de lectura de fichero GRF ",
	" Error de escritura en fichero GRF ",
	" N£mero de imagen repetido ",
	" No existe imagen en fichero ",
	" Error en fichero temporal ",
};
char *err_grfpcx[]={
	"",
	" Error de apertura de fichero ",
	" Error de lectura de fichero ",
	" Formato de fichero no v lido ",
	" Formato de imagen no v lido ",
	" Modo de pantalla incorrecto ",
	" No hay memoria suficiente para imagen ",
	" No existe esa imagen en fichero ",
};

/*** Programa principal ***/
void main(int argc, char *argv[])
{
STC_CUADRO cgrf;
STC_ELEM *e;
STC_ELEM_LISTA *lst;
STC_CABGRF grf;
STC_CABIMG img;
FILE *fgrf;
char nf_pcx[MAXPATH], buff[81], num[4],
  *cab=" CONVERSOR DE GRAFICOS SINTAC versi¢n "GRFS_VERSION"  (c)1995 JSJ Soft Ltd. ";
int i, salir=0, n_img, colores;

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

if(!*nf_grf) c_selecc_ficheros(C_CENT,C_CENT," Fichero GRF ",cfg.color_cs,
  cfg.color_css1,cfg.color_css2,cfg.color_csboton,cfg.color_csinput,
  cfg.color_cstec,cfg.color_cstecboton,cfg.color_cssel,"","*.*",nf_grf);

esconde_cursor();

if(*nf_grf) {
	if((fgrf=fopen(nf_grf,"rb+"))==NULL) {
		if(pregunta_siono(" Fichero GRF no existe, ¨crearlo? ")) {
			if((fgrf=fopen(nf_grf,"wb+"))==NULL) {
				imprime_error(" No se puede crear fichero GRF ");
				salir=1;
			}
			else {
				strcpy(grf.id,GRF_ID);
				grf.n_img=0;
				fwrite(&grf,sizeof(STC_CABGRF),1,fgrf);
			}
		}
		else salir=1;
	}
	else {
		/* comprueba si es fichero GRF */
		fread(&grf,sizeof(STC_CABGRF),1,fgrf);
		if(strcmp(grf.id,GRF_ID)) {
			imprime_error(" Fichero GRF no v lido ");
			salir=1;
		}
	}
	fclose(fgrf);
}
else salir=1;

c_crea_cuadro(&cgrf,cab,C_CENT,C_CENT,GRFS_ANCHO,GRFS_ALTO,cfg.color_cs,
  cfg.color_css1,cfg.color_css2,cfg.color_csboton,cfg.color_csinput,
  cfg.color_cssel,cfg.color_cstec,cfg.color_cstecboton);
c_crea_elemento(&cgrf,C_ELEM_BOTON,1,51,"^A¤adir",10);
c_crea_elemento(&cgrf,C_ELEM_BOTON,1,65,"^Eliminar",12);
c_crea_elemento(&cgrf,C_ELEM_BOTON,3,51,"^Visualizar",14);
c_crea_elemento(&cgrf,C_ELEM_BOTON,3,68,"^Salir",9);
e=c_crea_elemento(&cgrf,C_ELEM_LISTA,0,0," ^Im genes ",50,GRFS_ALTO-2,
  C_LSTNORMAL,C_LSTSINORDEN);
lst=(STC_ELEM_LISTA *)(e->info);
strcpy(num,"0");
c_crea_elemento(&cgrf,C_ELEM_INPUT,7,62,"^N§ imagen",6,num,3);

if(!salir) c_abre(&cgrf);
while(!salir) {
	c_borra_lista(lst);
	fgrf=fopen(nf_grf,"rb");
	fread(&grf,sizeof(STC_CABGRF),1,fgrf);
	for(i=0; i<grf.n_img; i++) {
		fread(&img,sizeof(STC_CABIMG),1,fgrf);
		colores=1 << img.bits_pixel*img.num_planos;
		sprintf(buff,"[%3i]  %7li bytes  %4ix%4i  %3i colores",
		  img.num,img.tam_img,img.ventana[2]-img.ventana[0]+1,
		  img.ventana[3]-img.ventana[1]+1,colores);
		c_mete_en_lista(lst,buff);
		fseek(fgrf,img.tam_img,SEEK_CUR);
	}
	fclose(fgrf);
	c_dibuja_elemento(&cgrf,e);

	i=c_gestiona(&cgrf);
	switch(i) {
		case 0 :	/* a¤adir */
			n_img=atoi(num);
			if(n_img<256) {
				c_selecc_ficheros(C_CENT,C_CENT," Fichero PCX ",
				  cfg.color_cs,cfg.color_css1,cfg.color_css2,
				  cfg.color_csboton,cfg.color_csinput,
				  cfg.color_cstec,cfg.color_cstecboton,
				  cfg.color_cssel,"","*.PCX",nf_pcx);
				esconde_cursor();
				if((i=pon_imgpcx(nf_grf,nf_pcx,n_img))!=0)
				  imprime_error(err_grf[i]);
				else if(n_img<255) itoa(n_img+1,num,10);
			}
			else imprime_error(" N£mero de imagen no v lido ");
			break;
		case 1 :	/* eliminar */
			if(!lst->num_elementos) break;
			n_img=atoi(lst->selecc+1);
			if(pregunta_siono(" ¨Est s seguro? ")) {
				esconde_cursor();
				if((i=borra_imgpcx(nf_grf,n_img))!=0)
				  imprime_error(err_grf[i]);
			}
			break;
		case 2 :	/* visualizar */
			if(!lst->num_elementos) break;
			n_img=atoi(lst->selecc+1);
			colores=atoi(lst->selecc+33);
			r_puntero(R_OCULTA);
			c_cierra(&cgrf);

			if(colores==256) {
				g_modovideo(G_MV_G1C256);
				i=grf_visualiza(nf_grf,n_img,0,0,0,0,0);
			}
			else {
				g_modovideo(G_MV_G3C16);
				i=grf_visualiza(nf_grf,n_img,0,0,0,0,0);
			}

			if(!i) pausa();
			activa_vga();
			r_puntero(R_MUESTRA);
			if(i) imprime_error(err_grfpcx[i]);
			c_abre(&cgrf);
			break;
		case 3 :	/* salir */
		case -1:
			salir=1;
			break;
	}
}

c_borra_lista(lst);
c_cierra(&cgrf);
c_elimina(&cgrf);
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
			'nf_grf' nombre de fichero de entrada
			indicadores correspondientes actualizados
****************************************************************************/
int analiza_args(int argc, char *argv[])
{
int ppar=1;

*nf_grf='\0';

while(ppar<argc) {
	/* si no empieza por '/' o '-' */
	if((*argv[ppar]!='/') && (*argv[ppar]!='-')) {
		if(*nf_grf=='\0') {
			strcpy(nf_grf,argv[ppar]);
			strupr(nf_grf);
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
	PAUSA: realiza una pausa hasta que se pulse una tecla o un
	  bot¢n del rat¢n.
****************************************************************************/
void pausa(void)
{
STC_RATON r;

do {
	if(bioskey(1)) {
		bioskey(0);
		break;
	}
	r_estado(&r);
} while(!r.boton1 && !r.boton2);

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
	MAX_MEM: comprueba m xima memoria disponible para funciones de
	  reserva de memoria (malloc)
	  Salida:	m xima cantidad de memoria disponible
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
	PON_IMGPCX: mete una imagen en formato PCX en un fihero GRF.
	  Entrada:	'nf_grf' nombre del fichero GRF
			'nf_pcx' nombre del fichero PCX
			'n_img' n£mero de la imagen
	  Salida: 	0 si no hubo error, o un c¢digo de error si lo
			hubo:
			  ERR_NOEXGRF : no existe fichero GRF
			  ERR_GRFNOV  : fichero GRF no v lido
			  ERR_APERPCX : error de apertura fichero PCX
			  ERR_LECTPCX : error de lectura fichero PCX
			  ERR_PCXNOV  : fichero PCX no v lido
			  ERR_LECTGRF : error de lectura fichero GRF
			  ERR_ESCRGRF : error de escritura fichero GRF
			  ERR_IMGREP  : n£mero de imagen repetido
****************************************************************************/
int pon_imgpcx(char *nf_grf, char *nf_pcx, int n_img)
{
STC_CABGRF grf;
STC_CABIMG img;
STC_CABPCX pcx;
FILE *fgrf, *fpcx;
fpos_t fpos;
int i, num_colores, bpcx, bufaux_usado=0;
long j, tam=0xff00L;
BYTE *buf, bufaux[256];

if((fgrf=fopen(nf_grf,"rb+"))!=NULL) {
	if(fread(&grf,sizeof(STC_CABGRF),1,fgrf)!=1) {
		fclose(fgrf);
		return(ERR_LECTGRF);
	}
	if(strcmp(grf.id,GRF_ID)) {
		fclose(fgrf);
		return(ERR_GRFNOV);
	}
}
else return(ERR_NOEXGRF);

/* comprueba si n£mero de imagen repetido */
for(i=0; i<grf.n_img; i++) {
	fread(&img,sizeof(STC_CABIMG),1,fgrf);
	if(n_img==img.num) {
		fclose(fgrf);
		return(ERR_IMGREP);
	}
	fseek(fgrf,img.tam_img,SEEK_CUR);
}

/* posiciona al final del fichero para a¤adir nueva imagen */
if(fseek(fgrf,0,SEEK_END)) {
	fclose(fgrf);
	return(ERR_LECTGRF);
}

if((fpcx=fopen(nf_pcx,"rb"))==NULL) {
	fclose(fgrf);
	return(ERR_APERPCX);
}

if(fread(&pcx,sizeof(STC_CABPCX),1,fpcx)!=1) {
	fclose(fgrf);
	fclose(fpcx);
	return(ERR_LECTPCX);
}

if(pcx.propietario!=0x0a) {
	fclose(fgrf);
	fclose(fpcx);
	return(ERR_PCXNOV);
}

/* n£mero de imagen */
img.num=n_img;
/* n£mero de bits por pixel */
img.bits_pixel=pcx.bits_pixel;
/* tama¤o dibujo (izq., arr., der., abajo) */
for(i=0; i<4; i++) img.ventana[i]=pcx.ventana[i];
/* n£mero de planos de color */
img.num_planos=pcx.num_planos;
/* n£mero de bytes por plano de l¡nea scan */
img.bytes_scan=pcx.bytes_scan;

/* tama¤o imagen (bytes) sin decodificar */
img.tam_img=filelength(fileno(fpcx))-sizeof(STC_CABPCX);

/* recoge informaci¢n de paleta */
for(i=0; i<256; i++) {
	img.paleta[i][0]=0;
	img.paleta[i][1]=0;
	img.paleta[i][2]=0;
}
num_colores=1 << (pcx.bits_pixel*pcx.num_planos);
if(num_colores<=16) {
	for(i=0; i<num_colores; i++) {
		img.paleta[i][0]=(BYTE)(pcx.paleta[i][0] >> 2);
		img.paleta[i][1]=(BYTE)(pcx.paleta[i][1] >> 2);
		img.paleta[i][2]=(BYTE)(pcx.paleta[i][2] >> 2);
	}
}
else {
	/* guarda el puntero del fichero */
	if(fgetpos(fpcx,&fpos)) {
		fclose(fpcx);
		fclose(fgrf);
		return(ERR_LECTPCX);
	}

	/* pone puntero del fichero al comienzo de informaci¢n de paleta */
	if(fseek(fpcx,-769L,SEEK_END)) {
		fclose(fpcx);
		fclose(fgrf);
		return(ERR_LECTPCX);
	}

	/* lee 1er byte de informaci¢n de cabecera y comprueba que sea 12 */
	if((bpcx=fgetc(fpcx))==EOF) {
		fclose(fpcx);
		fclose(fgrf);
		return(ERR_LECTPCX);
	}
	if(bpcx!=12) {
		fclose(fpcx);
		fclose(fgrf);
		return(ERR_PCXNOV);
	}

	/* lee y convierte la informaci¢n de paleta */
	for(i=0; i<num_colores; i++) {
		/* componente rojo */
		if((bpcx=fgetc(fpcx))==EOF) {
			fclose(fpcx);
			fclose(fgrf);
			return(ERR_LECTPCX);
		}
		img.paleta[i][0]=(BYTE)(bpcx >> 2);
		/* componente verde */
		if((bpcx=fgetc(fpcx))==EOF) {
			fclose(fpcx);
			fclose(fgrf);
			return(ERR_LECTPCX);
		}
		img.paleta[i][1]=(BYTE)(bpcx >> 2);
		/* componente azul */
		if((bpcx=fgetc(fpcx))==EOF) {
			fclose(fpcx);
			fclose(fgrf);
			return(ERR_LECTPCX);
		}
		img.paleta[i][2]=(BYTE)(bpcx >> 2);
	}

	/* recupera el puntero del fichero */
	if(fsetpos(fpcx,&fpos)) {
		fclose(fpcx);
		fclose(fgrf);
		return(ERR_LECTPCX);
	}

	/* ajusta tama¤o imagen sin codificar */
	img.tam_img-=769;
}

/* cabecera de imagen */
if(fwrite(&img,sizeof(STC_CABIMG),1,fgrf)!=1) {
	fclose(fpcx);
	fclose(fgrf);
	return(ERR_ESCRGRF);
}

/* reserva memoria para buffer, si no hay suficiente memoria busca */
/* la m xima cantidad disponible */
if((buf=(BYTE *)malloc((size_t)tam))==NULL) {
	tam=max_mem();
	/* si a£n asi no pudo reservar memoria, usa buffer auxiliar */
	if((buf=(BYTE *)malloc((size_t)tam))==NULL) {
		buf=bufaux;
		tam=sizeof(bufaux);
		bufaux_usado=1;
	}
}

/* lee imagen sin codificar y la almacena */
j=img.tam_img;
if(j<tam) tam=j;
while(j) {
	if(fread(buf,sizeof(BYTE),(size_t)tam,fpcx)!=(size_t)tam) {
		fclose(fpcx);
		fclose(fgrf);
		if(!bufaux_usado) free(buf);
		return(ERR_LECTPCX);
	}
	if(fwrite(buf,sizeof(BYTE),(size_t)tam,fgrf)!=(size_t)tam) {
		fclose(fpcx);
		fclose(fgrf);
		if(!bufaux_usado) free(buf);
		return(ERR_ESCRGRF);
	}

	j-=tam;
	if(j && (j<tam)) tam=j;
}

if(!bufaux_usado) free(buf);

fclose(fpcx);
grf.n_img++;

/* almacena cabecera definitiva */
if(fseek(fgrf,0,SEEK_SET)){
	fclose(fgrf);
	return(ERR_LECTGRF);
}
if(fwrite(&grf,sizeof(STC_CABGRF),1,fgrf)!=1) {
	fclose(fgrf);
	return(ERR_ESCRGRF);
}
fclose(fgrf);

return(0);
}

/****************************************************************************
	BORRA_IMGPCX: borra una imagen de un fihero GRF.
	  Entrada:	'nf_grf' nombre del fichero GRF
			'n_img' n£mero de la imagen
	  Salida: 	0 si no hubo error, o un c¢digo de error si lo
			hubo:
			  ERR_NOEXGRF : no existe fichero GRF
			  ERR_GRFNOV  : fichero GRF no v lido
			  ERR_LECTGRF : error de lectura fichero GRF
			  ERR_ESCRGRF : error de escritura fichero GRF
			  ERR_NOEXIMG : no existe esa imagen en fichero
			  ERR_FTEMP   : error en fichero temporal
****************************************************************************/
int borra_imgpcx(char *nf_grf, int n_img)
{
STC_CABGRF grf;
STC_CABIMG img;
FILE *fgrf, *ftemp;
char nf_temp[13];
int i, bufaux_usado=0;
long j, tam=0xff00L;
BYTE *buf, bufaux[256];

if((fgrf=fopen(nf_grf,"rb+"))!=NULL) {
	if(fread(&grf,sizeof(STC_CABGRF),1,fgrf)!=1) {
		fclose(fgrf);
		return(ERR_LECTGRF);
	}
	if(strcmp(grf.id,GRF_ID)) {
		fclose(fgrf);
		return(ERR_GRFNOV);
	}
}
else return(ERR_NOEXGRF);

/* crea fichero temporal */
tmpnam(nf_temp);
if((ftemp=fopen(nf_temp,"wb"))==NULL) {
	fclose(fgrf);
	return(ERR_FTEMP);
}
/* escribe cabecera en fichero temporal */
grf.n_img--;
if(fwrite(&grf,sizeof(STC_CABGRF),1,ftemp)!=1) {
		fclose(fgrf);
		fclose(ftemp);
		remove(nf_temp);
		return(ERR_FTEMP);
}
grf.n_img++;

/* reserva memoria para buffer, si no hay suficiente memoria busca */
/* la m xima cantidad disponible */
if((buf=(BYTE *)malloc((size_t)tam))==NULL) {
	tam=max_mem();
	/* si a£n asi no pudo reservar memoria, usa buffer auxiliar */
	if((buf=(BYTE *)malloc((size_t)tam))==NULL) {
		buf=bufaux;
		tam=sizeof(bufaux);
		bufaux_usado=1;
	}
}

/* busca imagen */
for(i=0; i<grf.n_img; i++) {
	if(fread(&img,sizeof(STC_CABIMG),1,fgrf)!=1) {
		fclose(fgrf);
		fclose(ftemp);
		remove(nf_temp);
		if(!bufaux_usado) free(buf);
		return(ERR_LECTGRF);
	}

	if(n_img!=img.num) {
		if(fwrite(&img,sizeof(STC_CABIMG),1,ftemp)!=1) {
			fclose(fgrf);
			fclose(ftemp);
			remove(nf_temp);
			if(!bufaux_usado) free(buf);
			return(ERR_FTEMP);
		}
	}
	else break;

	j=img.tam_img;
	if(j<tam) tam=j;
	while(j) {
		if(fread(buf,sizeof(BYTE),(size_t)tam,fgrf)!=(size_t)tam) {
			fclose(fgrf);
			fclose(ftemp);
			remove(nf_temp);
			if(!bufaux_usado) free(buf);
			return(ERR_LECTGRF);
		}
		if(fwrite(buf,sizeof(BYTE),(size_t)tam,ftemp)!=(size_t)tam) {
			fclose(fgrf);
			fclose(ftemp);
			remove(nf_temp);
			if(!bufaux_usado) free(buf);
			return(ERR_FTEMP);
		}

		j-=tam;
		if(j && (j<tam)) tam=j;
	}
}
if(i==grf.n_img) {
	fclose(fgrf);
	fclose(ftemp);
	remove(nf_temp);
	if(!bufaux_usado) free(buf);
	return(ERR_NOEXIMG);
}

/* salta imagen que se quiere borrar */
if(fseek(fgrf,img.tam_img,SEEK_CUR)) {
	fclose(fgrf);
	fclose(ftemp);
	remove(nf_temp);
	if(!bufaux_usado) free(buf);
	return(ERR_LECTGRF);
}

/* copia resto del fichero */
grf.n_img--;
for(; i<grf.n_img; i++) {
	if(fread(&img,sizeof(STC_CABIMG),1,fgrf)!=1) {
		fclose(fgrf);
		fclose(ftemp);
		remove(nf_temp);
		if(!bufaux_usado) free(buf);
		return(ERR_LECTGRF);
	}
	if(fwrite(&img,sizeof(STC_CABIMG),1,ftemp)!=1) {
		fclose(fgrf);
		fclose(ftemp);
		remove(nf_temp);
		if(!bufaux_usado) free(buf);
		return(ERR_FTEMP);
	}

	j=img.tam_img;
	if(j<tam) tam=j;
	while(j) {
		if(fread(buf,sizeof(BYTE),(size_t)tam,fgrf)!=(size_t)tam) {
			fclose(fgrf);
			fclose(ftemp);
			remove(nf_temp);
			if(!bufaux_usado) free(buf);
			return(ERR_LECTGRF);
		}
		if(fwrite(buf,sizeof(BYTE),(size_t)tam,ftemp)!=(size_t)tam) {
			fclose(fgrf);
			fclose(ftemp);
			remove(nf_temp);
			if(!bufaux_usado) free(buf);
			return(ERR_FTEMP);
		}

		j-=tam;
		if(j && (j<tam)) tam=j;
	}
}

fclose(fgrf);
fclose(ftemp);
if(!bufaux_usado) free(buf);

remove(nf_grf);
rename(nf_temp,nf_grf);

return(0);
}
