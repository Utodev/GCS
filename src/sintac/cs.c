/****************************************************************************
			      COMPILADOR SINTAC
			    (c)1995 JSJ Soft Ltd.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <io.h>
#include <dir.h>
#include <bios.h>
#include <dos.h>
#include "graf.h"
#include "rutvga.h"
#include "ventana.h"
#include "raton.h"
#include "cuadro.h"
#include "version.h"
#include "sintac.h"
#include "cserr.h"
#include "compil.h"
#include "color.h"
#include "cs.h"

/*** Variables externas ***/
extern char *c_Borde_Sel;
extern char *c_Esp;

/*** Variables globales ***/
/* tama§o del STACK */
unsigned _stklen=8192;

STC_CFG cfg;

/* cuadros de di†logo */
STC_CUADRO ccomp;               /* cuadro del compilador */
STC_CUADRO cerr;                /* cuadro de errores */
STC_ELEM *lsterr;

/* buffers para mandar mensajes a ventana */
char buff[81], vmsg[129];

/* indicadores del compilador */
BOOLEAN cs_eds=FALSE;           /* si se ejecuta desde entorno */

char *Nferr=NF_ERR;             /* nombre de fichero para almacenar errores */
FILE *ferr=NULL;                /* puntero a fichero de errores */

/* nombres de ficheros de entrada y salida */
char nf_ent[MAXPATH], nf_sal[MAXPATH];

/* tabla de nombres de ficheros temporales para cada secci¢n */
/* se reservan MAXPATH caracteres para almacenar path completo */
/* adem†s hace falta una tabla adicional para indicar que ficheros */
/* temporales est†n abiertos */
char tf_nombre[N_SECCS][MAXPATH];
FILE *tf_file[N_SECCS];
BOOLEAN tf_abierto[N_SECCS]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};

/* mensajes */
char *Msg_Err="ERROR: %s";
char *Str_Compil0=" %-20s :";
char *Str_Compil=" %-20s : %5u   %5u bytes";
char *Secc_Conx="Conexiones";

/* cabecera del fichero compilado */
CAB_SINTAC cab;

/* punteros a buffers de memoria para compilar secciones */
char *buff_compil1=NULL;
BYTE *buff_compil2=NULL;

/* tabla para guardar el vocabulario */
struct palabra vocabulario[NUM_PAL];

/* tabla para mensajes del sistema */
unsigned tab_desp_msy[MAX_MSY];

/* tabla para mensajes */
unsigned tab_desp_msg[MAX_MSG];

/* tablas para localidades y conexiones */
unsigned tab_desp_loc[MAX_LOC];
unsigned tab_desp_conx[MAX_LOC];

/* tabla para objetos */
unsigned tab_desp_obj[MAX_OBJ];

/* tabla para procesos */
unsigned tab_desp_pro[MAX_PRO];

/* identificadores de las secciones */
/* NOTA: el orden viene determinado por las constantes */
/* definidas en SINTAC.H */
char id_secc[N_SECCS][4]={"VOC", "MSY", "MSG", "LOC", "OBJ", "PRO"};
/* nombres de las secciones */
char *nomb_secc[N_SECCS]={
	"Vocabulario",
	"Mensajes del sistema",
	"Mensajes",
	"Localidades",
	"Objetos",
	"Procesos",
};

/* indicador de ruptura */
int ruptura;

