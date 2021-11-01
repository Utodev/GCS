/****************************************************************************
			     INSTALACION SINTAC
			    (c)1995 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dir.h>
#include <errno.h>
#include <dos.h>
#include <bios.h>
#include "graf.h"
#include "ventana.h"
#include "raton.h"
#include "menu.h"
#include "cuadro.h"
#include "version.h"
#include "sintac.h"
#include "color.h"
#include "instalar.h"

/*** Variables globales ***/
/* tama¤o del STACK */
unsigned _stklen=8192;

STC_CFG cfg;                    /* almacena configuraci¢n */
char *Nf_Cfg=NF_CFG;            /* nombre de fichero de configuraci¢n */
char *OpcMenu=" ^Directorios                        | ^Instalar: ^Salir";
STC_VENTANA vcab;
STC_VENTANA vconfig;

/* datos de ficheros a instalar */
STC_DATFICH datf[NUM_FICH_INST];
int num_fich_inst=0;    /* n£mero de ficheros a instalar */

/* mensajes de error */
char *MsgErr_NoFDat=" Fichero INSTALAR.DAT no encontrado ";
char *MsgErr_FDat=" Error en fichero INSTALAR.DAT ";
char *MsgErr_Dir=" No se puede crear directorio ";
char *MsgErr_Fich=" Error al copiar fichero ";

