/****************************************************************************
			 ENTORNO DE DESARROLLO SINTAC
			     (c)1995 JSJ Soft Ltd.
****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <process.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <ctype.h>
#include <bios.h>
#include <dos.h>
#include "graf.h"
#include "rutvga.h"
#include "ventana.h"
#include "raton.h"
#include "menu.h"
#include "cuadro.h"
#include "editor.h"
#include "ayuda.h"
#include "version.h"
#include "sintac.h"
#include "color.h"
#include "eds.h"

/*** Variables externas ***/
extern char *c_Borde_Sel;
extern char *c_Esp;

/*** Variables globales ***/
/* tama¤o del STACK */
unsigned _stklen=10240;

STC_CFG cfg;
STC_MENU *m;

/* marcas de secci¢n */
char *Mconst="ORG";
char *Mfin_Secc="\\END";
char *Mmsy="\\MSY";
char *Mvoc="\\VOC";
char *Mmsg="\\MSG";
char *Mloc="\\LOC";
char *Mobj="\\OBJ";
char *Mpro="\\PRO";

/* posici¢n del cursor de edici¢n dentro de cada secci¢n */
CURSOR_ED cur_org;
CURSOR_ED cur_msy;
CURSOR_ED cur_voc;
CURSOR_ED cur_msg[256];
CURSOR_ED cur_loc;
CURSOR_ED cur_obj;
CURSOR_ED cur_pro[256];

/* nombres de ficheros */
char *Nf_Bdtemp="$BD";
char *Nf_Bdaux="$BDAUX";
char *Nf_Secc="$%s";
char *Nf_Seccmult="$%s.%i";
char *Nf_Err=NF_ERR;

/* cadena con n£meros */
char *CNum="0123456789";

/* argumentos de llamada al compilador */
char *Arg_Cs="-E";

/* argumentos de llamada al int‚rprete-debugger */
char *Arg_Ds1="-L00";
char *Arg_Ds2="-E";

/* argumentos de llamada al linkador */
char *Arg_Lks="-E";

/* nombre de fichero de base de datos en uso */
char base_datos[MAXPATH];

/* indicador de base de datos modificada y fichero temporal actualizado */
BOOLEAN bd_modificada=FALSE;
BOOLEAN ft_actualizado=TRUE;

/* tablas para comprobar existencia de ficheros temporales de */
/* secciones m£ltiples */
BOOLEAN pro_existe[MAX_PRO];
BOOLEAN msg_existe[MAX_TMSG];

/* mensajes */
char *Merr_Lect=" Error de lectura de fichero ";
char *Merr_Escr=" Error de escritura en fichero ";
char *Merr_Aper=" Error de apertura de fichero ";
char *Merr_Nosecc=" Secci¢n no encontrada ";
char *Merr_Noend=" Falta marca de fin de secci¢n ";
char *Merr_Proex=" Proceso ya existe ";
char *Merr_Pronoex=" Proceso no existe ";
char *Merr_Pro0borr=" No se puede borrar Proceso 0 ";
char *Merr_Msgex=" Tabla de mensajes ya existe ";
char *Merr_Msgnoex=" Tabla de mensajes no existe ";
char *Merr_Msg0borr=" No se puede borrar tabla 0 ";
char *Merr_Npro=" N£mero de proceso no v lido ";
char *Merr_Ntmsg=" N£mero de tabla no v lida ";
char *Merr_Nocomp=" No se pudo ejecutar CS ";
char *Merr_Nolink=" No se pudo ejecutar LKS ";
char *Merr_Nods=" No se pudo ejecutar DS ";
char *Merr_Lin=" L¡nea no encontrada ";
char *Merr_Noutil=" No se pudo ejecutar utilidad ";

/* c¢digo de retorno de programas ejecutados con ejecuta() */
int cod_ejecuta=0;

/*** Programa principal ***/
#pragma warn -par
void main(int argc, char *argv[])
{
STC_MENU *m0, *m1, *m2, *m3, *mayd;
char *menu=" \xff^ð\xff:\xff^Fichero\xff:\xff^Secciones\xff:\xff^Compilar\xff:"
	   "\xff^Utilidades\xff:                \xff^Ayuda\xff ";
char *opcionesx=" ^Acerca de...: ^Configuraci¢n| Comando del ^DOS ";
char *opcionesf=" ^Nuevo: ^Abrir: ^Grabar: Grabar co^mo... |"
		" Grabar ^bloque: ^Liberar bloque| ^Salir";
char *opcioness=" ^Constantes: Mensajes ^sistema : ^Vocabulario:"
		" ^Mensajes: ^Localidades: ^Objetos: ^Procesos";
char *opcionesc=" ^Compilar base de datos : ^Ejecutar base de datos: E^rrores|"
		" Crear e^jecutable";
char *opcionesa=" ^Indice: ^Ayuda sobre...| ^Teclas del editor:"
		" Teclas ^cuadros di logo : ^Variables y banderas:"
		" ^Lista de condactos";
int i, o, o1, salida=0;
char nbd[MAXPATH], csecc[81];

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

/* si no pudo establecer modo de v¡deo */
if(!g_modovideo(G_MV_T80C)) {
	printf("\nEste programa requiere tarjeta CGA o mejor.\n");
	exit(1);
}
activa_vga();

lee_cfg(argv[0]);

m_color(cfg.color_men,cfg.color_mens1,cfg.color_mens2,cfg.color_mentec,
  cfg.color_mensel);
m=m_crea(MENU_HORZ | MENU_FIJO,NULL,menu,0,0,2);
m0=m_crea(MENU_VERT,NULL,opcionesx,1,0,0);
m1=m_crea(MENU_VERT,NULL,opcionesf,1,6,0);
m2=m_crea(MENU_VERT,NULL,opcioness,1,17,0);
m3=m_crea(MENU_VERT,NULL,opcionesc,1,30,0);
mayd=m_crea(MENU_VERT,NULL,opcionesa,1,54,0);
m_abre(m);

e_inicializa("",1,0,80,24,cfg.color_ved,cfg.color_veds1,cfg.color_veds2,
  cfg.color_vedblq,cfg.color_vedcoment,cfg.color_vedpalclv,cfg.color_vedesp,
  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,
  cfg.color_dlginput,cfg.color_dlgtec,cfg.color_dlgtecboton,"EDS.STX",E_EDITA);
e_dibuja_editor();

a_inicializa(C_CENT,C_CENT,AYD_ANCHO,AYD_ALTO,AYD_REFANCHO,cfg.color_ayd,
  cfg.color_ayds1,cfg.color_ayds2,cfg.color_aydboton,cfg.color_aydinput,
  cfg.color_aydtec,cfg.color_aydtecboton,cfg.color_aydsel);

/* inicializa variables */
ini_pos_cur();
*base_datos='\0';
for(i=0; i<MAX_PRO; i++) pro_existe[i]=FALSE;
for(i=0; i<MAX_TMSG; i++) msg_existe[i]=FALSE;

acerca_de();

do {
	o=m_elige_opcion(m);

	switch(o) {
		case -1 :
			edita_seccion(NULL,NULL,1);
			break;
		case 0 :
			m_abre(m0);
			o1=m_elige_opcion(m0);
			m_cierra(m0);
			if(o1==0) acerca_de();
			else if(o1==1) {
				if(configura()) {
					m_color(cfg.color_men,cfg.color_mens1,
					  cfg.color_mens2,cfg.color_mentec,
					  cfg.color_mensel);
					m_cambia_color(m);
					m_cambia_color(m0);
					m_cambia_color(m1);
					m_cambia_color(m2);
					m_cambia_color(m3);
					m_cambia_color(mayd);
					m_cierra(m);
					m_abre(m);
					a_cambia_color(cfg.color_ayd,
					  cfg.color_ayds1,cfg.color_ayds2,
					  cfg.color_aydboton,cfg.color_aydinput,
					  cfg.color_aydtec,cfg.color_aydtecboton,
					  cfg.color_aydsel);
					e_cambia_color(cfg.color_ved,
					  cfg.color_veds1,cfg.color_veds2,
  					  cfg.color_vedblq,cfg.color_vedcoment,
					  cfg.color_vedpalclv,cfg.color_vedesp,
					  cfg.color_dlg,cfg.color_dlgs1,
					  cfg.color_dlgs2,cfg.color_dlgboton,
  					  cfg.color_dlginput,cfg.color_dlgtec,
					  cfg.color_dlgtecboton,1);
				}
			}
			else if(o1==3) comando_dos();
			break;
		case 1 :
			m_abre(m1);
			o1=m_elige_opcion(m1);
			m_cierra(m1);
			if(o1==0) {
				graba_bd_modif();
				c_selecc_ficheros(C_CENT,C_CENT,
				  " Nueva base datos ",cfg.color_dlg,
				  cfg.color_dlgs1,cfg.color_dlgs2,
				  cfg.color_dlgboton,cfg.color_dlginput,
				  cfg.color_dlgtec,cfg.color_dlgtecboton,
				  cfg.color_dlgsel,cfg.dir_bd,"*.BD",nbd);
				if(*nbd) {
					borra_fichtemp();
					ini_pos_cur();
					edita_seccion(NULL,NULL,0);
					if(!crea_bd(nbd)) break;
					strcpy(base_datos,nbd);
					csecc[0]='$';
					strcpy(&csecc[1],Mconst);
					separa_seccion(Mconst,csecc);
					e_carga_texto(csecc);
					e_dibuja_editor();
				}
			}
			if(o1==1) {
				graba_bd_modif();
				c_selecc_ficheros(C_CENT,C_CENT,
				  " Abrir base datos ",cfg.color_dlg,
				  cfg.color_dlgs1,cfg.color_dlgs2,
				  cfg.color_dlgboton,cfg.color_dlginput,
				  cfg.color_dlgtec,cfg.color_dlgtecboton,
				  cfg.color_dlgsel,cfg.dir_bd,"*.BD",nbd);
				if(*nbd) {
					borra_fichtemp();
					ini_pos_cur();
					edita_seccion(NULL,NULL,0);
					if(!abre_bd(nbd)) break;
					strcpy(base_datos,nbd);
					csecc[0]='$';
					strcpy(&csecc[1],Mconst);
					separa_seccion(Mconst,csecc);
					e_carga_texto(csecc);
					e_dibuja_editor();
				}
			}
			else if(o1==2) {
				if(*base_datos) graba_bd(base_datos);
			}
			else if(o1==3) {
				c_selecc_ficheros(C_CENT,C_CENT,
				  " Grabar base datos ",cfg.color_dlg,
				  cfg.color_dlgs1,cfg.color_dlgs2,
				  cfg.color_dlgboton,cfg.color_dlginput,
				  cfg.color_dlgtec,cfg.color_dlgtecboton,
				  cfg.color_dlgsel,cfg.dir_bd,"*.BD",nbd);
				if(*nbd) {
					strcpy(base_datos,nbd);
					graba_bd(base_datos);
				}
			}
			else if(o1==5) graba_bloque();
			else if(o1==6) e_vacia_bloque();
			else if(o1==8) salida=1;
			break;
		case 2 :
			m_abre(m2);
			o1=m_elige_opcion(m2);
			m_cierra(m2);
			if(o1==0) edita_seccion(Mconst,&cur_org,1);
			else if(o1==1) edita_seccion(Mmsy,&cur_msy,1);
			else if(o1==2) edita_seccion(Mvoc,&cur_voc,1);
			else if(o1==3) {
				if(!*base_datos) break;
				i=elige_msg();
				if(i!=-1) {
					sprintf(csecc,"%s %i",Mmsg,i);
					edita_seccion(csecc,&cur_msg[i],1);
				}
			}
			else if(o1==4) edita_seccion(Mloc,&cur_loc,1);
			else if(o1==5) edita_seccion(Mobj,&cur_obj,1);
			else if(o1==6) {
				if(!*base_datos) break;
				i=elige_pro();
				if(i!=-1) {
					sprintf(csecc,"%s %i",Mpro,i);
					edita_seccion(csecc,&cur_pro[i],1);
				}
			}
			break;
		case 3 :
			m_abre(m3);
			o1=m_elige_opcion(m3);
			m_cierra(m3);
			if(o1==0) compila_bd();
			else if(o1==1) ejecuta_bd();
			else if(o1==2) errores_compil();
			else if(o1==4) linka_bd();
			break;
		case 4 :
			utilidades();
			break;
		case 5 :
			m_abre(mayd);
			o1=m_elige_opcion(mayd);
			m_cierra(mayd);
			if(o1==0) ayuda("");
			else if(o1==1) ayuda_sobre();
			else if(o1==3) ayuda("TECLAS DEL EDITOR");
			else if(o1==4) ayuda("CUADROS DE DIALOGO");
			else if(o1==5) ayuda("VARIABLES Y BANDERAS "
			  "DEL SISTEMA");
			else if(o1==6) ayuda("LISTA DE CONDACTOS");
			break;
	}
} while(!salida);

graba_bd_modif();
borra_fichtemp();
e_elimina();
a_elimina();

m_cierra(m);
m_elimina(m0);
m_elimina(m1);
m_elimina(m2);
m_elimina(m2);
m_elimina(m3);
m_elimina(mayd);
m_elimina(m);

vga_activa_fuente(NULL);
vga_parpadeo(1);

r_puntero(R_OCULTA);
g_modovideo(G_MV_T80C);

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
strcpy(cfg.dir_bd,ruta);
strcpy(cfg.dir_util,ruta);

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

/* lee fichero de configuraci¢n, si existe */
strcpy(nf_cfg,ruta);
strcat(nf_cfg,NF_CFG);
if((fcfg=fopen(nf_cfg,"rb"))!=NULL) {
	fread(&cfg,sizeof(STC_CFG),1,fcfg);
	fclose(fcfg);
}

}