/*** Programa principal ***/
void main(int argc, char *argv[])
{
size_t memoria;
int i;

/* instala 'handler' de errores cr°ticos */
harderr(int24_hnd);

/* analiza par†metros de entrada */
analiza_args(argc,argv);

if(cs_eds==FALSE) {
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

/* lee fichero de configuraci¢n */
lee_cfg(argv[0]);

/* cuadro di†logo principal */
c_crea_cuadro(&ccomp," COMPILADOR "COPYRIGHT" versi¢n "VERSION" ",CCOMP_FIL,
  CCOMP_COL,CCOMP_ANCHO,CCOMP_ALTO,cfg.color_cs,cfg.color_css1,cfg.color_css2,
  cfg.color_csboton,cfg.color_csinput,cfg.color_cssel,cfg.color_cstec,
  cfg.color_cstecboton);
c_crea_elemento(&ccomp,C_ELEM_INPUT,1,17,"^Fichero entrada",CCOMP_ANCHO-20,
  nf_ent,MAXPATH-1);
c_crea_elemento(&ccomp,C_ELEM_INPUT,3,17,"Fichero ^salida",CCOMP_ANCHO-20,
  nf_sal,MAXPATH-1);
if(cs_eds==FALSE) {
	c_crea_elemento(&ccomp,C_ELEM_BOTON,5,1,"^Compilar",12);
	c_crea_elemento(&ccomp,C_ELEM_BOTON,7,1,"Sa^lir",12);
}

/* cuadro de di†logo para lista de errores */
c_crea_cuadro(&cerr," Errores ",CERR_FIL,CERR_COL,CERR_ANCHO,CERR_ALTO,
  cfg.color_err,cfg.color_errs1,cfg.color_errs2,cfg.color_errboton,
  cfg.color_errinput,cfg.color_errsel,cfg.color_errtec,cfg.color_errtecboton);
lsterr=c_crea_elemento(&cerr,C_ELEM_LISTA,0,0,"^Lista de errores",CERR_ANCHO-2,
  CERR_ALTO-4,C_LSTNORMAL,C_LSTSINORDEN);
c_crea_elemento(&cerr,C_ELEM_BOTON,CERR_ALTO-4,(CERR_ANCHO-8)/2,"^Vale",8);

/* inicializa variables de cabecera */
cab.v_mov=V_MOV;       /* m†ximo n£m. de verbo de movimiento */
cab.n_conv=N_CONV;     /*   "     "   de nombre convertible */
cab.n_prop=N_PROP;     /*   "     "   de nombre propio */

memoria=reserva_memoria();

/* crea fichero para almacenar errores si se ejecuta desde entorno */
if(cs_eds==TRUE) if((ferr=fopen(Nferr,"wt"))==NULL) imp_error(_E_FTMP,0);

/* cuadro de di†logo del compilador */
c_abre(&ccomp);
if(cs_eds==FALSE) {
	do {
		i=c_gestiona(&ccomp);
		if((i==-1) || (i==3)) fin_prg(1);
	} while((*nf_ent=='\0') || (i!=2));
	if(*nf_sal=='\0') {
		construye_nfsal();
		c_dibuja_elemento(&ccomp,lsterr);
	}
}

for(i=0; i<N_SECCS; i++) {
	sprintf(buff,Str_Compil0,nomb_secc[i]);
	if(i<=LOC) v_pon_cursor(&ccomp.v,i+FIL_SECCS,COL_SECCS);
	else v_pon_cursor(&ccomp.v,i+FIL_SECCS+1,COL_SECCS);
	v_impcad(&ccomp.v,buff,V_RELLENA);
	if(i==LOC) {
		v_pon_cursor(&ccomp.v,i+FIL_SECCS+1,COL_SECCS);
		sprintf(buff,Str_Compil0,Secc_Conx);
		v_impcad(&ccomp.v,buff,V_RELLENA);
	}
}

/* instala 'handler' para Ctrl+Break */
ruptura=getcbrk();
setcbrk(1);
ctrlbrk(c_break);
men_info(" Ctrl+Break para salir ");
esconde_cursor();

crea_ficheros_temp();

/* mira a ver si est†n definidas las constantes V_MOV, N_CONV y N_PROP */
if(coge_const("V_MOV",&cab.v_mov)) aviso(0,"constante V_MOV no definida");
if(coge_const("N_CONV",&cab.n_conv)) aviso(0,"constante N_CONV no definida");
if(coge_const("N_PROP",&cab.n_prop)) aviso(0,"constante N_PROP no definida");

compila_seccs(memoria);

fin_prg(0);

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
	C_BREAK: rutina de respuesta a Ctrl+Break.
****************************************************************************/
int c_break(void)
{

/* quita 'Ctrl+Break' del buffer de teclado */
bioskey(0);

fin_prg(1);

return(0);
}

/****************************************************************************
	FIN_PRG: finaliza el programa cerrando ficheros y eliminando
	  los temporales (si se abrieron).
	  Entrada:      'codigo' c¢digo de retorno del programa
****************************************************************************/
void fin_prg(int codigo)
{
STC_RATON r;
int i;

/* restaura indicador de ruptura */
setcbrk(ruptura);

/* elimina los ficheros temporales que existan todav°a tras */
/* acabar la compilaci¢n */
fcloseall();
for(i=0; i<N_SECCS; i++) if(tf_abierto[i]) remove(tf_nombre[i]);

/* libera memoria */
free(buff_compil1);
free(buff_compil2);

/* si hay errores los muestra */
if(((STC_ELEM_LISTA *)(lsterr->info))->num_elementos!=0) {
	c_abre(&cerr);
	do {
		i=c_gestiona(&cerr);
	} while((i!=-1) && (i!=1));
	c_cierra(&cerr);
}

/* elimina cuadros de di†logo */
c_elimina(&ccomp);
c_elimina(&cerr);

/* elimina lista de errores */
c_borra_lista((STC_ELEM_LISTA *)(lsterr->info));

/* si hay fichero de mensajes de error y est† vac°o, lo borra */
if(!access(Nferr,0)) {
	if((ferr=fopen(Nferr,"rt"))!=NULL) {
		i=fileno(ferr);
		if(filelength(i)==0) {
			fclose(ferr);
			remove(Nferr);
		}
		else fclose(ferr);
	}
}

if(!codigo) {
	men_info("    Pulsa una tecla    ");
	if(cs_eds==TRUE) r_inicializa();
	do {
		if(bioskey(1)) {
			bioskey(0);
			break;
		}
		r_estado(&r);
	} while(!r.boton1 && !r.boton2);
}

r_puntero(R_OCULTA);
c_cierra(&ccomp);

if(cs_eds==FALSE) {
	g_modovideo(G_MV_T80C);
	vga_activa_fuente(NULL);
	vga_parpadeo(1);
}

/* si hubo errores borra fichero de salida */
if(codigo) remove(nf_sal);

exit(codigo);

}

/****************************************************************************
	MEN_INFO: imprime un mensaje informativo en la ventana del
	  compilador.
	  Entrada:	'men' mensaje
****************************************************************************/
void men_info(char *men)
{
int fil, col;

fil=ccomp.v.fil+ccomp.v.alto-1;
col=ccomp.v.col+((ccomp.v.ancho-strlen(men))/2);

while(*men) v_impcar(fil,col++,*men++,ccomp.v.clr_princ);

}

/****************************************************************************
	MEN_ERROR: devuelve el mensaje asociado a un c¢digo de error.
	  Entrada:	'cod_err' c¢digo del error
****************************************************************************/
char *men_error(int cod_err)
{
static char merr[81];
FILE *fmerr;
char nf_merr[MAXPATH];
int i;

strcpy(nf_merr,cfg.dir_sintac);
strcat(nf_merr,NF_CSERR);

if((fmerr=fopen(nf_merr,"rt"))!=NULL) {
	for(i=0; i<=cod_err; i++) {
		if(fgets(merr,81,fmerr)==NULL) {
			fclose(fmerr);
			sprintf(merr,"ERROR %i",cod_err);
			break;
		}
	}
	fclose(fmerr);
}
else sprintf(merr,"ERROR %i",cod_err);

/* elimina '\n' final, si lo hay */
for(i=0; i<81; i++) if(merr[i]=='\n') merr[i]='\0';

return(merr);
}

/****************************************************************************
	IMP_ERROR: imprime un mensaje de error y sale del programa.
	  Los c¢digos de error se guardan en la variable global 'cod_error'
	  Entrada:	'cod_err' c¢digo del error
			'nlin' n£mero de l°nea del fichero de entrada donde
			se detect¢ el error, si es 0 no se imprime n£mero de
			l°nea
****************************************************************************/
void imp_error(unsigned cod_err, unsigned long nlin)
{

if(cs_eds==FALSE) {
	sprintf(buff,Msg_Err,men_error(cod_err));
	strcpy(vmsg,buff);
	if(nlin) {
		sprintf(buff,", l°nea %lu",nlin);
		strcat(vmsg,buff);
	}
	c_mete_en_lista((STC_ELEM_LISTA *)(lsterr->info),vmsg);
}

/* si hay abierto fichero de errores, manda l°nea y mensaje a fichero */
if(ferr!=NULL) fprintf(ferr,"%lu: %s\n",nlin,men_error(cod_err));

fcloseall();

/* si es error de escritura en fichero de salida, lo borra si existe */
if(cod_err==_E_EFOU) {
	if(!access(nf_sal,0)) remove(nf_sal);
}

fin_prg(1);

}

/****************************************************************************
	RESERVA_MEMORIA: reserva buffers de memoria para compilar.
	  Salida:       tama§o del bloque de memoria reservado.
****************************************************************************/
size_t reserva_memoria(void)
{
size_t mem=TAM_MEM;

/* inicializa punteros a NULL */
buff_compil1=NULL;
buff_compil2=NULL;

do {
	if((buff_compil1=(char *)malloc(mem))==NULL) mem/=2;
	else if((buff_compil2=(BYTE *)malloc(mem))==NULL) {
		free(buff_compil1);
		buff_compil1=NULL;
		mem/=2;
	}

	if(mem<(TAM_MEM/16)) imp_error(_E_MMEM,0);

} while((buff_compil1==NULL) && (buff_compil2==NULL));

return(mem);
}

/****************************************************************************
	CREA_FICHEROS_TEMP: crea los ficheros temporales del compilador
	  separando las secciones.
****************************************************************************/
void crea_ficheros_temp(void)
{
FILE *f_ent;
char linea[LONG_LIN], *l, sec[4], lnum[40];
int i, seccion;
BOOLEAN secc_encontrada[N_SECCS]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
BOOLEAN err_sec=FALSE;
unsigned long num_lin;
COD_ERR err_const;

if((f_ent=fopen(nf_ent,"rt"))==NULL) imp_error(_E_AFIN,0);

/* inicializa variables */
for(i=0; i<N_SECCS; secc_encontrada[i]=FALSE, i++);
/* aunque se considera que la primera l°nea es la n£mero 1 */
/* la variable hay que inicializarla a 0 para que al leer la primera */
/* l°nea se incremente a 1 */
num_lin=0;

do {
	/* va leyendo lineas del fichero de entrada */
	/* sigue mientras encuentre comentarios o lineas nulas */
	/* si encuentra una marca de secci¢n comienza proceso de */
	/* separaci¢n y si encuentra una L_NORM saldr† con error */
	do {
		lee_linea(f_ent,linea,&num_lin);
		/* si encuentra una linea que no es comentario, nula */
		/* o marca de secci¢n indica error */
		if((i=tipo_lin(linea))==L_NORM) imp_error(_E_MSCC,num_lin);
	} while(((i==L_COMENT) || (i==L_NULA)) && !feof(f_ent));

	/* si encontr¢ fin de fichero, sale del bucle de creaci¢n de */
	/* ficheros temporales */
	if(feof(f_ent)) break;

	/* comprueba si encontr¢ una definici¢n de constante */
	if(i==L_CONST) {
		l=linea;
		/* salta espacios iniciales y deja puntero */
		/* al inicio del nombre de la constante */
		l=salta_espacios(l);
		l+=2;
		l=salta_espacios(l);

		/* intenta almacenar la constante en la tabla */
		err_const=mete_const(l,num_lin);
		if(err_const) imp_error(err_const,num_lin);

		/* vuelve al inicio del bucle */
		continue;
	}

	/* aqu° llegar† si encontr¢ una linea con formato de marca */
	/* de secci¢n, es decir, su primer car†cter no blanco ser† '\' */
	l=linea;
	l=salta_espacios(l);
	/* salta el car†cter '\' */
	l++;

	/* copia identificador de secci¢n en 'sec[]' */
	/* a§adiendo un '\0' en el £ltimo car†cter */
	/* y convirtiendo en may£sculas */
	for(i=0; i<3; i++, l++) sec[i]=mayuscula(*l);
	sec[i]='\0';

	/* mira si la marca de secci¢n es v†lida */
	/* si es v†lida, 'i' ser† un n£mero entre 0 y N_SECCS-1 */
	/* y si no es v†lida ser† N_SECCS */
	for(i=0; i<N_SECCS; i++) {
		if(!strcmp(sec,id_secc[i])) break;
	}
	seccion=i;
	if(seccion==N_SECCS) imp_error(_E_SCCI,num_lin);

	/* mira si esa secci¢n ya fue encontrada */
	/* en caso de que sea una secci¢n m£ltiple no da ese error */
	if((seccion!=PRO) && (seccion!=MSG) && (secc_encontrada[seccion]))
	  imp_error(_E_SCRP,num_lin);

	/* crea un nombre de fichero temporal para esa secci¢n */
	/* que ser† XXX$SINT.TMP, donde XXX ser† el identificador de */
	/* la secci¢n, y lo abre si no est† abierto ya */
	if(!tf_abierto[seccion]) {
		strcpy(tf_nombre[seccion],id_secc[seccion]);
		strcat(tf_nombre[seccion],"$SINT.TMP");
		if((tf_file[seccion]=fopen(tf_nombre[seccion],"w+t"))==NULL)
		  imp_error(_E_FTMP,0);
		tf_abierto[seccion]=TRUE;
	}
	/* activa el indicador de secci¢n encontrada */
	/* en caso de secci¢n m£ltiple el indicador se usa para */
	/* que no se abra de nuevo el fichero temporal */
	secc_encontrada[seccion]=TRUE;

	if((seccion!=OBJ) && (seccion!=PRO)) marca_seccion(seccion,MARCA_1);
	else marca_seccion(seccion+1,MARCA_1);

	/* si se trata de una secci¢n m£ltiple, coge su n£mero y */
	/* lo guarda antes del c¢digo de dicha secci¢n con el */
	/* siguiente formato:   'num_lin:\nnn...' d¢nde '\' es el */
	/* indicador de inicio de secci¢n m£ltiple, 'num_lin' es */
	/* el n£mero de l°nea dentro del fichero de entrada de la */
	/* marca de inicio de secci¢n y 'nnn...' es la cadena de */
	/* caracteres que precede a la marca de inicio de secci¢n */
	/* en el fichero de entrada */
	if((seccion==PRO) || (seccion==MSG)) {
		l=hasta_espacio(l);
		l=salta_espacios(l);
		ultoa(num_lin,lnum,10);
		strcat(lnum,":\\");
		if(fwrite(lnum,strlen(lnum),1,tf_file[seccion])!=1)
		  imp_error(_E_ETMP,0);
		if(fwrite(l,strlen(l),1,tf_file[seccion])!=1)
		  imp_error(_E_ETMP,0);
	}

	/* lee lineas desde el fichero de entrada y las escribe en el */
	/* fichero temporal si estas no son comentarios ni definiciones */
	/* de constantes (estas £ltimas tratar† de almacenarlas) */
	/* en cuanto encuentra una l°nea que es marca de secci¢n finaliza */
	/* adem†s a§ade el n£mero de l°nea actual dentro del fichero */
	/* de entrada a cada l°nea que escribe en el fichero temporal */
	do {
		lee_linea(f_ent,linea,&num_lin);
		/* si encuentra fin de fichero da error */
		if(feof(f_ent)) imp_error(_E_EOFI,num_lin);
		if((i=tipo_lin(linea))==L_NORM) {
			ultoa(num_lin,lnum,10);
			strcat(lnum,":");
			if(fwrite(lnum,strlen(lnum),1,tf_file[seccion])!=1)
			  imp_error(_E_ETMP,0);
			if(fwrite(linea,strlen(linea),1,tf_file[seccion])!=1)
			  imp_error(_E_ETMP,0);
		}
		/* si es una definici¢n de constante */
		if(i==L_CONST) {
			l=linea;
			/* salta espacios iniciales y deja puntero */
			/* al inicio del nombre de la constante */
			l=salta_espacios(l);
			l+=2;
			l=salta_espacios(l);

			/* intenta almacenar la constante en la tabla */
			err_const=mete_const(l,num_lin);
			if(err_const) imp_error(err_const,num_lin);
		}
	} while(i!=L_MARCA);
	/* mira si encuentra marca de fin de secci¢n */
	l=linea;
	l=salta_espacios(l);
	l++;
	for(i=0; i<3; i++, l++) sec[i]=mayuscula(*l);
	sec[i]='\0';
	if(strcmp(sec,"END")) imp_error(_E_MEND,num_lin);
} while(!feof(f_ent));

for(i=0; i<N_SECCS; i++) if(!secc_encontrada[i]) {
	aviso(0,"secci¢n de %s no encontrada",nomb_secc[i]);
	err_sec=TRUE;
}
if(err_sec) imp_error(_E_NOSC,0);

/* coloca puntero al inicio de los ficheros temporales */
for(i=0; i<N_SECCS; i++) rewind(tf_file[i]);

}

/****************************************************************************
	ANALIZA_ARGS: analiza los argumentos de la l°nea de llamada al
	  programa.
	  Entrada:      'argc' n£mero de argumentos en la l°nea de llamada
			'argv' matriz de punteros a los argumentos, el primero
			ser† siempre el nombre del programa
	  Salida:       1 si error, 0 si no
		      variables globales:-
			'nf_ent', 'nf_sal' nombres de ficheros de entrada y
			salida e indicadores corrrespondientes actualizados
****************************************************************************/
int analiza_args(int argc, char *argv[])
{
int ppar=1;
char par[129];

*nf_ent='\0';
*nf_sal='\0';

while(ppar<argc) {
	/* comprueba si empieza por '/' o '-' */
	if((*argv[ppar]=='/') || (*argv[ppar]=='-')) {
		/* copia argumento sin car†cter inicial y pasa a may£sculas */
		strcpy(par,argv[ppar]+1);
		strupr(par);

		/* mira si es alguno de los par†metros v†lidos */
		if(!strcmp(par,"E")) cs_eds=TRUE;
	}
	else {
		if(*nf_ent=='\0') {
			strcpy(nf_ent,argv[ppar]);
			strupr(nf_ent);
		}
		else if(*nf_sal=='\0') {
			strcpy(nf_sal,argv[ppar]);
			strupr(nf_sal);
		}
	}

	ppar++;
}

if(*nf_sal=='\0') construye_nfsal();

return(0);
}

/****************************************************************************
	CONSTRUYE_NFSAL: construye nombre de fichero de salida a partir
	  del de entrada.
	  Entrada:    variables globales:-
			'nfent' nombre de fichero de entrada
	  Salida:     variables globales:-
			'nfsal' nombre de fichero de salida
****************************************************************************/
void construye_nfsal(void)
{
char drive_n[MAXDRIVE], dir_n[MAXDIR], fname_n[MAXFILE], ext_n[MAXEXT];

if(*nf_ent=='\0') {
	*nf_sal='\0';
	return;
}

fnsplit(nf_ent,drive_n,dir_n,fname_n,ext_n);
strcpy(nf_sal,drive_n);
strcat(nf_sal,dir_n);
strcat(nf_sal,fname_n);
strcat(nf_sal,".DAT");
strupr(nf_sal);

}

/****************************************************************************
	LEE_LINEA: lee una linea de un fichero de entrada.
	  Entrada:      'f_ent' puntero al fichero de entrada
			'lin' buffer donde se guardar† la linea le°da la
			cual ser† de una longitud igual o mayor a LONG_LIN
			'nlin' puntero a n£mero de linea actual en fichero
			de entrada
	  Salida:       puntero a 'lin', adem†s	'nlin' actualizada
****************************************************************************/
char *lee_linea(FILE *f_ent, char *lin, unsigned long *nlin)
{

/* lee una l°nea del fichero de entrada y comprueba si hubo error */
/* en cuyo caso finaliza el proceso */
if((fgets(lin,LONG_LIN,f_ent))==NULL)
  if(ferror(f_ent)) imp_error(_E_LFIN,*nlin);

/* incrementa n£mero de l°nea */
(*nlin)++;

return(lin);
}

/****************************************************************************
	TIPO_LIN: devuelve el tipo de linea.
	  Entrada:	'lin' puntero a la linea
	  Salida:	tipo de linea (NOTA: blanco=espacio o tabulaci¢n)
			L_COMENT si el primer car†cter no blanco es indicador
			de comentario
			L_CONST si los dos primeros car†cteres no blancos
			son indicadores de marca de secci¢n
			L_MARCA si el primer car†cter no blanco es indicador
			de marca de secci¢n (y no es L_CONST)
			L_NULA si la linea s¢lo contiene blancos
			L_NORM en cualquier otro caso
****************************************************************************/
int tipo_lin(char *lin)
{
char *aux_lin;

/* apunta al inicio de la linea a comprobar y salta blancos iniciales */
aux_lin=lin;
aux_lin=salta_espacios(aux_lin);
switch(*aux_lin) {
	case CHR_COMENT :
		return(L_COMENT);
	case MARCA_S :
		if(*(aux_lin+1)==MARCA_S) return(L_CONST);
		return(L_MARCA);
	case '\n' :
		return(L_NULA);
	case '\0' :
		return(L_NULA);

}

return(L_NORM);
}

/****************************************************************************
	CHEQUEA_CONX: chequea la validez de la tabla de conexiones buscando
	  alguna conexi¢n a una localidad no v†lida.
	  Entrada:      'num_loc' n£mero de localidades compiladas
			'conx' puntero a inicio tabla de conexiones
			'desp_conx' puntero a tabla de desplazamientos
			de las conexiones
			'err_conx' puntero a variable d¢nde dejar n£mero de
			localidad
			'num_conx' puntero a variable d¢nde dejar n£mero de
			orden de conexi¢n
	  Salida:       _E_NERR si no hay errores
			_E_CLOC si los hubo
			'err_conx' el n£mero de localidad en la que se
			detect¢ el error, 'num_conx' el n£mero de orden
			de la conexi¢n que produjo error
****************************************************************************/
COD_ERR chequea_conx(BYTE num_loc, BYTE *conx, unsigned *desp_conx,
  BYTE *err_conx, int *num_conx)
{
BYTE *pc;
int i, j;

for(i=0; i<(int)num_loc; i++) {
	pc=conx+desp_conx[i];
	j=0;
	while(*pc) {
		/* salta el verbo de movimiento */
		pc++;
		/* n£mero de orden de conexi¢n */
		j++;
		/* si detecta un conexi¢n a una localidad no definida */
		/* almacena su n£mero de localidad y sale indicando error */
		if((*pc++>=num_loc)) {
			*err_conx=(BYTE)i;
			*num_conx=j;
			return(_E_CLOC);
		}
	}
}

return(_E_NERR);
}

/****************************************************************************
	CODIFICA: codifica/decodifica una tabla de secci¢n.
	  Entrada:      'mem' puntero a la tabla a codificar/decodificar
			'bytes_mem' tama§o de la tabla
****************************************************************************/
void codifica(BYTE *mem, unsigned bytes_mem)
{
BYTE *p, *ult_p;

p=mem;
ult_p=p+bytes_mem;

for(; p<ult_p; p++) *p=CODIGO(*p);

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
	mov bh,0                // supone p†gina 0
	mov dh,25               // DH = fila del cursor
	mov dl,0                // DL = columna del cursor
	int 10h
}

}

/****************************************************************************
	MARCA_SECCION: coloca una marca al lado del nombre de la secci¢n
	  que est† siendo compilada.
	  Entrada:      'secc' n£mero de l°nea relativo del nombre de la
			secci¢n
			'marca' marca a imprimir
****************************************************************************/
void marca_seccion(int secc, char *marca)
{

v_pon_cursor(&ccomp.v,FIL_SECCS+secc,COL_SECCS+25);
v_color(&ccomp.v,cfg.color_cs);
v_impcad(&ccomp.v,marca,V_NORELLENA);
v_color(&ccomp.v,ccomp.v.clr_princ);

}

/****************************************************************************
	COMPILA_SECCS: compila las secciones de la base de datos y las va
	  almacenando en el fichero de salida.
	  Entrada:      'memoria' tama§o del buffer reservado para compilar
****************************************************************************/
void compila_seccs(size_t memoria)
{
FILE *f_sal;
int i, num_conx;
STC_ERR err;
BYTE err_conx, n_tmsg, num_msg;
unsigned bytes_msg, tot_nt_msg=0, tot_msg=0;

/* abrir fichero de salida */
if((f_sal=fopen(nf_sal,"wb"))==NULL) imp_error(_E_AFOU,0);

/* cadena de reconocimiento */
strcpy(cab.srecon,SRECON);

/* escribe cabecera 'falsa' que luego ser† sobreescrita */
if(fwrite(&cab,sizeof(CAB_SINTAC),1,f_sal)!=1) imp_error(_E_EFOU,0);

/* VOCABULARIO */
marca_seccion(0,MARCA_2);
err=compila_voc(tf_file[VOC],vocabulario,&cab.pal_voc);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[VOC],cab.pal_voc,
  sizeof(struct palabra)*cab.pal_voc);
v_pon_cursor(&ccomp.v,FIL_SECCS,COL_SECCS);
v_impcad(&ccomp.v,buff,V_RELLENA);
/* elimina fichero temporal de vocabulario */
fclose(tf_file[VOC]);
tf_abierto[VOC]=FALSE;
remove(tf_nombre[VOC]);
cab.fpos_voc=ftell(f_sal);
if(fwrite(vocabulario,sizeof(struct palabra),(size_t)cab.pal_voc,
  f_sal)!=(size_t)cab.pal_voc) imp_error(_E_EFOU,0);

/* MENSAJES DEL SISTEMA */
marca_seccion(1,MARCA_2);
err=compila_msy(tf_file[MSY],buff_compil1,memoria,(MAX_MSY-1),tab_desp_msy,
  &cab.num_msy,&cab.bytes_msy);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[MSY],cab.num_msy,cab.bytes_msy);