/*** Programa principal ***/
void main(void)
{
STC_MENU *menu;
int i, salir=0, instalado=0, opcion;

/* si no pudo establecer modo de v¡deo */
if(!g_modovideo(G_MV_T80C)) {
	printf("\nEste programa requiere tarjeta CGA o mejor.\n");
	exit(1);
}

v_crea(&vcab,0,0,80,3,COLOR_MEN,COLOR_MENS1,COLOR_MENS2,NULL,0);
v_crea(&vconfig,3,0,80,22,COLOR_MEN,COLOR_MENS1,COLOR_MENS2,NULL,0);
m_color(COLOR_MEN,COLOR_MENS1,COLOR_MENS2,COLOR_MENTEC,COLOR_MENSEL);
menu=m_crea(MENU_VERT | MENU_FIJO,NULL,OpcMenu,MENU_FIL,MENU_COL,0);

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

/* inicializa configuraci¢n */
inicializa_cfg();

v_abre(&vcab);
v_pon_cursor(&vcab,0,9);
v_impcad(&vcab,"Instalaci¢n del sistema "COPYRIGHT,V_NORELLENA);
v_abre(&vconfig);
m_abre(menu);

while(!salir) {
	opcion=m_elige_opcion(menu);
	switch(opcion) {
		case 0 :
			directorios();
			break;
		case 2 :
			instalar();
			instalado=1;
			salir=1;
			break;
		case 3 :
			salir=1;
			break;
	}
};

m_elimina(menu);
v_cierra(&vconfig);
v_cierra(&vcab);

r_puntero(R_OCULTA);
g_modovideo(G_MV_T80C);

i=strlen(cfg.dir_sintac);
if(cfg.dir_sintac[i-1]=='\\') cfg.dir_sintac[i-1]='\0';

if(instalado) printf("\nEl sistema SINTAC se ha instalado satisfactoriamente\n"
  "en el directorio: %s\n"
  "Comprueba que el fichero CONFIG.SYS contenga: FILES=20\n"
  "Consulta el fichero LEEME.TXT para cambios de £ltima hora\n",
  cfg.dir_sintac);

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
	INICIALIZA_CFG: inicializa configuraci¢n asignando valores por
	  defecto.
****************************************************************************/
void inicializa_cfg(void)
{

/* inicializa directorios */
strcpy(cfg.dir_sintac,"C:\\SINTAC\\");
strcpy(cfg.dir_bd,"C:\\SINTAC\\DATOS\\");
strcpy(cfg.dir_util,"C:\\SINTAC\\UTIL\\");

/* inicializa colores */
cfg.color_men=COLOR_MEN;
cfg.color_mens1=COLOR_MENS1;
cfg.color_mens2=COLOR_MENS2;
cfg.color_mentec=COLOR_MENTEC;
cfg.color_mensel=COLOR_MENSEL;

cfg.color_ved=COLOR_VED;
cfg.color_veds1=COLOR_VEDS1;
cfg.color_veds2=COLOR_VEDS2;
cfg.color_vedblq=COLOR_VEDBLQ;
cfg.color_vedcoment=COLOR_VEDCOMENT;
cfg.color_vedesp=COLOR_VEDESP;
cfg.color_vedpalclv=COLOR_VEDPALCLV;

cfg.color_dlg=COLOR_DLG;
cfg.color_dlgs1=COLOR_DLGS1;
cfg.color_dlgs2=COLOR_DLGS2;
cfg.color_dlgboton=COLOR_DLGBOTON;
cfg.color_dlginput=COLOR_DLGINPUT;
cfg.color_dlgsel=COLOR_DLGSEL;
cfg.color_dlgtec=COLOR_DLGTEC;
cfg.color_dlgtecboton=COLOR_DLGTECBOTON;

cfg.color_ayd=COLOR_AYD;
cfg.color_ayds1=COLOR_AYDS1;
cfg.color_ayds2=COLOR_AYDS2;
cfg.color_aydboton=COLOR_AYDBOTON;
cfg.color_aydinput=COLOR_AYDINPUT;
cfg.color_aydsel=COLOR_AYDSEL;
cfg.color_aydtec=COLOR_AYDTEC;
cfg.color_aydtecboton=COLOR_AYDTECBOTON;

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
	AJUSTA_NDIR: ajusta ruta a directorio a¤adiendo un '\' al final
	  si no lo tiene, y adem s convierte a may£sculas.
	  Entrada:      'dir' puntero a cadena con la ruta
****************************************************************************/
void ajusta_ndir(char *dir)
{
char *c;

/* va hasta el final de la cadena */
for(c=dir; *c; c++);

/* si no tiene '\' al final, se la a¤ade */
if(*(c-1)!='\\') {
	*c='\\';
	*(c+1)='\0';
}

/* pasa cadena a may£sculas */
strupr(dir);

}

/****************************************************************************
	DIRECTORIOS: fija los directorios de trabajo.
****************************************************************************/
void directorios(void)
{
STC_CUADRO cdir;
int i;

c_crea_cuadro(&cdir," Directorios del SINTAC ",C_CENT,C_CENT,CDIR_ANCHO,
  CDIR_ALTO,COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,COLOR_DLGBOTON,COLOR_DLGINPUT,
  COLOR_DLGSEL,COLOR_DLGTEC,COLOR_DLGTECBOTON);
c_crea_elemento(&cdir,C_ELEM_INPUT,1,16,"^SINTAC",CDIR_ANCHO-19,
  cfg.dir_sintac,MAXPATH-1);
c_crea_elemento(&cdir,C_ELEM_INPUT,3,16,"^Bases de datos",CDIR_ANCHO-19,
  cfg.dir_bd,MAXPATH-1);
c_crea_elemento(&cdir,C_ELEM_INPUT,5,16,"^Utilidades",CDIR_ANCHO-19,
  cfg.dir_util,MAXPATH-1);
c_crea_elemento(&cdir,C_ELEM_BOTON,7,28,"^Vale",8);

c_abre(&cdir);
do {
	i=c_gestiona(&cdir);
} while((i!=-1) && (i!=3));
c_cierra(&cdir);
c_elimina(&cdir);

ajusta_ndir(cfg.dir_sintac);
ajusta_ndir(cfg.dir_bd);
ajusta_ndir(cfg.dir_util);

}

/****************************************************************************
	IMPRIME_ERROR: imprime un mensaje de error y sale al sistema
	  operativo.
	  Entrada:      'msg' mensaje
****************************************************************************/
void imprime_error(char *msg)
{
STC_CUADRO c;
int i, ancho;

ancho=strlen(msg)+4;

c_crea_cuadro(&c," ERROR ",C_CENT,C_CENT,ancho,8,COLOR_ERR,COLOR_ERRS1,
  COLOR_ERRS2,COLOR_ERRBOTON,COLOR_ERRINPUT,COLOR_ERRSEL,COLOR_ERRTEC,
  COLOR_ERRTECBOTON);
c_crea_elemento(&c,C_ELEM_BOTON,3,(ancho-10)/2,"^Vale",8);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,msg,ancho-2,3,C_TXTLINEA,C_TXTNOBORDE);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);
c_elimina(&c);

fcloseall();
g_modovideo(G_MV_T80C);
exit(1);

}