/****************************************************************************
	INI_POS_CUR: inicializa posici¢n de cursor de edici¢n para todas
	  las secciones.
****************************************************************************/
void ini_pos_cur(void)
{
int i;

cur_org.lin=0;
cur_org.col=0;
cur_org.filcur=0;
cur_org.colcur=0;
cur_msy.lin=0;
cur_msy.col=0;
cur_msy.filcur=0;
cur_msy.colcur=0;
cur_voc.lin=0;
cur_voc.col=0;
cur_voc.filcur=0;
cur_voc.colcur=0;
for(i=0; i<256; i++) {
	cur_msg[i].lin=0;
	cur_msg[i].col=0;
	cur_msg[i].filcur=0;
	cur_msg[i].colcur=0;
}
cur_loc.lin=0;
cur_loc.col=0;
cur_loc.filcur=0;
cur_loc.colcur=0;
cur_obj.lin=0;
cur_obj.col=0;
cur_obj.filcur=0;
cur_obj.colcur=0;
for(i=0; i<256; i++) {
	cur_pro[i].lin=0;
	cur_pro[i].col=0;
	cur_pro[i].filcur=0;
	cur_pro[i].colcur=0;
}

}

/****************************************************************************
	ACERCA_DE: informaci¢n sobre el entorno.
****************************************************************************/
void acerca_de(void)
{
STC_CUADRO c;
int i;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,INF_ANCHO,INF_ALTO,
  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,
  cfg.color_dlginput,cfg.color_dlgsel,cfg.color_dlgtec,cfg.color_dlgtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,INF_ALTO-5,(INF_ANCHO-10)/2,"^Vale",8);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,
  "      Entorno de desarrollo versi¢n "VERSION"\n       "COPYRIGHT,
  INF_ANCHO-2,5,C_TXTPASA,C_TXTNOBORDE);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);
c_elimina(&c);

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

c_crea_cuadro(&c," ERROR ",C_CENT,C_CENT,ancho,8,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,cfg.color_errboton,cfg.color_errinput,
  cfg.color_errsel,cfg.color_errtec,cfg.color_errtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,4,(ancho-10)/2,"^Vale",8);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,msg,ancho-2,3,C_TXTLINEA,C_TXTNOBORDE);

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);
c_elimina(&c);

}

#if SHARE==0
/****************************************************************************
	AYUDA: muestra ayuda del entorno.
	  Entrada:      'tema' tema sobre el que mostrar ayuda
****************************************************************************/
void ayuda(char *tema)
{
char nf_ayd[MAXPATH];

strcpy(nf_ayd,cfg.dir_sintac);
strcat(nf_ayd,"EDS");
a_ayuda(nf_ayd,tema);

}
#else
/****************************************************************************
	AYUDA: muestra mensaje de aviso de versi¢n "shareware"
	  Entrada:      'tema' tema sobre el que mostrar ayuda
****************************************************************************/
void ayuda(char *tema)
{
STC_CUADRO c;
int i;

c_crea_cuadro(&c," AVISO ",C_CENT,C_CENT,34,9,cfg.color_err,cfg.color_errs1,
  cfg.color_errs2,cfg.color_errboton,cfg.color_errinput,cfg.color_errsel,
  cfg.color_errtec,cfg.color_errtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,4,12,"^Vale",8);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,
  " Ayuda en l¡nea no disponible\n   en la versi¢n 'shareware'",
  32,4,C_TXTPASA,C_TXTBORDE)

c_abre(&c);
do {
	i=c_gestiona(&c);
} while((i!=-1) && (i!=0));
c_cierra(&c);
c_elimina(&c);

}
#endif

/****************************************************************************
	COMANDO_DOS: ejecuta un comando del sistema operativo.
****************************************************************************/
void comando_dos(void)
{
STC_CUADRO c;
STC_ELEM *elem;
STC_ELEM_CHECK *chkpausa;
char comando[81], cmd[129], ant_dir[MAXDIR];
int i, shell=0, ant_drive;

comando[0]='\0';

c_crea_cuadro(&c," Comando del DOS ",C_CENT,C_CENT,DOS_ANCHO,DOS_ALTO,
  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,
  cfg.color_dlginput,cfg.color_dlgsel,cfg.color_dlgtec,cfg.color_dlgtecboton);
c_crea_elemento(&c,C_ELEM_INPUT,0,DOS_ANCHO-31,"^Comando",28,comando,
  sizeof(comando)-1);
c_crea_elemento(&c,C_ELEM_BOTON,3,DOS_ANCHO-27,"^Ejecutar",12);
c_crea_elemento(&c,C_ELEM_BOTON,3,DOS_ANCHO-13,"^Salir",9);
elem=c_crea_elemento(&c,C_ELEM_CHECK,3,1,"^Pausa");
chkpausa=(STC_ELEM_CHECK *)elem->info;
chkpausa->estado=1;

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);
c_elimina(&c);
if(i==1) {
	m_cierra(m);
	r_puntero(R_OCULTA);
	g_modovideo(G_MV_T80C);

	/* guarda unidad y directorio actuales */
	ant_drive=getdisk();
	getcwd(ant_dir,MAXDIR);

	cmd[0]='\0';
	if(!*comando) {
		shell=1;
		printf("\nTeclea EXIT para regresar.\n");
	}
	else {
		strcat(cmd,"/C ");
		strcat(cmd,comando);
	}
	sspawn("",cmd,".");
	if(chkpausa->estado && !shell) {
		printf("\nPulsa una tecla.");
		bioskey(0);
	}

	/* recupera unidad y directorio actuales */
	setdisk(ant_drive);
	chdir(ant_dir);
	activa_vga();

	m_abre(m);
	e_dibuja_editor();
	return;
}

}

/****************************************************************************
	GRABA_BLOQUE: graba el £ltimo bloque seleccionado.
****************************************************************************/
void graba_bloque(void)
{
char fichero[MAXPATH];

c_selecc_ficheros(C_CENT,C_CENT," Grabar bloque ",cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,cfg.color_dlginput,
  cfg.color_dlgtec,cfg.color_dlgtecboton,cfg.color_dlgsel,"","*.*",fichero);

if(*fichero) {
	if(e_graba_bloque(fichero)==0) imprime_error(Merr_Escr);
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
	COPIA_FICHERO: copia un fichero en otro.
	  Entrada:      'nfichorg' nombre de fichero de origen
			'nfichdest' nombre de fichero destino
	  Salida:       1 si pudo copiar, 0 si error
****************************************************************************/
int copia_fichero(char *nfichorg, char *nfichdest)
{
char *buf, bufaux[256];
int horg, hdest, bufaux_usado=0;
long tam=0xff00L, flng;

esconde_cursor();

if((horg=open(nfichorg,O_BINARY | O_RDONLY))==-1) {
	imprime_error(Merr_Aper);
	return(0);
}
if((hdest=open(nfichdest,O_BINARY | O_WRONLY | O_CREAT | O_TRUNC,
  S_IREAD | S_IWRITE))==-1) {
	close(horg);
	imprime_error(Merr_Aper);
	return(0);
}

flng=filelength(horg);
if(flng<tam) tam=flng;

/* reserva memoria para buffer, si no hay suficiente memoria busca */
/* la m xima cantidad disponible */
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
		imprime_error(Merr_Lect);
		close(horg);
		close(hdest);
		if(!bufaux_usado) free(buf);
		return(0);
	}
	if(write(hdest,buf,(unsigned)tam)==-1) {
		imprime_error(Merr_Escr);
		close(horg);
		close(hdest);
		if(!bufaux_usado) free(buf);
		return(0);
	}
}

close(horg);
close(hdest);
if(!bufaux_usado) free(buf);

return(1);
}

/****************************************************************************
	CREA_BD: crea una nueva base de datos.
	  Entrada:      'bd' nombre del fichero de base de datos
	  Salida:       1 si la pudo crear, 0 si error
****************************************************************************/
int crea_bd(char *bd)
{
STC_CUADRO c;
FILE *fbd;
char *marca_fin="\n\\END\n\n";
char nf_bdinic[MAXPATH];
int i;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,AV_ANCHO,AV_ALTO,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,cfg.color_dlginput,
  cfg.color_dlgsel,cfg.color_dlgtec,cfg.color_dlgtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,AV_ALTO-5,15,"^Si",6);
c_crea_elemento(&c,C_ELEM_BOTON,AV_ALTO-5,23,"^No",6);
c_crea_elemento(&c,C_ELEM_BOTON,AV_ALTO-5,31,"Sa^lir",9);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,"           ¨Usar base de datos de inicio?",
  AV_ANCHO-2,3,C_TXTPASA,C_TXTNOBORDE);

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);
c_elimina(&c);

if((i==-1) || (i==2)) return(0);
else if(i==0) {
	strcpy(nf_bdinic,cfg.dir_bd);
	strcat(nf_bdinic,NF_BDINIC);
	if(!copia_fichero(nf_bdinic,bd)) return(0);
}
else if(i==1) {
	if((fbd=fopen(bd,"wt"))==NULL) return(0);

	fputs("\\\\V_MOV\t\t14\t; m ximo n§ para verbos de movimiento\n",fbd);
	fputs("\\\\N_CONV\t20\t; m ximo n§ para nombres convertibles\n",fbd);
	fputs("\\\\N_PROP\t50\t; m ximo n§ para nombres propios\n\n",fbd);

	fputs("\\MSY\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\VOC\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\MSG 0\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\LOC\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\OBJ\n",fbd);
	fputs(marca_fin,fbd);

	fputs("\\PRO 0\n",fbd);
	fputs(marca_fin,fbd);

	fclose(fbd);
}