v_pon_cursor(&ccomp.v,FIL_SECCS+1,COL_SECCS);
v_impcad(&ccomp.v,buff,V_RELLENA);
/* comprueba si el n£mero de mensajes compilados es el m°nimo requerido */
/* por el sistema, si no da un aviso */
if(cab.num_msy<NUM_MSY)
  aviso(0,"hay menos mensajes del sistema que los requeridos");
/* elimina fichero temporal de mensajes del sistema */
fclose(tf_file[MSY]);
tf_abierto[MSY]=FALSE;
remove(tf_nombre[MSY]);
cab.fpos_msy=ftell(f_sal);
if(fwrite(tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY,f_sal)!=MAX_MSY)
  imp_error(_E_EFOU,0);
codifica((BYTE *)buff_compil1,cab.bytes_msy);
if(fwrite(buff_compil1,sizeof(char),(size_t)cab.bytes_msy,f_sal)!=
  (size_t)cab.bytes_msy) imp_error(_E_EFOU,0);

/* MENSAJES */
marca_seccion(2,MARCA_2);
for(i=0; i<MAX_TMSG; i++) {
	cab.fpos_msg[i]=(fpos_t)0;
	cab.num_msg[i]=0;
	cab.bytes_msg[i]=0;
}
while(1) {
	err=compila_msg(tf_file[MSG],buff_compil1,memoria,(MAX_MSG-1),
	  (MAX_TMSG-1),tab_desp_msg,&n_tmsg,&num_msg,&bytes_msg);

	cab.num_msg[n_tmsg]=num_msg;
	cab.bytes_msg[n_tmsg]=bytes_msg;

	if(err.codigo && (err.codigo!=_E_MXXX)) imp_error(err.codigo,err.linea);

	/* si ha llegado al final del fichero, sale */
	if(err.codigo!=_E_MXXX) break;

	tot_nt_msg++;
	tot_msg+=num_msg;
	sprintf(buff," %-20s : %5u   %5u mens.",nomb_secc[MSG],tot_nt_msg,
	  tot_msg);
	v_pon_cursor(&ccomp.v,FIL_SECCS+2,COL_SECCS);
	v_impcad(&ccomp.v,buff,V_RELLENA);

	cab.fpos_msg[n_tmsg]=ftell(f_sal);
	if(fwrite(tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG,
	  f_sal)!=MAX_MSG) imp_error(_E_EFOU,0);
	codifica((BYTE *)buff_compil1,bytes_msg);
	if(fwrite(buff_compil1,sizeof(char),(size_t)bytes_msg,
	  f_sal)!=(size_t)bytes_msg) imp_error(_E_EFOU,0);
}
/* elimina fichero temporal de mensajes */
fclose(tf_file[MSG]);
tf_abierto[MSG]=FALSE;
remove(tf_nombre[MSG]);