/****************************************************************************
	LEE_DATOS_FICH: lee los datos de los ficheros a instalar.
	  Entrada:      'nf_datfich' nombre del fichero con los datos
			de ficheros a instalar
	  Salida:     Variables globales:-
			'num_fich_inst' n£mero de ficheros a instalar
			'datf[i]' con los datos de los ficheros
			si hay alg£n error lo imprime y sale al sistema
			operativo
****************************************************************************/
void lee_datos_fich(char *nf_datfich)
{
FILE *ffich;
int i=0;

/* abre fichero con datos de ficheros a instalar */
if((ffich=fopen(nf_datfich,"rt"))==NULL) imprime_error(MsgErr_NoFDat);

/* lee datos de ficheros */
while(i<NUM_FICH_INST) {
	if(fscanf(ffich,"%s %u %u %s\n",&datf[i].nombre,&datf[i].tipo,
	  &datf[i].disco,&datf[i].subdir)!=4) {
		if(feof(ffich)) break;
		else imprime_error(MsgErr_FDat);
	}

	if((datf[i].tipo<0) || (datf[i].tipo>9)) imprime_error(MsgErr_FDat);
	if((datf[i].disco<0) || (datf[i].disco>9)) imprime_error(MsgErr_FDat);

	i++;
}

fclose(ffich);

num_fich_inst=i;

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
	COPIA: copia un fichero en otro.
	  Entrada:      'origen' fichero de origen
			'destino' fichero destino
	  Salida:       0 si no hubo errores o 1 si lo hubo
****************************************************************************/
int copia(char *origen, char *destino)
{
long tam=0xff00L;
char *buff, bufaux[256];
int forg, fdest, bufaux_usado=0;

/* abre fichero de origen */
if((forg=open(origen,O_BINARY | O_RDONLY))==-1) return(1);

/* crea fichero de destino, si existe lo borra */
if((fdest=open(destino,O_BINARY | O_WRONLY | O_CREAT,
  S_IREAD | S_IWRITE))==-1) return(1);

if(filelength(forg)<tam) tam=filelength(forg);

/* reserva un buffer de memoria, si no hay memoria suficiente */
/* reserva la m xima posible */
if((buff=(char *)malloc((size_t)tam))==NULL) {
	tam=max_mem();
	if((buff=(char *)malloc((size_t)tam))==NULL) {
		buff=bufaux;
		tam=sizeof(bufaux);
		bufaux_usado=1;
	}
}

/* lee-escribe hasta final de fichero */
while(!eof(forg)) {
	if((tam=read(forg,buff,(unsigned)tam))==-1) {
		close(forg);
		close(fdest);
		if(!bufaux_usado) free(buff);
		return(1);
	}
	if(write(fdest,buff,(unsigned)tam)==-1) {
		close(forg);
		close(fdest);
		if(!bufaux_usado) free(buff);
		return(1);
	}
}

/* cierra ficheros y libera memoria */
close(forg);
close(fdest);
if(!bufaux_usado) free(buff);

return(0);
}

/****************************************************************************
	BEEP: produce un pitido en el altavoz del PC.
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
	INSTALAR: rutina principal de instalaci¢n.
****************************************************************************/
void instalar(void)
{
STC_VENTANA v;
char nombre_fich[MAXPATH], unidad[MAXDRIVE], dir[MAXDIR], nombref[MAXFILE],
  ext[MAXEXT], msg[41];
FILE *fich;
int i, disco_act=0;

esconde_cursor();

v_crea(&v,13,20,40,7,COLOR_DLG,COLOR_DLGS2,COLOR_DLGS1,NULL,0);
v_abre(&v);

/* coge datos de ficheros a instalar */
lee_datos_fich("INSTALAR.DAT");

/* crea directorios de destino */
v_pon_cursor(&v,2,4);
v_impcad(&v,"Creando directorios de destino",V_NORELLENA);

/* SINTAC */
strcpy(nombre_fich,cfg.dir_sintac);
i=strlen(nombre_fich);
if(nombre_fich[i-1]=='\\') nombre_fich[i-1]='\0';
if((mkdir(nombre_fich)==-1) && (errno!=EACCES)) imprime_error(MsgErr_Dir);

/* BASES DE DATOS */
strcpy(nombre_fich,cfg.dir_bd);
i=strlen(nombre_fich);
if(nombre_fich[i-1]=='\\') nombre_fich[i-1]='\0';
if((mkdir(nombre_fich)==-1) && (errno!=EACCES)) imprime_error(MsgErr_Dir);

/* UTILIDADES */
strcpy(nombre_fich,cfg.dir_util);
i=strlen(nombre_fich);
if(nombre_fich[i-1]=='\\') nombre_fich[i-1]='\0';
if((mkdir(nombre_fich)==-1) && (errno!=EACCES)) imprime_error(MsgErr_Dir);

/* crea fichero de configuraci¢n */
v_borra(&v);
v_pon_cursor(&v,2,3);
v_impcad(&v,"Creando fichero de configuraci¢n",V_NORELLENA);
strcpy(nombre_fich,cfg.dir_sintac);
strcat(nombre_fich,Nf_Cfg);
if((fich=fopen(nombre_fich,"wb"))==NULL) imprime_error(MsgErr_Fich);
if(fwrite(&cfg,sizeof(STC_CFG),1,fich)!=1) imprime_error(MsgErr_Fich);
fclose(fich);

/* copia ficheros a sus directorios correspondientes, seg£n tipo */
v_borra(&v);
v_pon_cursor(&v,2,10);
v_impcad(&v,"Copiando ficheros",V_NORELLENA);

for(i=0; i<num_fich_inst; i++) {
	v_borra(&v);
	v_pon_cursor(&v,1,11);
	v_impcad(&v,"Copiando ficheros",V_NORELLENA);
	sprintf(msg,"Fichero %02i de %02i",i+1,num_fich_inst);
	v_pon_cursor(&v,3,11);
	v_impcad(&v,msg,V_NORELLENA);

	/* asigna directorio de destino seg£n tipo de fichero */
	switch(datf[i].tipo) {
		case 0 :
			strcpy(nombre_fich,cfg.dir_sintac);
			break;
		case 1 :
			strcpy(nombre_fich,cfg.dir_bd);
			break;
		case 2 :
			strcpy(nombre_fich,cfg.dir_util);
			break;
		default :
			strcpy(nombre_fich,cfg.dir_sintac);
	}

	/* comprueba subdirectorio de destino de fichero */
	if(strcmp(datf[i].subdir,".")) {
		strcat(nombre_fich,datf[i].subdir);
		if((mkdir(nombre_fich)==-1) && (errno!=EACCES))
		  imprime_error(MsgErr_Dir);
		strcat(nombre_fich,"\\");
	}

	/* comprueba si hay que cambiar de disco */
	if(datf[i].disco!=disco_act) {
		disco_act=datf[i].disco;
		sprintf(msg,"Inserta disco %i y pulsa una tecla",disco_act+1);
		v_pon_cursor(&v,4,2);
		v_impcad(&v,msg,V_NORELLENA);
		beep();
		bioskey(0);
		v_pon_cursor(&v,4,0);
		v_impcad(&v,"",V_RELLENA);
	}

	/* crea nombre de fichero de destino */
	fnsplit(datf[i].nombre,unidad,dir,nombref,ext);
	strcat(nombre_fich,nombref);
	strcat(nombre_fich,ext);

	if(copia(datf[i].nombre,nombre_fich)==1) imprime_error(MsgErr_Fich);
}

v_cierra(&v);

}