bd_modificada=TRUE;
ft_actualizado=FALSE;
if(!abre_bd(bd)) return(0);

return(1);
}

/****************************************************************************
	ABRE_BD: abre una base de datos.
	  Entrada:      'bd' nombre del fichero de base de datos
	  Salida:       1 si la pudo abrir, 0 si error
****************************************************************************/
int abre_bd(char *bd)
{

/* copia el fichero de base de datos en fichero temporal */
if(!copia_fichero(bd,Nf_Bdtemp)) return(0);

ft_actualizado=TRUE;

return(1);
}

/****************************************************************************
	ES_MARCA_SECC: comprueba si una l¡nea es marca de secci¢n.
	  Entrada:      'lin' l¡nea a comprobar
	  Salida:       1 si es marca de secci¢n, 0 si no
****************************************************************************/
int es_marca_secc(char *lin)
{

/* salta blancos iniciales */
for(; ((*lin==' ') || (*lin=='\t')); lin++);

/* mira es marca de secci¢n (y no definici¢n de constante) */
if((*lin==MARCA_S) && (*(lin+1)!=MARCA_S)) return(1);

return(0);
}

/****************************************************************************
	BUSCA_MARCA: busca una marca de inicio o final de secci¢n.
	  Entrada:      'marca' marca a buscar ("\MSY", "\MSG", "\END",
			etc...)
			'l' l¡nea a analizar
	  Salida:       1 si la encontr¢, 0 si no
****************************************************************************/
int busca_marca(char *marca, char *l)
{

/* salta blancos iniciales */
for(; (*l && ((*l==' ') || (*l=='\t'))); l++);

/* mira si encuentra la marca */
for(; (*marca && *l); marca++, l++) if(*marca!=*l) return(0);

return(1);
}

/****************************************************************************
	SALTA_HASTA_MARCA: salta texto de fichero de base de datos
	  hasta la siguiente marca de secci¢n, deja el puntero del fichero
	  en la l¡nea que contiene la marca.
	  Entrada:      'fbd' puntero a fichero de base de datos
	  Salida:       1 se complet¢ operaci¢n, 0 si error
****************************************************************************/
int salta_hasta_marca(FILE *fbd)
{
char c[E_MAXLNGLIN];
fpos_t pos;

while(1) {
	fgetpos(fbd,&pos);
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			return(0);
		}
	}

	if(es_marca_secc(c)) {
		fsetpos(fbd,&pos);
		break;
	}
}

return(1);
}

/****************************************************************************
	COGE_MARCA: recoge la marca de secci¢n de una l¡nea de texto.
	  Entrada:      'l' l¡nea de texto a analizar
			'marca' puntero a buffer donde se guardar  la
			marca de secci¢n (m ximo 80 caracteres)
****************************************************************************/
void coge_marca(char *l, char *marca)
{
int i;

/* salta blancos iniciales */
for(; (*l && ((*l==' ') || (*l=='\t'))); l++);

for(i=0; ((i<80) && (*l!=' ') && (*l!='\t') && (*l!='\n') &&
  (*l!=CHR_COMENT) && *l); l++, marca++, i++) *marca=*l;
*marca='\0';

}

/****************************************************************************
	PON_EXTENSIONSECC: coloca al nombre de fichero temporal la
	  extensi¢n correspondiente de una secci¢n m£ltiple (el n£mero
	  que acompa¤a a la marca de inicio de secci¢n)
	  Entrada:      'csecc' marca de inicio de secci¢n
			'nf' nombre de fichero temporal sin extensi¢n
	  Salida:       n£mero que acompa¤a a marca de secci¢n, -1 si error
****************************************************************************/
int pon_extensionsecc(char *csecc, char *nf)
{
int i;
char num[4];

/* salta blancos iniciales */
for(; ((*csecc==' ') || (*csecc=='\t')); csecc++);

/* salta caracteres hasta siguiente car cter blanco o fin de cadena */
for(; ((*csecc!=' ') && (*csecc!='\t') && (*csecc!='\0') && (*csecc!='\n'));
  csecc++);

/* si ha encontrado final de cadena, da error */
if((*csecc=='\0') || (*csecc=='\n')) return(-1);

/* salta blancos */
for(; ((*csecc==' ') || (*csecc=='\t')); csecc++);

/* se supone que lo siguiente, hasta el final de la cadena o */
/* hasta el siguiente blanco, es un n£mero de 3 d¡gitos m ximo */
for(i=0; ((*csecc!=' ') && (*csecc!='\t') && (*csecc!='\0') &&
  (*csecc!='\n') && (i<3)); csecc++, i++) {
	num[i]=*csecc;
}
num[i]='\0';

/* a¤ade extensi¢n a nombre de fichero temporal */
strcat(nf,".");
strcat(nf,num);

return(atoi(num));
}

/****************************************************************************
	COPIA_FICHSECC: copia fichero temporal de secci¢n en fichero
	  de base de datos.
	  Entrada:      'nfsecc' nombre de fichero de secci¢n
			'fbd' puntero a fichero de base de datos
			'msecc' marca de inicio de secci¢n (NULL ninguna)
	  Salida:       1 si lo pudo copiar, 0 si error
****************************************************************************/
int copia_fichsecc(char *nfsecc, FILE *fbd, char *msecc)
{
FILE *fsecc;
char *buf, bufaux[256];
int bufaux_usado=0;
long tam=0xff00L, flng;

if((fsecc=fopen(nfsecc,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

flng=filelength(fileno(fsecc));
if(flng<tam) tam=flng;

/* reserva memoria para buffer, si es un fichero de longitud 0 */
/* indica buffer auxiliar usado (aunque realmente no se usa) para */
/* evitar tener que reservar y liberar memoria */
if(!flng) bufaux_usado=1;
else if((buf=(char *)malloc((size_t)tam))==NULL) {
	/* si no hay suficiente memoria busca lo m ximo disponible */
	tam=max_mem();
	/* si a£n asi no pudo reservar memoria, usa buffer auxiliar */
	if((buf=(char *)malloc((size_t)tam))==NULL) {
		buf=bufaux;
		tam=sizeof(bufaux);
		bufaux_usado=1;
	}
}

/* escribe marca de inicio de secci¢n */
if(msecc!=NULL) fprintf(fbd,"%s\n",msecc);

if(flng) while(!feof(fsecc)) {
	tam=fread(buf,sizeof(char),(size_t)tam,fsecc);
	if(ferror(fsecc)) {
		imprime_error(Merr_Lect);
		fclose(fsecc);
		if(!bufaux_usado) free(buf);
		return(0);
	}

	if(fwrite(buf,sizeof(char),(size_t)tam,fbd)!=tam) {
		imprime_error(Merr_Escr);
		fclose(fsecc);
		if(!bufaux_usado) free(buf);
		return(0);
	}
}

/* escribe marca de fin de secci¢n */
if(msecc!=NULL) fprintf(fbd,"%s\n",Mfin_Secc);

fclose(fsecc);
if(!bufaux_usado) free(buf);

return(1);
}

/****************************************************************************
	COPIA_SECC: copia secci¢n de fichero de base de datos a otro
	  fichero.
	  Entrada:      'fbd' puntero a fichero de base de datos
			'faux' puntero a fichero donde se copiar  secci¢n
			'msecc' marca de inicio de secci¢n (NULL ninguna)
	  Salida:       1 si pudo copiar, 0 si error
****************************************************************************/
int copia_secc(FILE *fbd, FILE *faux, char *msecc)
{
char c[E_MAXLNGLIN];

/* escribe marca de inicio de secci¢n */
if(msecc!=NULL) fprintf(faux,"%s\n",msecc);

/* copia l¡neas hasta encontrar marca de secci¢n */
while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			return(0);
		}
	}

	if(es_marca_secc(c)) break;

	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		return(0);
	}
}

/* escribe marca de fin de secci¢n */
if(msecc!=NULL) fprintf(faux,"%s\n",Mfin_Secc);

return(1);
}

/****************************************************************************
	ACTUALIZA_BDTEMP: actualiza el fichero de base de datos temporal,
	  'Nf_Bdtemp' con el contenido de los ficheros temporales asociados
	  a cada secci¢n.
	  Salida:       1 si se complet¢ la operaci¢n, 0 si error
****************************************************************************/
int actualiza_bdtemp(void)
{
FILE *fbd, *faux;
char nfsecc[13], c[E_MAXLNGLIN], marca[81];
int i;

esconde_cursor();

/* si base de datos no ha sido modificada o ya est  actualizado el fichero */
/* temporal, no hace falta actualizarla */
if((bd_modificada==FALSE) || (ft_actualizado==TRUE)) return(1);

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}
if((faux=fopen(Nf_Bdaux,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	fclose(fbd);
	return(0);
}

/* mira secciones de la base de datos, si alguna tiene asociado */
/* un fichero temporal copia el contenido del fichero, si no deja */
/* la secci¢n como est  */

/* constantes */
sprintf(nfsecc,Nf_Secc,Mconst);
if(!access(nfsecc,0)) {
	if(!copia_fichsecc(nfsecc,faux,NULL)) {
		fclose(faux);
		fclose(fbd);
		return(0);
	}
	if(!salta_hasta_marca(fbd)) {
		fclose(faux);
		fclose(fbd);
		return(0);
	}
}
else {
	if(!copia_secc(fbd,faux,NULL)) {
		fclose(faux);
		fclose(fbd);
		return(0);
	}
	rewind(fbd);
	if(!salta_hasta_marca(fbd)) {
		fclose(faux);
		fclose(fbd);
		return(0);
	}
}

while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(faux);
			fclose(fbd);
			return(0);
		}
	}

	/* si encuentra marca de secci¢n, mira cual es y comprueba si */
	/* existe fichero temporal para esa secci¢n */
	if(es_marca_secc(c)) {
		coge_marca(c,marca);
		sprintf(nfsecc,Nf_Secc,&marca[1]);
		if(!strcmp(marca,Mpro)) {
			i=pon_extensionsecc(c,nfsecc);
			sprintf(marca,"%s %i",Mpro,i);
			if(pro_existe[i]) {
				if(!copia_fichsecc(nfsecc,faux,marca)) {
					fclose(faux);
					fclose(fbd);
					return(0);
				}
				if(!salta_hasta_marca(fbd)) {
					fclose(faux);
					fclose(fbd);
					return(0);
				}
				fgets(c,E_MAXLNGLIN,fbd);
			}
			else if(!copia_secc(fbd,faux,marca)) {
				fclose(faux);
				fclose(fbd);
				return(0);
			}
		}
		else if(!strcmp(marca,Mmsg)) {
			i=pon_extensionsecc(c,nfsecc);
			sprintf(marca,"%s %i",Mmsg,i);
			if(msg_existe[i]) {
				if(!copia_fichsecc(nfsecc,faux,marca)) {
					fclose(faux);
					fclose(fbd);
					return(0);
				}
				if(!salta_hasta_marca(fbd)) {
					fclose(faux);
					fclose(fbd);
					return(0);
				}
				fgets(c,E_MAXLNGLIN,fbd);
			}
			else if(!copia_secc(fbd,faux,marca)) {
				fclose(faux);
				fclose(fbd);
				return(0);
			}
		}
		else if(!access(nfsecc,0)) {
			if(!copia_fichsecc(nfsecc,faux,marca)) {
				fclose(faux);
				fclose(fbd);
				return(0);
			}
			if(!salta_hasta_marca(fbd)) {
				fclose(faux);
				fclose(fbd);
				return(0);
			}
			fgets(c,E_MAXLNGLIN,fbd);
		}
		else if(!copia_secc(fbd,faux,marca)) {
			fclose(faux);
			fclose(fbd);
			return(0);
		}
	 }
}