/* LOCALIDADES */
marca_seccion(3,MARCA_2);
err=compila_loc(tf_file[LOC],buff_compil1,memoria,(MAX_LOC-1),tab_desp_loc,
  buff_compil2,memoria/8,tab_desp_conx,vocabulario,cab.pal_voc,cab.v_mov,
  &cab.num_loc,&cab.bytes_loc,&cab.bytes_conx);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[LOC],cab.num_loc,cab.bytes_loc);
v_pon_cursor(&ccomp.v,FIL_SECCS+3,COL_SECCS);
v_impcad(&ccomp.v,buff,V_RELLENA);
sprintf(buff,Str_Compil,Secc_Conx,(cab.bytes_conx-cab.num_loc)/2,
  cab.bytes_conx);
v_pon_cursor(&ccomp.v,FIL_SECCS+4,COL_SECCS);
v_impcad(&ccomp.v,buff,V_RELLENA);
/* comprueba validez de la tabla de conexiones */
err.codigo=chequea_conx(cab.num_loc,buff_compil2,tab_desp_conx,&err_conx,
  &num_conx);
if(err.codigo) {
	if(cs_eds==FALSE) {
		sprintf(buff,Msg_Err,men_error(err.codigo));
		strcpy(vmsg,buff);
		sprintf(buff,", localidad %u, conexi¢n %i",err_conx,
		  num_conx);
		strcat(vmsg,buff);
		c_mete_en_lista((STC_ELEM_LISTA *)(lsterr->info),vmsg);
	}
	else if(ferr!=NULL) fprintf(ferr,"0: %s, localidad %u, conexi¢n %i",
	  men_error(err.codigo),err_conx,num_conx);

	fin_prg(1);
}
/* elimina fichero temporal de localidades */
fclose(tf_file[LOC]);
tf_abierto[LOC]=FALSE;
remove(tf_nombre[LOC]);
cab.fpos_loc=ftell(f_sal);
if(fwrite(tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC,f_sal)!=MAX_LOC)
  imp_error(_E_EFOU,0);