fclose(faux);
fclose(fbd);

/* borra fichero temporal de base de datos y renombra fichero auxiliar */
/* a fichero temporal de base de datos */
remove(Nf_Bdtemp);
rename(Nf_Bdaux,Nf_Bdtemp);

ft_actualizado=TRUE;

return(1);
}

/****************************************************************************
	GRABA_BD: graba la base de datos actual.
	  Entrada:      'bd' nombre de fichero donde se grabar 
	  Salida:       1 si se pudo grabar, 0 si error
****************************************************************************/
int graba_bd(char *bd)
{

if(!actualiza_bdtemp()) return(0);
if(!copia_fichero(Nf_Bdtemp,bd)) return(0);

bd_modificada=FALSE;

return(1);
}

/****************************************************************************
	GRABA_BD_MODIF: graba la base de datos actual si ha sido modificada.
	  Salida:       1 si se pudo grabar, 0 si error
****************************************************************************/
int graba_bd_modif(void)
{
STC_CUADRO c;
int i;

if(bd_modificada==TRUE) {
	c_crea_cuadro(&c,NULL,C_CENT,C_CENT,AV_ANCHO,AV_ALTO,
	  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,
	  cfg.color_dlginput,cfg.color_dlgsel,cfg.color_dlgtec,
	  cfg.color_dlgtecboton);
	c_crea_elemento(&c,C_ELEM_BOTON,AV_ALTO-5,20,"^Si",6);
	c_crea_elemento(&c,C_ELEM_BOTON,AV_ALTO-5,28,"^No",6);
	c_crea_elemento(&c,C_ELEM_TEXTO,0,0,
	  "          Base de datos ha sido modificada\n"
	  "                     ¨Grabarla?",AV_ANCHO-2,4,C_TXTLINEA,
	  C_TXTNOBORDE);

	c_abre(&c);
	i=c_gestiona(&c);
	c_cierra(&c);
	c_elimina(&c);

	if(i==0) {
		if(!graba_bd(base_datos)) return(0);
		else return(1);
	}
}

return(1);
}

/****************************************************************************
	BORRA_FICHTEMP: borra ficheros temporales usados por el entorno.
****************************************************************************/
void borra_fichtemp(void)
{
char nfsecc[13];
int i;

esconde_cursor();

remove(Nf_Bdtemp);
remove(Nf_Bdaux);
remove(Nf_Err);

sprintf(nfsecc,Nf_Secc,Mconst);
remove(nfsecc);
sprintf(nfsecc,Nf_Secc,&Mmsy[1]);
remove(nfsecc);
sprintf(nfsecc,Nf_Secc,&Mvoc[1]);
remove(nfsecc);

for(i=0; i<MAX_TMSG; i++) {
	if(msg_existe[i]==TRUE) {
		sprintf(nfsecc,Nf_Seccmult,&Mmsg[1],i);
		remove(nfsecc);
		msg_existe[i]=FALSE;
	}
}

sprintf(nfsecc,Nf_Secc,&Mloc[1]);
remove(nfsecc);
sprintf(nfsecc,Nf_Secc,&Mobj[1]);
remove(nfsecc);

for(i=0; i<MAX_PRO; i++) {
	if(pro_existe[i]==TRUE) {
		sprintf(nfsecc,Nf_Seccmult,&Mpro[1],i);
		remove(nfsecc);
		pro_existe[i]=FALSE;
	}
}

}

/****************************************************************************
	SEPARA_SECCION: separa del fichero de base de datos temporal
	  'Nf_Bdtemp' una secci¢n y la almacena en un fichero temporal.
	  Entrada:      'csecc' cabecera de la secci¢n ("ORG", "\VOC",
			"\PRO 1",...), si es "ORG" separa el inicio del
			fichero hasta la primera marca de secci¢n
			'nfsecc' nombre del fichero donde se guardar  la
			secci¢n
	  Salida:       1 si se pudo completar operaci¢n, 0 si error
****************************************************************************/
int separa_seccion(char *csecc, char *nfsecc)
{
FILE *fbd, *fsecc;
char c[E_MAXLNGLIN];
BOOLEAN fin_seccion=FALSE;

esconde_cursor();

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

if((fsecc=fopen(nfsecc,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	fclose(fbd);
	return(0);
}

if(!strcmp(csecc,Mconst)) while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
	}

	/* comprueba si ha encontrado una marca de secci¢n */
	if(es_marca_secc(c)) break;

	if(fputs(c,fsecc)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(fsecc);
		return(0);
	}
}
else while(fin_seccion==FALSE) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) {
			imprime_error(Merr_Nosecc);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(fsecc);
			return(0);
		}
	}

	/* si encuentra encabezamiento de secci¢n, copia l¡neas hasta */
	/* encontrar marca de fin de secci¢n */
	if(busca_marca(csecc,c)) {
		while(1) {
			if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
				if(feof(fbd)) {
					imprime_error(Merr_Noend);
					fclose(fbd);
					fclose(fsecc);
					return(0);
				}
				else {
					imprime_error(Merr_Lect);
					fclose(fbd);
					fclose(fsecc);
					return(0);
				}
			}

			if(busca_marca(Mfin_Secc,c)) {
				fin_seccion=TRUE;
				break;
			}

			if(fputs(c,fsecc)==EOF) {
				imprime_error(Merr_Escr);
				fclose(fbd);
				fclose(fsecc);
				return(0);
			}
		}
	}
}

fclose(fbd);
fclose(fsecc);

return(1);
}

/****************************************************************************
	EDITA_SECCION: edita una secci¢n de la base de datos.
	  Entrada:      'csecc' marca de inicio de secci¢n ("ORG", "\VOC",
			"\PRO 1",...), NULL si quiere editar £ltima secci¢n
			que fue editada
			'cur_pos' puntero a datos de posici¢n del cursor,
			NULL si se recupera posici¢n dentro de £ltima
			secci¢n editada
			'modo' 0 para inicializar (usar despu‚s de cargar
			otra base de datos), 1 normal
	  Salida:       1 si se pudo editar, 0 si error
****************************************************************************/
int edita_seccion(char *csecc, CURSOR_ED *cur_pos, int modo)
{
static char ultsecc[9]="ORG";
static CURSOR_ED cur_ult={0, 0, 0, 0};
char *secc, nftemp[13];
int i;

if(!modo) {
	strcpy(ultsecc,"ORG");
	cur_ult.lin=0;
	cur_ult.col=0;
	cur_ult.filcur=0;
	cur_ult.colcur=0;
	return(1);
}

if(!*base_datos) return(1);

/* comprueba si quiere editar £ltima secci¢n editada */
if(csecc==NULL) secc=ultsecc;
else secc=csecc;

/* crea nombre de fichero temporal correspondiente a la secci¢n */
nftemp[0]='$';
if(!strcmp(secc,Mconst)) strcpy(&nftemp[1],Mconst);
else if(strstr(secc,Mpro)!=NULL) {
	strcpy(&nftemp[1],&Mpro[1]);
	i=pon_extensionsecc(secc,nftemp);
}
else if(strstr(secc,Mmsg)!=NULL) {
	strcpy(&nftemp[1],&Mmsg[1]);
	i=pon_extensionsecc(secc,nftemp);
}
else strcpy(&nftemp[1],&secc[1]);

/* si no existe fichero temporal de esa secci¢n, lo crea */
if(access(nftemp,0)) {
	if(!separa_seccion(secc,nftemp)) {
		remove(nftemp);
		return(0);
	}
}

/* si es secci¢n m£ltiple, indica que ha sido creado fichero temporal */
if(strstr(secc,Mpro)!=NULL) pro_existe[i]=TRUE;
else if(strstr(secc,Mmsg)!=NULL) msg_existe[i]=TRUE;

/* edita secci¢n */
if(!e_carga_texto(nftemp)) {
	imprime_error(Merr_Lect);
	e_borra_texto();
	return(0);
}

/* recupera posici¢n del cursor */
if(cur_pos==NULL) e_cursor_recupera(&cur_ult);
else e_cursor_recupera(cur_pos);

do {
	i=e_editor();
	if(i==1) ayuda("");
	else if(i==2) ayuda(e_palabra_cursor());
} while(i);

/* guarda posici¢n actual del cursor */
if(cur_pos!=NULL) e_cursor_guarda(cur_pos);

/* guarda secci¢n que ha sido editada */
if(secc!=ultsecc) strcpy(ultsecc,secc);
e_cursor_guarda(&cur_ult);

if(e_modificado()) {
	bd_modificada=TRUE;
	ft_actualizado=FALSE;
	if(!e_graba_texto(NULL)) {
		imprime_error(Merr_Escr);
		return(0);
	}
}

return(1);
}

/****************************************************************************
	CREA_SECCMULT: crea una secci¢n m£ltiple nueva en fichero temporal
	  de base de datos 'Nf_Bdtemp'.
	  Entrada:      'ns' n£mero de secci¢n
			'msecc' marca de inicio de secci¢n
			'merr_ex' mensaje de error que se imprimir  si la
			secci¢n ya existe
			'secc_existe' puntero a tabla de comprobaci¢n de
			existencia de ficheros temporales de secciones
			m£ltiples
	  Salida:       1 si pudo crearla, 0 si error
****************************************************************************/
int crea_seccmult(int ns, char *msecc, char *merr_ex, BOOLEAN *secc_existe)
{
FILE *fbd, *faux;
char c[E_MAXLNGLIN], *p, num[17], nfsecc[13];
int i, numero, secc_encontrada=0;

esconde_cursor();

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

if((faux=fopen(Nf_Bdaux,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	fclose(fbd);
	return(0);
}

while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	/* si es marca de inicio de secci¢n, comprueba su n£mero */
	if(busca_marca(msecc,c)) {
		/* indica que se ha encontrado secci¢n */
		secc_encontrada=1;

		/* busca '\' */
		for(p=c; *p!=MARCA_S; p++);

		/* avanza hasta primer blanco */
		for(; ((*p!=' ') && (*p!='\t')); p++);

		/* salta espacios */
		for(; ((*p==' ') || (*p=='\t')); p++);

		/* recoge n£mero, m ximo 3 d¡gitos */
		for(i=0; i<3; i++, p++) {
			if(strchr(CNum,*p)==NULL) break;
			num[i]=*p;
		}
		num[i]='\0';

		numero=atoi(num);

		/* si n£mero de secci¢n encontrada es mayor que la */
		/* que queremos insertar, sale de bucle */
		if(numero>ns) break;

		/* si es igual finaliza */
		if(numero==ns) {
			imprime_error(merr_ex);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}
	/* si ha encontrado otra secci¢n, para de buscar e inserta */
	/* secci¢n m£ltiple (se supone que las secciones m£ltiples */
	/* van una detr s de otra en la base de datos) */
	else if(es_marca_secc(c) && !busca_marca(Mfin_Secc,c) &&
	  secc_encontrada) break;

	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(faux);
		return(0);
	}
}

bd_modificada=TRUE;
ft_actualizado=FALSE;

/* inserta secci¢n nueva */
fprintf(faux,"%s %i\n",msecc,ns);
fprintf(faux,"%s\n",Mfin_Secc);

/* copia resto de 'Nf_Bdtemp' en 'Nf_Bdaux' */
/* pone condici¢n de que no est‚ al final del fichero 'fbd' ya que si */
/* es as¡, la nueva tabla se insert¢ al final */
while(!feof(fbd)) {
	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(faux);
		return(0);
	}

	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}
}

fclose(fbd);
fclose(faux);

/* borra fichero temporal de base de datos y renombra fichero auxiliar */
/* a fichero temporal de base de datos */
remove(Nf_Bdtemp);
rename(Nf_Bdaux,Nf_Bdtemp);

/* crea fichero temporal vac¡o */
sprintf(nfsecc,Nf_Seccmult,&msecc[1],ns);
if((faux=fopen(nfsecc,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}
fclose(faux);
secc_existe[ns]=TRUE;

return(1);
}

/****************************************************************************
	BORRA_SECCMULT: borra una secci¢n m£ltiple del fichero temporal
	  de base de datos 'Nf_Bdtemp'.
	  Entrada:      'ns' n£mero de secci¢n
			'msecc' marca de inicio de secci¢n
			'merr_noex' mensaje de error que se imprimir  si la
			secci¢n no existe
			'secc_existe' puntero a tabla de comprobaci¢n de
			existencia de ficheros temporales de secciones
			m£ltiples
	  Salida:       1 si pudo borrarla, 0 si error
****************************************************************************/
int borra_seccmult(int ns, char *msecc, char *merr_noex, BOOLEAN *secc_existe)
{
FILE *fbd, *faux;
char c[E_MAXLNGLIN], *p, num[17], nfsecc[13];
int i, numero;

esconde_cursor();

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

if((faux=fopen(Nf_Bdaux,"wt"))==NULL) {
	imprime_error(Merr_Aper);
	fclose(fbd);
	return(0);
}

while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	/* si es marca de inicio de secci¢n, comprueba su n£mero */
	if(busca_marca(msecc,c)) {
		/* busca '\' */
		for(p=c; *p!=MARCA_S; p++);

		/* avanza hasta primer blanco */
		for(; ((*p!=' ') && (*p!='\t')); p++);

		/* salta espacios */
		for(; ((*p==' ') || (*p=='\t')); p++);

		/* recoge n£mero, m ximo 3 d¡gitos */
		for(i=0; i<3; i++, p++) {
			if(strchr(CNum,*p)==NULL) break;
			num[i]=*p;
		}
		num[i]='\0';

		numero=atoi(num);

		/* si n£mero de secci¢n encontrada es mayor que la */
		/* que queremos borrar finaliza */
		if(numero>ns) {
			imprime_error(merr_noex);
			fclose(fbd);
			fclose(faux);
			return(0);
		}

		/* si es igual sale de bucle */
		if(numero==ns) break;
	}

	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(faux);
		return(0);
	}
}

bd_modificada=TRUE;
ft_actualizado=FALSE;

/* salta hasta marca de fin de seccion */
while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	if(busca_marca(Mfin_Secc,c)) break;
}

/* copia resto de 'Nf_Bdtemp' en 'Nf_Bdaux' */
while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			fclose(faux);
			return(0);
		}
	}

	if(fputs(c,faux)==EOF) {
		imprime_error(Merr_Escr);
		fclose(fbd);
		fclose(faux);
		return(0);
	}
}

fclose(fbd);
fclose(faux);

/* borra fichero temporal de base de datos y renombra fichero auxiliar */
/* a fichero temporal de base de datos */
remove(Nf_Bdtemp);
rename(Nf_Bdaux,Nf_Bdtemp);

/* si existe fichero temporal asociado a esa secci¢n, lo borra */
if(secc_existe[ns]==TRUE) {
	sprintf(nfsecc,Nf_Seccmult,&msecc[1],ns);
	remove(nfsecc);
	secc_existe[ns]=FALSE;
}

return(1);
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
int i, ancho;

ancho=strlen(preg)+4;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,ancho,8,cfg.color_dlg,cfg.color_dlgs1,
  cfg.color_dlgs2,cfg.color_dlgboton,cfg.color_dlginput,cfg.color_dlgsel,
  cfg.color_dlgtec,cfg.color_dlgtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,4,1,"^Si",6);
c_crea_elemento(&c,C_ELEM_BOTON,4,9,"^No",6);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,preg,ancho-2,3,C_TXTLINEA,C_TXTBORDE);

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);
c_elimina(&c);

if(i==0) return(1);

return(0);
}

/****************************************************************************
	ELIGE_PRO: presenta una lista de procesos de la base de datos
	  y permite elegir uno; se usa la base de datos temporal
	  'Nf_Bdtemp'.
	  Salida:       n£mero de proceso seleccionado o -1 si no
			seleccion¢ ninguno o error
****************************************************************************/
int elige_pro(void)
{
STC_CUADRO c;
STC_ELEM *elem;
STC_ELEM_LISTA *pro;
FILE *fbd, *fsecc;
char l[E_MAXLNGLIN], nfsecc[13], buff[81], npro[4];
int i, lng, encontrado;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,PRO_ANCHO,PRO_ALTO,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,cfg.color_dlginput,
  cfg.color_dlgsel,cfg.color_dlgtec,cfg.color_dlgtecboton);
elem=c_crea_elemento(&c,C_ELEM_LISTA,0,0," ^Procesos ",PRO_ANCHO-2,PRO_ALTO-5,
  C_LSTNORMAL,C_LSTSINORDEN);
pro=(STC_ELEM_LISTA *)elem->info;
c_crea_elemento(&c,C_ELEM_BOTON,PRO_ALTO-4,1,"^Vale",8);
c_crea_elemento(&c,C_ELEM_BOTON,PRO_ALTO-4,11,"^Borrar",10);
c_crea_elemento(&c,C_ELEM_BOTON,PRO_ALTO-4,23,"^Salir",9);
c_crea_elemento(&c,C_ELEM_BOTON,PRO_ALTO-4,PRO_ANCHO-33,"^Nuevo proceso",17);
npro[0]='\0';
c_crea_elemento(&c,C_ELEM_INPUT,PRO_ALTO-4,PRO_ANCHO-9,"P^ro",6,npro,
  sizeof(npro)-1);

/* crea lista de procesos */
if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(-1);
}

while(1) {
	if(fgets(l,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			return(-1);
		}
	}

	/* si encuentra proceso comprueba si existe fichero temporal */
	/* asociado; si es as¡ lee l¡nea de fichero temporal, si no */
	/* lee l¡nea de fichero de base de datos */
	if(busca_marca(Mpro,l)) {
		sprintf(nfsecc,Nf_Secc,&Mpro[1]);
		i=pon_extensionsecc(l,nfsecc);
		if(pro_existe[i]==TRUE) {
			if((fsecc=fopen(nfsecc,"rt"))==NULL) {
				imprime_error(Merr_Aper);
				fclose(fbd);
				return(-1);
			}
			fgets(l,E_MAXLNGLIN,fsecc);
			fclose(fsecc);
		}
		else fgets(l,E_MAXLNGLIN,fbd);

		/* almacena entrada en lista */
		lng=strlen(l);
		if(l[lng-1]=='\n') l[lng-1]='\0';
		sprintf(buff,"%3i: %-75s",i,l);
		c_mete_en_lista(pro,buff);
	}
}

fclose(fbd);

c_abre(&c);

while(1) {
	i=c_gestiona(&c);

	if((i==-1) || (i==3)) break;
	else if(i==2) {
		i=atoi(pro->selecc);
		if(!i) imprime_error(Merr_Pro0borr);
		else if(pregunta_siono(" ¨Quieres borrar proceso? ")) {
			borra_seccmult(i,Mpro,Merr_Pronoex,pro_existe);
			break;
		}
	}
	else if(i==4) {
		i=atoi(npro);

		/* comprueba si n£mero dentro de rango permitido */
		if((i<0) || (i>=MAX_PRO)) {
			imprime_error(Merr_Npro);
			continue;
		}

		/* comprueba si existe fichero asociado a proceso */
		if(pro_existe[i]==TRUE) {
			imprime_error(Merr_Proex);
			continue;
		}

		/* comprueba si existe proceso en fichero de base de datos */
		if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
			imprime_error(Merr_Aper);
			c_cierra(&c);
			c_elimina(&c);
			return(-1);
		}
		sprintf(buff,"%s %i",Mpro,i);
		encontrado=0;
		while(1) {
			if(fgets(l,E_MAXLNGLIN,fbd)==NULL) {
				if(feof(fbd)) break;
				else {
					imprime_error(Merr_Lect);
					fclose(fbd);
					c_cierra(&c);
					c_elimina(&c);
					return(-1);
				}
			}
			if(busca_marca(buff,l)) {
				imprime_error(Merr_Proex);
				encontrado=1;
				break;
			}
		}
		fclose(fbd);

		if(encontrado) continue;
		else if(crea_seccmult(i,Mpro,Merr_Proex,pro_existe)) {
			c_cierra(&c);
			c_elimina(&c);
			return(i);
		}
		break;
	}
	else {
		if(*pro->selecc) {
			c_cierra(&c);
			c_elimina(&c);
			return(atoi(pro->selecc));
		}
		break;
	}
}

c_cierra(&c);
c_elimina(&c);

return(-1);
}

/****************************************************************************
	ELIGE_MSG: presenta una lista de tablas de mensajes de la base de
	  datos y permite elegir una; se usa la base de datos temporal
	  'Nf_Bdtemp'.
	  Salida:       n£mero de tabla seleccionada o -1 si no
			seleccion¢ ninguna o error
****************************************************************************/
int elige_msg(void)
{
STC_CUADRO c;
STC_ELEM *elem;
STC_ELEM_LISTA *tmsg;
FILE *fbd, *fsecc;
char l[E_MAXLNGLIN], nfsecc[13], buff[81], nmsg[4];
int i, lng, encontrado;

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,MSG_ANCHO,MSG_ALTO,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,cfg.color_dlginput,
  cfg.color_dlgsel,cfg.color_dlgtec,cfg.color_dlgtecboton);
elem=c_crea_elemento(&c,C_ELEM_LISTA,0,0," ^Tablas de mensajes ",MSG_ANCHO-2,
  MSG_ALTO-5,C_LSTNORMAL,C_LSTSINORDEN);
tmsg=(STC_ELEM_LISTA *)elem->info;
c_crea_elemento(&c,C_ELEM_BOTON,MSG_ALTO-4,1,"^Vale",8);
c_crea_elemento(&c,C_ELEM_BOTON,MSG_ALTO-4,11,"^Borrar",10);
c_crea_elemento(&c,C_ELEM_BOTON,MSG_ALTO-4,23,"^Salir",9);
c_crea_elemento(&c,C_ELEM_BOTON,MSG_ALTO-4,MSG_ANCHO-33,"^Nueva tabla",15);
nmsg[0]='\0';
c_crea_elemento(&c,C_ELEM_INPUT,PRO_ALTO-4,PRO_ANCHO-9,"T^abla",6,nmsg,
  sizeof(nmsg)-1);

/* crea lista de tablas de mensajes */
if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(-1);
}

while(1) {
	if(fgets(l,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			return(-1);
		}
	}

	/* si encuentra tabla de mensajes comprueba si existe fichero */
	/* temporal asociado; si es as¡ lee l¡nea de fichero temporal */
	/* si no lee l¡nea de fichero de base de datos */
	if(busca_marca(Mmsg,l)) {
		sprintf(nfsecc,Nf_Secc,&Mmsg[1]);
		i=pon_extensionsecc(l,nfsecc);
		if(msg_existe[i]==TRUE) {
			if((fsecc=fopen(nfsecc,"rt"))==NULL) {
				imprime_error(Merr_Aper);
				fclose(fbd);
				return(-1);
			}
			fgets(l,E_MAXLNGLIN,fsecc);
			fclose(fsecc);
		}
		else fgets(l,E_MAXLNGLIN,fbd);

		/* almacena entrada en lista */
		lng=strlen(l);
		if(l[lng-1]=='\n') l[lng-1]='\0';
		sprintf(buff,"%3i: %-75s",i,l);
		c_mete_en_lista(tmsg,buff);
	}
}