codifica((BYTE *)buff_compil1,cab.bytes_loc);
if(fwrite(buff_compil1,sizeof(char),(size_t)cab.bytes_loc,f_sal)!=
  (size_t)cab.bytes_loc) imp_error(_E_EFOU,0);
if(fwrite(tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC,f_sal)!=MAX_LOC)
  imp_error(_E_EFOU,0);
codifica(buff_compil2,cab.bytes_conx);
if(fwrite(buff_compil2,sizeof(BYTE),(size_t)cab.bytes_conx,f_sal)!=
  (size_t)cab.bytes_conx) imp_error(_E_EFOU,0);

/* OBJETOS */
marca_seccion(5,MARCA_2);
err=compila_obj(tf_file[OBJ],buff_compil1,memoria/2,(MAX_OBJ-1),tab_desp_obj,
  vocabulario,cab.pal_voc,cab.num_loc,&cab.num_obj,&cab.bytes_obj);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[OBJ],cab.num_obj,cab.bytes_obj);
v_pon_cursor(&ccomp.v,FIL_SECCS+5,COL_SECCS);
v_impcad(&ccomp.v,buff,V_RELLENA);
/* elimina fichero temporal de objetos */
fclose(tf_file[OBJ]);
tf_abierto[OBJ]=FALSE;
remove(tf_nombre[OBJ]);
cab.fpos_obj=ftell(f_sal);
if(fwrite(tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ,f_sal)!=MAX_OBJ)
  imp_error(_E_EFOU,0);