fclose(fbd);

c_abre(&c);

while(1) {
	i=c_gestiona(&c);

	if((i==-1) || (i==3)) break;
	else if(i==2) {
		i=atoi(tmsg->selecc);
		if(!i) imprime_error(Merr_Msg0borr);
		else if(pregunta_siono(" ¨Quieres borrar tabla "
		  "de mensajes? ")) {
			borra_seccmult(i,Mmsg,Merr_Msgnoex,msg_existe);
			break;
		}
	}
	else if(i==4) {
		i=atoi(nmsg);

		/* comprueba si n£mero dentro de rango permitido */
		if((i<0) || (i>=MAX_TMSG)) {
			imprime_error(Merr_Ntmsg);
			continue;
		}

		/* comprueba si existe fichero asociado a tabla */
		if(msg_existe[i]==TRUE) {
			imprime_error(Merr_Msgex);
			continue;
		}

		/* comprueba si existe tabla en fichero de base de datos */
		if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
			imprime_error(Merr_Aper);
			c_cierra(&c);
			c_elimina(&c);
			return(-1);
		}
		sprintf(buff,"%s %i",Mmsg,i);
		encontrado=0;
		while(1) {
			if(fgets(l,E_MAXLNGLIN,fbd)==NULL) {
				if(feof(fbd)) break;
				else {
					imprime_error(Merr_Lect);
					fclose(fbd);
					c_cierra(&c);
					c_elimina(&c);
					return(-1);
				}
			}
			if(busca_marca(buff,l)) {
				imprime_error(Merr_Msgex);
				encontrado=1;
				break;
			}
		}
		fclose(fbd);

		if(encontrado) continue;
		else if(crea_seccmult(i,Mmsg,Merr_Msgex,msg_existe)) {
			c_cierra(&c);
			c_elimina(&c);
			return(i);
		}
		break;
	}
	else {
		if(*tmsg->selecc) {
			c_cierra(&c);
			c_elimina(&c);
			return(atoi(tmsg->selecc));
		}
		break;
	}
}

c_cierra(&c);
c_elimina(&c);

return(-1);
}

/****************************************************************************
	EJECUTA: ejecuta un programa externo.
	  Entrada:      'ruta' ruta d¢nde se encuentra en programa
			'prg' nombre del programa
			'arg' cadena con los argumentos
			'modo' 0 no modifica modo de v¡deo, 1 activa
			modo texto antes de ejecutar programa
	  Salida:       1 si se pudo ejecutar, 0 si no
		      variables globales:-
			'cod_ejecuta' c¢digo de retorno del programa
****************************************************************************/
int ejecuta(char *ruta, char *prg, char *arg, int modo)
{
char programa[256];
unsigned codigo;

r_puntero(R_OCULTA);

/* pasa a modo texto antes de ejecutar */
if(modo) {
	vga_activa_fuente(NULL);
	vga_parpadeo(1);
	g_modovideo(G_MV_T80C);
}

/* reutiliza memoria */
e_vacia_bloque();
e_borra_texto();

/* crea nombre completo con ruta */
strcpy(programa,ruta);
strcat(programa,prg);

codigo=sspawn(programa,arg,".");

activa_vga();
m_cierra(m);
m_abre(m);
e_dibuja_editor();

cod_ejecuta=codigo;

if(codigo>8000) return(0);

return(1);
}

/****************************************************************************
	HAZ_FICH_SAL: crea nombre de fichero de salida del compilador a
	  partir del de entrada.
	  Entrada:      'fich' nombre de fichero de entrada
			'fsal' puntero a buffer d¢nde se dejar  nombre de
			fichero de salida
****************************************************************************/
void haz_fich_sal(char *fich, char *fsal)
{
char drive_n[MAXDRIVE], dir_n[MAXDIR], fname_n[MAXFILE], ext_n[MAXEXT];

fnsplit(fich,drive_n,dir_n,fname_n,ext_n);

/* el nombre del fichero de salida ser  como el de entrada pero */
/* con extensi¢n .DAT */
strcpy(fsal,drive_n);
strcat(fsal,dir_n);
strcat(fsal,fname_n);
strcat(fsal,".DAT");

}

/****************************************************************************
	COMPILA_BD: compila la base de datos actual.
	  Salida:       1 si se compil¢, 0 si error
****************************************************************************/
int compila_bd(void)
{
char fichero_csal[MAXPATH], argum[128], fichero_ed[MAXPATH];
int error=0;

if(!*base_datos) return(1);

/* borra fichero de errores */
remove(Nf_Err);

/* ruta completa de fichero de salida */
haz_fich_sal(base_datos,fichero_csal);

/* actualiza fichero temporal de base de datos */
if(!actualiza_bdtemp()) return(0);

/* argumentos */
argum[0]='\0';
strcat(argum," ");
strcat(argum,Nf_Bdtemp);
strcat(argum," ");
strcat(argum,fichero_csal);
strcat(argum," ");
strcat(argum,Arg_Cs);

e_nombre_fichero(fichero_ed);

if(!ejecuta(cfg.dir_sintac,"CS.EXE",argum,0)) {
	imprime_error(Merr_Nocomp);
	error=1;
}

e_carga_texto(fichero_ed);
e_dibuja_editor();

if(error) return(0);

/* procesa mensajes error del compilador */
if(errores_compil()) return(0);

/* otros errores (Ctrl+Break,...) */
if(cod_ejecuta!=0) return(0);

return(1);
}

/****************************************************************************
	HAZ_FICH_EXE: crea nombre de fichero ejecutable del linkador a
	  partir del de entrada.
	  Entrada:      'fich' nombre de fichero de entrada
			'fexe' puntero a buffer d¢nde se dejar  nombre de
			fichero ejecutable
****************************************************************************/
void haz_fich_exe(char *fich, char *fexe)
{
char drive_n[MAXDRIVE], dir_n[MAXDIR], fname_n[MAXFILE], ext_n[MAXEXT];

fnsplit(fich,drive_n,dir_n,fname_n,ext_n);

/* el nombre del fichero ejecutable ser  como el de entrada pero */
/* con extensi¢n .EXE */
strcpy(fexe,drive_n);
strcat(fexe,dir_n);
strcat(fexe,fname_n);
strcat(fexe,".EXE");

}

/****************************************************************************
	LINKA_BD: crea un ejecutable con la base de datos actual.
	  Salida:       1 si se compil¢, 0 si error
****************************************************************************/
int linka_bd(void)
{
char fichero_dat[MAXPATH], fichero_exe[MAXPATH], argum[128],
  fichero_ed[MAXPATH];
int error=0;

if(!*base_datos) return(1);

/* primero compila la base de datos */
if(!compila_bd()) return(0);

/* ruta completa de fichero compilado */
haz_fich_sal(base_datos,fichero_dat);

/* ruta completa de fichero de ejecutable */
haz_fich_exe(base_datos,fichero_exe);

/* argumentos */
argum[0]='\0';
strcat(argum," ");
strcat(argum,fichero_dat);
strcat(argum," ");
strcat(argum,fichero_exe);
strcat(argum," ");
strcat(argum,Arg_Cs);

e_nombre_fichero(fichero_ed);

if(!ejecuta(cfg.dir_sintac,"LKS.EXE",argum,0)) {
	imprime_error(Merr_Nolink);
	error=1;
}

e_carga_texto(fichero_ed);
e_dibuja_editor();

if(error) return(0);

return(1);
}

/****************************************************************************
	EJECUTA_BD: ejecuta la base de datos compilada, si el fichero de
	  base de datos ha sido modificado o si no se compil¢ lo compila.
	  Salida:       1 si se pudo ejecutar, 0 si error
****************************************************************************/
int ejecuta_bd(void)
{
STC_CUADRO c;
struct stat stfbd, stfdat;
char fichero_dat[MAXPATH], drive_n[MAXDRIVE], dir_n[MAXDIR],
  fname_n[MAXFILE], ext_n[MAXEXT], ant_dir[MAXDIR], argum[128],
  fichero_ed[MAXPATH];
int ant_drive, i, error=0;

if(!*base_datos) return(1);
if(!actualiza_bdtemp()) return(0);

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,AV_ANCHO,AV_ALTO,cfg.color_dlg,
  cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,cfg.color_dlginput,
  cfg.color_dlgsel,cfg.color_dlgtec,cfg.color_dlgtecboton);
c_crea_elemento(&c,C_ELEM_BOTON,AV_ALTO-5,14,"^Si",6);
c_crea_elemento(&c,C_ELEM_BOTON,AV_ALTO-5,22,"^No",6);
c_crea_elemento(&c,C_ELEM_BOTON,AV_ALTO-5,30,"Sa^lir",9);
c_crea_elemento(&c,C_ELEM_TEXTO,0,0,
  "         Base de datos ha sido modificada\n"
  "               ¨Quieres compilarla?",AV_ANCHO-2,4,C_TXTLINEA,C_TXTNOBORDE);

/* ruta completa de fichero de base de datos compilada */
haz_fich_sal(base_datos,fichero_dat);

/* si no existe base de datos compilada, la compila */
if(access(fichero_dat,0)){
	if(!compila_bd()) return(0);
}
else {
	stat(Nf_Bdtemp,&stfbd);
	stat(fichero_dat,&stfdat);

	/* si fecha de fichero temporal de base de datos es posterior a */
	/* fecha de fichero de base de datos compilada, recompila */
	if(stfbd.st_atime>stfdat.st_atime) {
		c_abre(&c);
		i=c_gestiona(&c);
		c_cierra(&c);
		c_elimina(&c);

		if((i==-1) || (i==2)) return(1);
		else if(i==0) {
			if(!compila_bd()) return(0);
		}
	}
}

/* cambia a unidad y directorio de datos */
fnsplit(base_datos,drive_n,dir_n,fname_n,ext_n);
ant_drive=getdisk();
getcwd(ant_dir,MAXDIR);
setdisk((int)(toupper(*drive_n)-'A'));
/* anula '\' final */
i=strlen(dir_n)-1;
if(dir_n[i]=='\\') dir_n[i]='\0';
chdir(dir_n);

/* argumentos */
argum[0]='\0';
strcat(argum," ");
strcat(argum,fichero_dat);
strcat(argum," ");
strcat(argum,Arg_Ds1);
strcat(argum," ");
strcat(argum,Arg_Ds2);

e_nombre_fichero(fichero_ed);

if(!ejecuta(cfg.dir_sintac,"DS.EXE",argum,1)) {
	imprime_error(Merr_Nods);
	error=1;
}

/* restaura unidad y directorio */
setdisk(ant_drive);
chdir(ant_dir);

e_carga_texto(fichero_ed);
e_dibuja_editor();

if(error) return(0);

return(1);
}

/****************************************************************************
	BUSCA_LINEA_SECC: busca a que secci¢n pertenece una l¡nea, y devuelve
	  la marca identificadora de esa secci¢n.
	  Entrada:      'nlin' n£mero de l¡nea (distinto de 0)
			'secc' puntero a cadena donde se colocar  marca de
			secci¢n (debe tener capacidad para al menos 9
			caracteres)
			'lin_secc' puntero a variable donde se guardar  el
			n£mero de l¡nea relativa al inicio de la secci¢n
	  Salida:       1 si se encontr¢, 0 si error
****************************************************************************/
int busca_linea_secc(unsigned long nlin, char *secc, int *lin_secc)
{
FILE *fbd;
char c[E_MAXLNGLIN], marca[9], *l;
unsigned long lin_marca=0, lin_act=0;
int i;

/* si el n£mero de l¡nea es 0, sale con error */
if(!nlin) return(0);

if((fbd=fopen(Nf_Bdtemp,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

/* inicializa cadenas de marca de secci¢n */
*secc='\0';
marca[0]='\0';

while(1) {
	if(fgets(c,E_MAXLNGLIN,fbd)==NULL) {
		if(feof(fbd)) break;
		else {
			imprime_error(Merr_Lect);
			fclose(fbd);
			return(0);
		}
	}

	/* incrementa n£mero de l¡nea del fichero de base de datos */
	lin_act++;

	/* si encuentra una marca de secci¢n, mira si el n£mero de l¡nea es */
	/* mayor que la que buscamos, si es as¡ devuelve la marca anterior */
	/* y n£mero de l¡nea relativo; si no almacena la marca y su l¡nea */
	if(es_marca_secc(c)==TRUE) {
		if(lin_act>nlin) {
			strcpy(secc,marca);
			*lin_secc=(int)(nlin-lin_marca);
			fclose(fbd);
			return(1);
		}
		else {
			lin_marca=lin_act;

			/* recoge la marca de secci¢n */
			for(l=c; *l!=MARCA_S; l++);
			for(i=0; i<4; i++, l++) marca[i]=(char)toupper(*l);
			marca[i]='\0';

			/* si es marca de secci¢n m£ltiple tambi‚n recoge */
			/* su n£mero */
			if(!strcmp(marca,Mpro) || !strcmp(marca,Mmsg)) {
				/* salta espacios */
				for(; ((*l==' ') || (*l=='\t')); l++);

				/* inserta espacio */
				marca[i++]=' ';

				/* recoge n£mero, m ximo 3 d¡gitos */
				for(; i<8; i++, l++) {
					if(strchr(CNum,*l)==NULL) break;
					marca[i]=*l;
				}
				marca[i]='\0';
			}
		}
	}
}

imprime_error(Merr_Lin);
fclose(fbd);

return(0);
}

/****************************************************************************
	ERRORES_COMPIL: muestra los errores del compilador, si se produjo
	  alguno.
	  Salida:       1 si se complet¢ operaci¢n, 0 si error o si no
			hay errores
****************************************************************************/
int errores_compil(void)
{
STC_CUADRO c;
STC_ELEM *elem;
STC_ELEM_LISTA *lsterr;
CURSOR_ED cur_pos;
FILE *ferr;
char lerr[81], *l, nl[41], seccion[9];
unsigned long nlin;
int i, lin_seccion;

/* si no hay fichero de errores, sale */
if(access(Nf_Err,0)!=0) return(0);

c_crea_cuadro(&c,NULL,C_CENT,C_CENT,CERR_ANCHO,CERR_ALTO,cfg.color_err,
  cfg.color_errs1,cfg.color_errs2,cfg.color_errboton,cfg.color_errinput,
  cfg.color_errsel,cfg.color_errtec,cfg.color_errtecboton);
elem=c_crea_elemento(&c,C_ELEM_LISTA,0,0," ^Errores ",CERR_ANCHO-2,
  CERR_ALTO-5,C_LSTNORMAL,C_LSTSINORDEN);
lsterr=(STC_ELEM_LISTA *)elem->info;
c_crea_elemento(&c,C_ELEM_BOTON,CERR_ALTO-4,1,"^Vale",8);
c_crea_elemento(&c,C_ELEM_BOTON,CERR_ALTO-4,11,"^Salir",9);

if((ferr=fopen(Nf_Err,"rt"))==NULL) {
	imprime_error(Merr_Aper);
	return(0);
}

/* lee y almacena mensajes de error */
while(1) {
	if(fgets(lerr,81,ferr)==NULL) break;

	/* elimina '\n' final, si lo hay */
	i=strlen(lerr);
	if(lerr[i-1]=='\n') lerr[i-1]='\0';

	if(!c_mete_en_lista(lsterr,lerr)) break;
}

fclose(ferr);

c_abre(&c);
i=c_gestiona(&c);
c_cierra(&c);
c_elimina(&c);

if(i==1) {
	/* separa n£mero de l¡nea */
	for(l=lsterr->selecc, i=0; ((*l!=':') && (i<41)); l++, i++) nl[i]=*l;
	nl[i]='\0';
	nlin=atol(nl);

	if(busca_linea_secc(nlin,seccion,&lin_seccion)) {
		cur_pos.lin=lin_seccion-1;
		cur_pos.col=0;
		cur_pos.filcur=0;
		cur_pos.colcur=0;
		if(nlin==(unsigned long)lin_seccion) edita_seccion(Mconst,
		  &cur_pos,1);
		else edita_seccion(seccion,&cur_pos,1);
	}
}

return(1);
}

/****************************************************************************
	AYUDA_SOBRE: busca ayuda sobre un tema.
****************************************************************************/
void ayuda_sobre(void)
{
STC_CUADRO c;
char tema[A_LNGREF];
int i;

c_crea_cuadro(&c," Ayuda sobre... ",C_CENT,C_CENT,AYT_ANCHO,AYT_ALTO,
  cfg.color_dlg,cfg.color_dlgs1,cfg.color_dlgs2,cfg.color_dlgboton,
  cfg.color_dlginput,cfg.color_dlgsel,cfg.color_dlgtec,cfg.color_dlgtecboton);
c_crea_elemento(&c,C_ELEM_INPUT,0,6,"^Tema",AYT_ANCHO-9,tema,sizeof(tema)-1);
c_crea_elemento(&c,C_ELEM_TEXTO,2,0,
  " Teclea el tema sobre el que buscar\n ayuda, p. ej. un condacto.",
  AYT_ANCHO-2,4,C_TXTLINEA,C_TXTBORDE);
c_crea_elemento(&c,C_ELEM_BOTON,AYT_ALTO-4,9,"^Vale",8);
c_crea_elemento(&c,C_ELEM_BOTON,AYT_ALTO-4,19,"^Salir",9);

*tema='\0';
c_abre(&c);
do {
	i=c_gestiona(&c);
	if(i==2) {
		c_cierra(&c);
		ayuda(tema);
		c_abre(&c);
	}
} while((i!=-1) && (i!=3));

c_cierra(&c);
c_elimina(&c);

}

/****************************************************************************
	UTILIDADES: men£ de utilidades, se usa un fichero ASCII que debe
	  contener la ruta, texto de men£ y nombre de las utilidades que
	  se quieren incluir (una utilidad en cada l¡nea del fichero, los
	  apartados separados por comas).
****************************************************************************/
void utilidades(void)
{
STC_MENU *mutil;
FILE *futil;
char nf_utl[MAXPATH], lin[81], menu_util[1024], *l, ruta[MAX_NUTILS][129],
  texto_opc[30], nombre_utls[MAX_NUTILS][13], fichero_ed[MAXPATH];
int i, num_opc=0;

strcpy(nf_utl,cfg.dir_sintac);
strcat(nf_utl,"EDS.UTL");
if((futil=fopen(nf_utl,"rt"))==NULL) return;

*menu_util='\0';

while(1) {
	if(fgets(lin,81,futil)==NULL) {
		if(feof(futil)) break;
		else {
			fclose(futil);
			return;
		}
	}

	l=lin;

	/* recoge ruta */
	/* NOTA: la ruta debe acabar en barra invertida '\' */
	i=0;
	while(*l && (*l!='\n') && (*l!=',') && (i<128))
	  ruta[num_opc][i++]=*l++;
	ruta[num_opc][i]='\0';
	/* si ruta = '*SINTAC' asigna directorio del SINTAC */
	if(!strcmp(ruta[num_opc],"*SINTAC")) strcpy(ruta[num_opc],
	  cfg.dir_sintac);
	/* si ruta = '*UTIL' asigna directorio de utilidades */
	if(!strcmp(ruta[num_opc],"*UTIL")) strcpy(ruta[num_opc],cfg.dir_util);

	/* recoge texto de la opci¢n */
	if(*l) l++;
	else return;
	i=0;
	while(*l && (*l!='\n') && (*l!=',') && (i<29)) texto_opc[i++]=*l++;
	texto_opc[i]='\0';
	if(num_opc!=0) strcat(menu_util,":");
	strcat(menu_util,texto_opc);

	/* recoge nombre de la utilidad */
	if(*l) l++;
	else return;
	i=0;
	while(*l && (*l!='\n') && (i<12)) nombre_utls[num_opc][i++]=*l++;
	nombre_utls[num_opc][i]='\0';

	num_opc++;
	if(num_opc>=MAX_NUTILS) break;
}

fclose(futil);

if(!num_opc) return;

mutil=m_crea(MENU_VERT,NULL,menu_util,1,42,0);

m_abre(mutil);
i=m_elige_opcion(mutil);
m_cierra(mutil);
m_elimina(mutil);

e_nombre_fichero(fichero_ed);

if((i>=0) && (i<num_opc)) {
	if(!ejecuta(ruta[i],nombre_utls[i],"",1)) imprime_error(Merr_Noutil);
}

e_carga_texto(fichero_ed);
e_dibuja_editor();

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
	SELECC_COLOR: rutina para selecci¢n de colores.
	  Entrada:	'color' color actual
	  Salida:	color seleccionado
****************************************************************************/
unsigned char selecc_color(unsigned char color)
{
STC_VENTANA v;
STC_RATON r;
unsigned char a;
int i, j, salida=0, tecla;

v_crea(&v,0,0,35,18,COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2," Colores ",1);

v_abre(&v);
r_puntero(R_MUESTRA);
esconde_cursor();

for(i=0; i<16; i++) {
	for(j=0; j<16; j++) {
		v_color(&v,(i << 4) | j);
		v_pon_cursor(&v,j,(i*2)+1);
		v_impc(&v,'*');
	}
}

/* calcula posici¢n de atributo actual */
i=color >> 4;
j=color & 0x0f;

v_color(&v,COLOR_DLGS1);
do {
	v_pon_cursor(&v,j,(i*2)+2);
	v_impc(&v,'<');

	/* comprueba estado del rat¢n */
	r_estado(&r);

	if(r.boton1) {
		if((r.col>(v.col+1)) && (r.col<(v.col+v.ancho-1)) &&
		  (r.fil>v.fil) && (r.fil<(v.fil+v.alto-1))) {
			v_pon_cursor(&v,j,(i*2)+2);
			v_impc(&v,' ');
			i=(r.col-(v.col+2))/2;
			j=r.fil-(v.fil+1);
		}

	}
	else if(r.boton2) salida=1;
	else if(bioskey(1)) {
		tecla=bioskey(0);
		switch((tecla >> 8) & 0x00ff) {
			case 0x01 :		/* ESCAPE */
				salida=1;
				break;
			case 0x1c :		/* RETURN */
				salida=1;
				break;
			case 0x4b :		/* cursor izquierda */
				if(i>0) {
					v_pon_cursor(&v,j,(i*2)+2);
					v_impc(&v,' ');
					i--;
				}
				break;
			case 0x4d :		/* cursor derecha */
				if(i<15) {
					v_pon_cursor(&v,j,(i*2)+2);
					v_impc(&v,' ');
					i++;
				}
				break;
			case 0x48 :		/* cursor arriba */
				if(j>0) {
					v_pon_cursor(&v,j,(i*2)+2);
					v_impc(&v,' ');
					j--;
				}
				break;
			case 0x50 :		/* cursor abajo */
				if(j<15) {
					v_pon_cursor(&v,j,(i*2)+2);
					v_impc(&v,' ');
					j++;
				}
				break;
		}
	}
} while(!salida);

r_puntero(R_OCULTA);
v_cierra(&v);

a=(i << 4) | j;

return(a);
}

/****************************************************************************
	ELIGE_COLORES: selecci¢n de colores.
	  Entrada:      'titulo' t¡tulo de la ventana
			'clr_princ' puntero a color principal
			'clr_s1', 'clr_s2' punteros a colores de
			sombreado
			'clr_boton' puntero a color de bot¢n
			'clr_input' puntero a color de zona de input
			'clr_sel' puntero a color de selecci¢n
			'clr_tecla' puntero a color de tecla de
			activaci¢n
			'clr_tecboton' puntero a color de tecla de activaci¢n
			de bot¢n
			'modo' 0 normal, 1 selecci¢n colores resalte sintaxis
****************************************************************************/
void elige_colores(char *titulo, unsigned char *clr_princ,
  unsigned char *clr_s1, unsigned char *clr_s2, unsigned char *clr_boton,
  unsigned char *clr_input, unsigned char *clr_sel, unsigned char *clr_tecla,
  unsigned char *clr_tecboton, int modo)
{
STC_VENTANA vcolor;
STC_CUADRO ccolor;
int i, fin;

v_crea(&vcolor,VCOLOR_FIL,VCOLOR_COL,VCOLOR_ANCHO,VCOLOR_ALTO,*clr_princ,
  *clr_s1,*clr_s2,titulo,1);
v_abre(&vcolor);

c_crea_cuadro(&ccolor,NULL,CCOLOR_FIL,CCOLOR_COL,CCOLOR_ANCHO,CCOLOR_ALTO,
  COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,COLOR_DLGBOTON,COLOR_DLGINPUT,
  COLOR_DLGSEL,COLOR_DLGTEC,COLOR_DLGTECBOTON);
if(!modo) {
	c_crea_elemento(&ccolor,C_ELEM_BOTON,0,0,"^Principal",
	  ((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,0,((CCOLOR_ANCHO-2)/2)+1,
	  "Borde ^1",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,2,0,"Borde ^2",
	  ((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,2,((CCOLOR_ANCHO-2)/2)+1,
	  "^Bot¢n",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,4,0,"^Input",
	  ((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,4,((CCOLOR_ANCHO-2)/2)+1,
	  "^Tecla bot¢n",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,6,0,"T^eclas",
	  ((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,6,((CCOLOR_ANCHO-2)/2)+1,
	  "^Seleccionado",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,8,0,"^Vale",
	  ((CCOLOR_ANCHO-2)/2)-1);
}
else {
	c_crea_elemento(&ccolor,C_ELEM_BOTON,0,0,"^Principal",
	  ((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,0,((CCOLOR_ANCHO-2)/2)+1,
	  "Borde ^1",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,2,0,"Borde ^2",
	  ((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,2,((CCOLOR_ANCHO-2)/2)+1,
	  "^Comentario",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,4,0,"^Especiales",
	  ((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,4,((CCOLOR_ANCHO-2)/2)+1,
	  "^Palabra clv.",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,6,0,"",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,6,((CCOLOR_ANCHO-2)/2)+1,
	  "^Seleccionado",((CCOLOR_ANCHO-2)/2)-1);
	c_crea_elemento(&ccolor,C_ELEM_BOTON,8,0,"^Vale",
	  ((CCOLOR_ANCHO-2)/2)-1);
}
c_abre(&ccolor);

fin=0;
while(!fin) {
	vcolor.clr_princ=*clr_princ;
	vcolor.clr_s1=*clr_s1;
	vcolor.clr_s2=*clr_s2;
	v_dibuja(&vcolor,1);

	v_pon_cursor(&vcolor,1,1);
	v_color(&vcolor,*clr_princ);
	v_impcad(&vcolor," Texto normal ",V_NORELLENA);

	if(clr_boton!=NULL) {
		v_pon_cursor(&vcolor,3,1);
		v_color(&vcolor,*clr_boton);
		if(!modo) v_impcad(&vcolor,"< Bot¢n >",V_NORELLENA);
		else v_impcad(&vcolor," Comentario ",V_NORELLENA);
	}

	if(clr_tecboton!=NULL) {
	       	v_color(&vcolor,*clr_tecboton);
		if(!modo) {
			v_pon_cursor(&vcolor,3,3);
			v_impc(&vcolor,'B');
		}
		else {
			v_pon_cursor(&vcolor,5,1);
			v_impcad(&vcolor," Palabra clave ",V_NORELLENA);
		}
	}

	if(clr_input!=NULL) {
		if(!modo) {
			v_pon_cursor(&vcolor,3,12);
			v_color(&vcolor,*clr_princ);
			v_impcad(&vcolor,"Input ",V_NORELLENA);
			v_color(&vcolor,*clr_input);
			v_impcad(&vcolor,"[ abc... ]",V_NORELLENA);
		}
		else {
			v_pon_cursor(&vcolor,3,14);
			v_color(&vcolor,*clr_input);
			v_impcad(&vcolor," Especial ",V_NORELLENA);
		}
	}

	if(clr_tecla!=NULL) {
		if(!modo) {
			v_pon_cursor(&vcolor,5,1);
			v_color(&vcolor,*clr_tecla);
			v_impcad(&vcolor," Teclas activaci¢n ",V_NORELLENA);
		}
	}

	if(clr_sel!=NULL) {
		v_pon_cursor(&vcolor,7,1);
		v_color(&vcolor,*clr_sel);
		v_impcad(&vcolor," Seleccionado ",V_NORELLENA);
	}

	i=c_gestiona(&ccolor);
	switch(i) {
		case 0 :
			if(clr_princ==NULL) break;
			*clr_princ=selecc_color(*clr_princ);
			break;
		case 1 :
			if(clr_s1==NULL) break;
			*clr_s1=selecc_color(*clr_s1);
			break;
		case 2 :
			if(clr_s2==NULL) break;
			*clr_s2=selecc_color(*clr_s2);
			break;
		case 3 :
			if(clr_boton==NULL) break;
			*clr_boton=selecc_color(*clr_boton);
			break;
		case 4 :
			if(clr_input==NULL) break;
			*clr_input=selecc_color(*clr_input);
			break;
		case 5 :
			if(clr_tecboton==NULL) break;
			*clr_tecboton=selecc_color(*clr_tecboton);
			break;
		case 6 :
			if(clr_tecla==NULL) break;
			*clr_tecla=selecc_color(*clr_tecla);
			break;
		case 7 :
			if(clr_sel==NULL) break;
			*clr_sel=selecc_color(*clr_sel);
			break;
		case 8 :
		case -1 :
			fin=1;
			break;
	}

}

v_cierra(&vcolor);
c_cierra(&ccolor);
c_elimina(&ccolor);

}

/****************************************************************************
	CONFIGURA: configuraci¢n del entorno.
	  Salida:	1 si modific¢ la configuraci¢n, 0 si no
****************************************************************************/
int configura(void)
{
STC_CFG cfg0;
STC_CUADRO cdir;
STC_MENU *mcolor;
FILE *fcfg;
int i, fin, opcion, modifica=0;

/* guarda configuraci¢n actual */
memcpy(&cfg0,&cfg,sizeof(cfg));

m_color(COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,COLOR_DLGTEC,COLOR_DLGSEL);
mcolor=m_crea(MENU_VERT," Colores SINTAC ",
  " ^Men£ de opciones                  : ^Editor: ^Cuadros di logo:"
  " ^Ventana ayuda: Ventana e^rrores: C^ompilador| ^Salir",
  MCOLOR_FIL,MCOLOR_COL,0);

c_crea_cuadro(&cdir," Directorios del SINTAC ",C_CENT,C_CENT,CDIR_ANCHO,
  CDIR_ALTO,COLOR_DLG,COLOR_DLGS1,COLOR_DLGS2,COLOR_DLGBOTON,COLOR_DLGINPUT,
  COLOR_DLGSEL,COLOR_DLGTEC,COLOR_DLGTECBOTON);
c_crea_elemento(&cdir,C_ELEM_INPUT,1,16,"^SINTAC",CDIR_ANCHO-19,cfg.dir_sintac,
  MAXPATH-1);
c_crea_elemento(&cdir,C_ELEM_INPUT,3,16,"^Bases de datos",CDIR_ANCHO-19,
  cfg.dir_bd,MAXPATH-1);
c_crea_elemento(&cdir,C_ELEM_INPUT,5,16,"^Utilidades",CDIR_ANCHO-19,
  cfg.dir_util,MAXPATH-1);
c_crea_elemento(&cdir,C_ELEM_BOTON,7,17,"^Vale",8);
c_crea_elemento(&cdir,C_ELEM_BOTON,7,27,"^Salir",9);
c_crea_elemento(&cdir,C_ELEM_BOTON,7,38,"^Colores",11);

c_abre(&cdir);

fin=0;
while(!fin) {
	i=c_gestiona(&cdir);

	ajusta_ndir(cfg.dir_sintac);
	ajusta_ndir(cfg.dir_bd);
	ajusta_ndir(cfg.dir_util);

	if((i==-1) || (i==4)) {
			/* restaura configuraci¢n anterior */
			memcpy(&cfg,&cfg0,sizeof(cfg0));
			modifica=0;
			fin=1;
	}
	if(i==3) {
		if((fcfg=fopen(NF_CFG,"wb"))==NULL) break;
		fwrite(&cfg,sizeof(STC_CFG),1,fcfg);
		fclose(fcfg);
		modifica=1;
		fin=1;
	}
	else if(i==5) {
		m_abre(mcolor);
		do {
			opcion=m_elige_opcion(mcolor);
			switch(opcion) {
				case 0 :
					elige_colores(" Men£ opciones ",
				  	  &cfg.color_men,&cfg.color_mens1,
				  	  &cfg.color_mens2,NULL,NULL,
					  &cfg.color_mensel,&cfg.color_mentec,
				          NULL,0);
					break;
				case 1 :
					elige_colores(" Editor ",
					  &cfg.color_ved,&cfg.color_veds1,
					  &cfg.color_veds2,&cfg.color_vedcoment,
					  &cfg.color_vedesp,&cfg.color_vedblq,
					  NULL,&cfg.color_vedpalclv,1);
					break;
				case 2 :
					elige_colores(" Cuadros di logo ",
				  	  &cfg.color_dlg,&cfg.color_dlgs1,
				  	  &cfg.color_dlgs2,&cfg.color_dlgboton,
					  &cfg.color_dlginput,&cfg.color_dlgsel,
					  &cfg.color_dlgtec,&cfg.color_dlgtecboton,
					  0);
					break;
				case 3 :
					elige_colores(" Ventana de ayuda ",
				  	  &cfg.color_ayd,&cfg.color_ayds1,
				  	  &cfg.color_ayds2,&cfg.color_aydboton,
					  &cfg.color_aydinput,&cfg.color_aydsel,
					  &cfg.color_aydtec,&cfg.color_aydtecboton,
				  	  0);
					break;
				case 4 :
					elige_colores(" Ventana de errores ",
				          &cfg.color_err,&cfg.color_errs1,
				  	  &cfg.color_errs2,&cfg.color_errboton,
					  &cfg.color_errinput,&cfg.color_errsel,
					  &cfg.color_errtec,&cfg.color_errtecboton,
					  0);
					break;
				case 5 :
					elige_colores(" Compilador ",&cfg.color_cs,
			  	  	  &cfg.color_css1,&cfg.color_css2,
			  	  	  &cfg.color_csboton,&cfg.color_csinput,
					  &cfg.color_cssel,&cfg.color_cstec,
					  &cfg.color_cstecboton,0);
					break;
			}
		} while((opcion!=-1) && (opcion!=7));
		m_cierra(mcolor);
	}
}

c_cierra(&cdir);
c_elimina(&cdir);
m_elimina(mcolor);
m_color(cfg.color_men,cfg.color_mens1,cfg.color_mens2,cfg.color_mentec,
  cfg.color_mensel);

return(modifica);
}