codifica((BYTE *)buff_compil1,cab.bytes_obj);
if(fwrite(buff_compil1,sizeof(char),(size_t)cab.bytes_obj,f_sal)!=
  (size_t)cab.bytes_obj) imp_error(_E_EFOU,0);

/* PROCESOS */
marca_seccion(6,MARCA_2);
err=compila_pro(tf_file[PRO],buff_compil2,memoria,MAX_PRO-1,tab_desp_pro,
  vocabulario,cab.pal_voc,cab.num_loc,cab.num_msy,cab.num_obj,cab.n_conv,
  &cab.num_pro,&cab.bytes_pro);
if(err.codigo) imp_error(err.codigo,err.linea);
sprintf(buff,Str_Compil,nomb_secc[PRO],cab.num_pro,cab.bytes_pro);
v_pon_cursor(&ccomp.v,FIL_SECCS+6,COL_SECCS);
v_impcad(&ccomp.v,buff,V_RELLENA);
/* elimina fichero temporal de procesos */
fclose(tf_file[PRO]);
tf_abierto[PRO]=FALSE;
remove(tf_nombre[PRO]);
cab.fpos_pro=ftell(f_sal);
if(fwrite(tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO,f_sal)!=MAX_PRO)
  imp_error(_E_EFOU,0);
codifica(buff_compil2,cab.bytes_pro);
if(fwrite(buff_compil2,sizeof(BYTE),(size_t)cab.bytes_pro,f_sal)!=
  (size_t)cab.bytes_pro) imp_error(_E_EFOU,0);

/* escribe cabecera con datos reales */
rewind(f_sal);
if(fwrite(&cab,sizeof(CAB_SINTAC),1,f_sal)!=1) imp_error(_E_EFOU,0);

/* cierra el fichero de salida */
fclose(f_sal);

}
