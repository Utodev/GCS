/****************************************************************************
			 INTERPRETE-DEBUGGER SINTAC
			    (c)1995 JSJ Soft Ltd.

	NOTA: mediante la constante DEBUGGER se controla si se genera
		  el código del intérprete o del intérprete-debugger, 
	Con: DEBUGGER=0 se genera el código del intérprete-debugger
	     DEBUGGER=1 se genera el código del intérprete
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>
#include <time.h>
#include <dir.h>
#include <dos.h>
#include <bios.h>
#include "version.h"
#include "sintac.h"
#include "graf.h"
#include "vv.h"
#include "condacto.h"
#include "ds.h"

/*** Variables externas ***/
extern BYTE loc_obj[MAX_OBJ];   /* tabla de localidades act. de objetos */
extern STC_VV w[N_VENT];        /* tabla para guardar parámetros de ventanas */
extern int ptrp;                /* puntero de pila */
extern STC_BANCORAM ram[BANCOS_RAM];    /* para RAMSAVE y RAMLOAD */
extern DAAD_CONDACTO cd[];       /* tabla de función-tipo condactos */

/*** Variables globales ***/
#if DEBUGGER==1 
#include "tabcond.h"            /* tabla de nombre-tipo de condactos */
BOOLEAN entorno=FALSE;          /* indica si se ejecuta desde entorno */
STC_VV vv_jsj;                  /* ventana de presentación */
STC_VV vv_deb;                  /* ventana de debugger */
BOOLEAN debugg=TRUE;            /* TRUE si paso a paso activado */
BOOLEAN pra_lin=FALSE;          /* TRUE si en primera línea de una entrada */
unsigned char far *img_debug;   /* puntero buffer para fondo ventana debug. */
#endif


BYTE *ddb; 
CAB_DAAD cab;

/* nombre de fichero de base de datos */
char nf_base_datos[MAXPATH];

/* variables para Vocabulario */
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */
STC_VV vv_err;                  /* ventana para mensajes de error */
BYTE flag[FLAGS];                 /* variables del sistema (8 bits) */
BYTE pro_act;                   /* número de proceso actual */
WORD ptr_proc;  	            /* puntero a la tabla del proceso actual */
WORD ptr_entry;					/* puntero a la entrada actual */
WORD ptr_condact;				/* puntero al condacto actual */
unsigned sgte_ent;              /* desplazamiento de sgte. entrada */
BOOLEAN resp_act;               /* RESP (=1) o NORESP (=0) */
BOOLEAN nueva_ent;              /* indica que no debe ajustar ptr_proc para */
				/* saltar a siguiente entrada */

int ruptura;                    /* indicador de ruptura (BREAK) */

int modovideo;                  /* modo vídeo, 0=640x480x16, 1=320x200x256 */
BYTE num_cols, num_rows;
int columnastxt;                /* columnas de texto */

/*** Programa principal ***/


BYTE getDDBByte(unsigned int address)
{
	return *(ddb+address);
}

WORD getDDBWord(unsigned int address)
{
	return *(ddb+address) + 256 * *(ddb + address + 1);
}

void main(int argc, char *argv[])
{

	#if DEBUGGER==1
	BYTE lin_deb=WDEB_FIL;
	BYTE max_lindeb, *pro_d;
	unsigned long tam_img_debug;
	#endif
	BYTE i, indir, msc_indir, ncondacto, par[8], npar;
	BOOLEAN res_pro;

	/* establece modo de pantalla */
	modo_video(0);

	/* comprueba modo de pantalla */
	if(g_coge_modovideo()!=G_MV_G3C16) {
		printf("\nEste programa requiere VGA.\n");
		exit(1);
	}

	if(argc<2) m_err(7,"Falta nombre de fichero",1);

	#if DEBUGGER==1
	/* detrás del nombre del fichero espera /lxx o /Lxx (o -lxx o -Lxx) */
	if(argc==3) {
		/* si introdujo /l o /L (o -l o -L) recoge los dos siguientes */
		/* dígitos y calcula línea de la ventana del debugger */
		if(((argv[2][0]=='/') || (argv[2][0]=='-')) && ((argv[2][1]=='l') ||
		(argv[2][1]=='L'))) {
			lin_deb=(BYTE)(((argv[2][2]-'0')*10)+(argv[2][3]-'0'));
			max_lindeb=(BYTE)(MODO0_FIL-WDEB_ALTO);
			if(lin_deb>max_lindeb) lin_deb=max_lindeb;
		}
	}
	/* como tercer parámetro espera /e o /E (o -E o -e) */
	if(argc==4) {
		if(((argv[3][0]=='/') || (argv[3][0]=='-')) && ((argv[3][1]=='e') ||
		(argv[3][1]=='E'))) entorno=TRUE;
	}

	/* crea la ventana del debugger */
	vv_crea(lin_deb,WDEB_COL,WDEB_ANCHO,WDEB_ALTO,WDEB_COLORF,WDEB_COLOR,NO_BORDE,
	&vv_deb);

	/* reserva buffer para guardar fondo */
	tam_img_debug=blq_tam(0,0,(vv_deb.lx*8)-1,(vv_deb.ly*vv_deb.chralt)-1);
	img_debug=farmalloc(tam_img_debug);

	#endif

	/* instala 'handler' de errores críticos */
	harderr(int24_hnd);

	cls();

	ddb = (BYTE*) malloc(65535);
	/* carga base de datos e inicializa variables */
	carga_db(argv[1]);

	/* inicializa */
	inic();

	/* guarda indicador de ruptura y lo desactiva */
	ruptura=getcbrk();
	setcbrk(0);

	#if DEBUGGER==1
	/* presentación */
	cls();
	if(entorno==FALSE) {
		rg_puntero(RG_OCULTA);
		vv_crea(WJSJ_FIL,WJSJ_COL,WJSJ_ANCHO,WJSJ_ALTO,1,15,BORDE_3,&vv_jsj);
		/* centra ventana */
		vv_jsj.vx=(BYTE)((MODO0_COL-vv_jsj.lx)/2);
		vv_jsj.vxi=(BYTE)(vv_jsj.vx+1);
		vv_cls(&vv_jsj);
		vv_imps("\n  Intérprete-Debugger versión "VERSION"\n"
			"\n  "COPYRIGHT"\n\n\n"
			"\n           Pulsa una tecla",&vv_jsj);
		vv_lee_tecla();
		cls();
		rg_puntero(RG_MUESTRA);
	}
	#endif

	/* inicializa puntero a proceso actual */
	ptr_proc = cab.pro_pos;

	/* y puntero a la entrada actual */
	ptr_entry = getDDBWord(ptr_proc);

	/* bucle principal de ejecución de procesos*/
	while(1) 
	{
	
		/* actualiza flags de estado de ratón */
		actualiza_raton();

		/* si no es fin de proceso */
		if (getDDBByte(ptr_entry)!=END_OF_PROCESS_MARK) 
		{
			/* si 'res_pro' es FALSE no debe ejecutar entrada */
			res_pro=!resp_act || (resp_act && ((getDDBByte(ptr_entry)==NO_PAL) ||  (getDDBByte(ptr_entry)==getflag(FVERB))) && ((getDDBByte(ptr_entry+1)==NO_PAL) || (getDDBByte(ptr_entry+1)==getflag(FNOUN))));

			/* salta verbo-nombre */
			ptr_entry+=2;

			ptr_condact = getDDBWord(ptr_entry);

			#if DEBUGGER==1
			/* indica que es primera línea de entrada */
			pra_lin=TRUE;
			#endif

		}
		/* si fin de proceso */
		else
		{
			res_pro=done();
			ptr_proc++;     /* ajustamos ptr_proc */

			#if DEBUGGER==1
			pra_lin=FALSE;  /* no es primera línea de entrada */
			#endif
		}

		/* indica que deberá ajustar ptr_proc a siguiente */
		/* a menos que algún condacto cambie esta variable */
		nueva_ent=TRUE;

		/* si 'res_pro' es TRUE y no fin de entrada ejecuta esta entrada */
		/* si no, salta a la siguiente */
		while(res_pro && (getDDBByte(ptr_condact)!=END_OF_CONDACTS_MARK)) 
		{
			#if DEBUGGER==1
			/* guarda dirección de condacto en curso */
			pro_d=ptr_condact;
			#endif

			/* tomar condacto y detectar indirección */
			ncondacto= getDDBByte(ptr_condact);
			if (ncondacto & 0x80)
			{
				indir = 1;
				ncondacto &= 0x7F;
			} else indir = 0;
			npar=cd[ncondacto].npar;

			/* tomar parámetros y aplicar indirección si procede */
			for(i=0; i<npar; i++) 
			{
				par[i]=getDDBWord(ptr_condact+i+1);
			}
			if (indir) par[0] = getflag(par[0]);

			#if DEBUGGER==1
			/* si está activado el paso a paso */
			if(debugg==TRUE) debugger(indir,npar,pro_d);
			/* sino, activa paso a paso */
			else if(bioskey(1)==F10) {
				bioskey(1);
				debugg=TRUE;
				/* indicamos que no es primera línea de entrada para */
				/* que imp_condacto() no imprima el verbo-nombre ya */
				/* que el puntero al condacto no está ajustado */
				pra_lin=FALSE;
			}
			#endif

			/* ejecuta condacto según número de parámetros */
			#pragma warn -pro
			switch(npar) 
			{
				case 0 :
					res_pro=cd[ncondacto].cond();
					ptr_condact++;
					break;
				case 1 :
					res_pro=cd[ncondacto].cond(par[0]);
					ptr_condact+=2;
					break;
				case 2 :
					res_pro=cd[ncondacto].cond(par[0],par[1]);
					ptr_condact+=3;
					break;
			}
			#pragma warn +pro

			if(res_pro) nueva_ent=TRUE;
		}

		/* si fin entrada, pasa a la siguiente */
		if (getDDBByte(ptr_condact)!=END_OF_CONDACTS_MARK) ptr_condact++;
		else if(nueva_ent==TRUE) 
		{
			ptr_entry+=2;
			nueva_ent=FALSE;
		}
	}  /* bucle condactos */
}	/* bucle de entradas */



#pragma warn -par
/****************************************************************************
	INT24_HND: rutina de manejo de errores críticos de hardware.
****************************************************************************/
int int24_hnd(int errval, int ax, int bp, int si)
{

hardretn(2);

return(2);
}
#pragma warn +par

#if DEBUGGER==1
/****************************************************************************
	SACA_PAL: devuelve el número de la primera entrada en el vocabulario
	  que se corresponda con el número y tipo de palabra dado.
	  Entrada:      'num_pal' número de la palabra a buscar
			'tipo_pal' tipo de la palabra a buscar
	  Salida:       número dentro de la tabla de vocabulario o
			(NUM_PAL+1) si no se encontró
****************************************************************************/
int saca_pal(BYTE num_pal, BYTE tipo_pal)
{
int i;

for(i=0; i<cab.pal_voc; i++) {
	if((vocabulario[i].num==num_pal) && (vocabulario[i].tipo==tipo_pal))
	  return(i);
}

return(NUM_PAL+1);
}

/****************************************************************************
	IMP_CONDACTO: imprime condacto en curso en la ventana de debug.
	  Entrada:      'indir' indicadores de indirección
			'npar' número de parámetros
			'pro_d' dirección del condacto en curso
		      variables globales:-
			'pra_lin' TRUE si es 1era línea de entrada
			'ptr_proc' puntero a byte de condacto + parámetros
****************************************************************************/
void imp_condacto(BYTE indir, BYTE npar, BYTE *pro_d)
{
BYTE i, *pcp, msc_indir;
int j, dirrel;
unsigned dir;
char lin_cond[LNG_LINDEB+1], par[7];
char *Pal_Nula="-     ";

/* si es la primera línea de la entrada pone el puntero apuntando al */
/* campo verbo, si no apunta al condacto */
if(pra_lin==TRUE) dir=(unsigned)(pro_d-tab_pro-4);
else dir=(unsigned)(pro_d-tab_pro);

/* imprime dirección del condacto */
sprintf(lin_cond,"%5u: ",dir);
vv_imps(lin_cond,&vv_deb);

if(pra_lin==TRUE) {
	pcp=ptr_proc-4;                 /* apunta a verbo-nombre */
	if(indir) pcp-=2;               /* ajuste por indirección */
	if(*pcp==NO_PAL) sprintf(lin_cond,"%s  ",Pal_Nula);
	else {
		j=saca_pal(*pcp,_VERB);
		/* si no es verbo, quizá sea nombre */
		if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
		sprintf(lin_cond,"%s  ",vocabulario[j].p);
	}
	vv_imps(lin_cond,&vv_deb);
	pcp++;

	if(*pcp==NO_PAL) sprintf(lin_cond,"%s",Pal_Nula);
	else sprintf(lin_cond,"%s",vocabulario[saca_pal(*pcp,_NOMB)].p);
	vv_imps(lin_cond,&vv_deb);
}

/* siguiente línea de ventana */
vv_deb.cvx=1*8;
vv_deb.cvy+=vv_deb.chralt;

/* imprime condacto */
sprintf(lin_cond,"%s ",condacto[*ptr_proc].cnd);
vv_imps(lin_cond,&vv_deb);

/* imprime parámetros según tipo de condacto */
switch(condacto[*ptr_proc].tipo) {
	case 0 :
		sprintf(lin_cond," ");
		break;
	case 11 :
		dirrel=(*(ptr_proc+2) << 8) | *(ptr_proc+1);
		if(pra_lin==TRUE) dirrel+=4;
		sprintf(lin_cond,"%5i",dirrel);
		break;
	case 13 :
		sprintf(lin_cond,"%s",
		  vocabulario[saca_pal(*(ptr_proc+1),_ADJT)].p);
		break;
	case 14 :
		sprintf(lin_cond,"%s",
		  vocabulario[saca_pal(*(ptr_proc+1),_NOMB)].p);
		break;
	case 16 :
		if(*(ptr_proc+1)==NO_PAL) sprintf(lin_cond,"%s ",Pal_Nula);
		else {
			j=saca_pal(*(ptr_proc+1),_VERB);
			/* si no es vebro, quizá sea nombre convertible */
			if(j==(NUM_PAL+1)) j=saca_pal(*(ptr_proc+1),_NOMB);
			sprintf(lin_cond,"%s ",vocabulario[j].p);
		}
		vv_imps(lin_cond,&vv_deb);
		if(*(ptr_proc+2)==NO_PAL) sprintf(lin_cond,"%s",Pal_Nula);
		else sprintf(lin_cond,"%s",vocabulario[saca_pal(*(ptr_proc+2),
		  _NOMB)].p);
		break;
	default :
		*lin_cond='\0';
		msc_indir=0x01;
		for(i=0; i<npar; i++) {
			if(indir & msc_indir) sprintf(par,"[%u] ",
			  *(ptr_proc+i+1));
			else sprintf(par,"%u ",*(ptr_proc+i+1));
			strcat(lin_cond,par);
			msc_indir <<= 1;
		}
		break;
}

/* imprime parámetros */
vv_imps(lin_cond,&vv_deb);

/* si estaba en primera línea de entrada indica que ya no está */
pra_lin=FALSE;

}

/****************************************************************************
	INP_DEB: rutina de introducción por teclado de números para
	  debugger.
	  Salida:       número introducido en el rango 0-255 (BYTE).
****************************************************************************/
BYTE inp_deb(void)
{
unsigned k;
char numero[4];
int i, antcwx, anch;

/* guarda antigua posición cursor */
antcwx=vv_deb.cvx;

/* repite hasta que sea un número válido */
do {
	i=0;
	/* restaura posición cursor */
	vv_deb.cvx=antcwx;

	/* repite mientras no introduzca 3 dígitos o no pulse RETURN */
	do {
		anch=vv_impc(CUR_DEBUG,&vv_deb);
		vv_deb.cvx-=anch;
		k=vv_lee_tecla();
		if((k>='0') && (k<='9')) {
			numero[i]=(char)k;
			i++;
			vv_impc((char)k,&vv_deb);
		}
	} while((i<3) && (k!=RETURN));
	numero[i]='\0';

	/* pasa cadena ASCII a número */
	i=atoi(numero);
} while((i<0) || (i>255));

return((BYTE)i);
}

/****************************************************************************
	imp_flag: imprime en la línea del debugger la variable 
	  actuales.
	  Entrada:      'variable' número de variable a imprimir
****************************************************************************/
void imp_flag(BYTE variable)
{
char lin_deb[LNG_LINDEB+1];
int palabra;

/* imprime variable */
vv_deb.cvx=9*8;
vv_deb.cvy=0;
vv_imps("Var ",&vv_deb);
sprintf(lin_deb,"%3u=\xff",variable);
vv_deb.cvx=13*8;
vv_imps(lin_deb,&vv_deb);
sprintf(lin_deb,"%3u\xff\xff",getflag(variable));
vv_deb.cvx=17*8;
vv_imps(lin_deb,&vv_deb);

/* si es una variable de sentencia lógica imprime además la */
/* palabra del vocabulario correspondiente */
if(variable==FVERB) {
	palabra=saca_pal(getflag(FVERB),_VERB);
	/* puede ser nombre convertible */
	if(palabra==(NUM_PAL+1)) palabra=saca_pal(getflag(FVERB),_NOMB);
}
else if(variable==FNOUN) palabra=saca_pal(getflag(FNOUN),_NOMB);
else if(variable==FADJECT) palabra=saca_pal(getflag(FADJECT),_ADJT);
else if(variable==FNOUN2) palabra=saca_pal(getflag(FNOUN2),_NOMB);
else if(variable==FADJECT2) palabra=saca_pal(getflag(FADJECT2),_ADJT);
else palabra=NUM_PAL+1;

sprintf(lin_deb,"\xff\xff\xff\xff\xff\xff");
vv_deb.cvx=21*8;
vv_imps(lin_deb,&vv_deb);

if(palabra==(NUM_PAL+1)) sprintf(lin_deb,"------");
else sprintf(lin_deb,"%s",vocabulario[palabra].p);

vv_deb.cvx=21*8;
vv_imps(lin_deb,&vv_deb);


}

/****************************************************************************
	GUARDA_DEBUGGER: guarda la zona de pantalla sobreimpresa por la
	  ventana del debugger.
****************************************************************************/
void guarda_debugger(void)
{

if(img_debug!=NULL) {
	rg_puntero(RG_OCULTA);
	blq_coge(vv_deb.vx*8,vv_deb.vy*vv_deb.chralt,
	  ((vv_deb.vx+vv_deb.lx)*8)-1,((vv_deb.vy+vv_deb.ly)*vv_deb.chralt)-1,
	  img_debug);
	rg_puntero(RG_MUESTRA);
}

}

/****************************************************************************
	RECUPERA_DEBUGGER: recupera la zona de pantalla sobreimpresa por la
	  ventana del debugger.
****************************************************************************/
void recupera_debugger(void)
{

if(img_debug!=NULL) {
	rg_puntero(RG_OCULTA);
	blq_pon(vv_deb.vx*8,vv_deb.vy*vv_deb.chralt,img_debug);
	rg_puntero(RG_MUESTRA);
}

}

/****************************************************************************
	IMP_DEBUGGER: imprime la ventana del debugger.
	  Entrada:      'indir' indicadores de indirección
			'npar' número de parámetros
			'pro_d' dirección del condacto en curso
			'variable' número de variable
			que se mostrarán
			'txt_deb' texto a imprimir en la última línea
****************************************************************************/
void imp_debugger(BYTE indir, BYTE npar, BYTE *pro_d, BYTE flag, char *txt_deb)
{
char lin_deb[LNG_LINDEB+1];

/* borra la ventana del debugger e imprime información */
vv_cls(&vv_deb);

vv_deb.cvx=1*8;
vv_deb.cvy=0;
sprintf(lin_deb,"PRO %3u",(unsigned)pro_act);
vv_imps(lin_deb,&vv_deb);
imp_flag(flag);

vv_deb.cvx=1*8;
vv_deb.cvy=1*vv_deb.chralt;
imp_condacto(indir,npar,pro_d);

vv_deb.cvx=1*8;
vv_deb.cvy=3*vv_deb.chralt;
vv_imps(txt_deb,&vv_deb);

}

/****************************************************************************
	DEBUGGER: función principal del debugger.
	  Entrada:      'indir' indicadores de indirección
			'npar' número de parámetros
			'pro_d' dirección del condacto en curso
****************************************************************************/
void debugger(BYTE indir, BYTE npar, BYTE *pro_d)
{
unsigned tecla;
static BYTE flagvalue=0;
BYTE valor;
char *txt_deb1="Var.  Band.  Pant.  Desact.  Salir";
char *txt_deb2="Otra  Modificar  Fin"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";

guarda_debugger();

/* imprime ventana del debugger */
imp_debugger(indir,npar,pro_d,flagvalue,txt_deb1);

tecla=mayuscula((char)vv_lee_tecla());

while(esta_en("VPDS",(char)tecla)) {
	switch(tecla) {
		case 'V' :      /* variables */
			vv_deb.cvx=1*8;
			vv_deb.cvy=3*vv_deb.chralt;
			vv_imps(txt_deb2,&vv_deb);
			do {
				imp_flag(flagvalue);
				tecla=mayuscula((char)vv_lee_tecla());
				switch((BYTE)tecla) {
				case 'M' :              /* modificar */
					vv_deb.cvx=17*8;
					vv_deb.cvy=0;
					valor=inp_deb();
					setflag(flagvalue,valor);
					break;
				case 'O' :              /* otra variable */
					vv_deb.cvx=13*8;
					vv_deb.cvy=0;
					flagvalue=inp_deb();
					break;
				case COD_ARR :
					flagvalue--;
					break;
				case COD_ABJ :
					flagvalue++;
					break;
				}
			} while(tecla!='F');
			break;
			case 'P' :      /* pantalla */
			recupera_debugger();
			vv_lee_tecla();
			imp_debugger(indir,npar,pro_d,flagvalue, txt_deb1);
			break;
		case 'D' :      /* desactivar */
			/* desactiva paso a paso */
			debugg=FALSE;
			recupera_debugger();
			return;
		case 'S' :      /* salir */
			m_err(0,"",2);
			break;
	}
	vv_deb.cvx=1*8;
	vv_deb.cvy=3*vv_deb.chralt;
	vv_imps(txt_deb1,&vv_deb);
	tecla=mayuscula((char)vv_lee_tecla());
}

recupera_debugger();

}

#endif

/****************************************************************************
	MODO_VIDEO: establece el modo de pantalla.
	  Entrada:      'modo' modo a seleccionar, 0 para 640x480x16, 1 para
			320x200x256
	  Salida:     variables globales:-
			'modovideo' último modo seleccionado
			'columnastxt' número de columnas de texto
****************************************************************************/
void modo_video(int modo)
{

rg_puntero(RG_OCULTA);

/* selecciona modo de vídeo y actualiza variables 14 y 15 */
if(modo==0) {
	/* modo de 640x480x16, 80 columnas, 30 filas */
	g_modovideo(G_MV_G3C16);
	modovideo=0;
	columnastxt=MODO0_COL;
	num_cols = MODO0_COL;
	num_rows = MODO0_FIL;
	setflag(FVIDEOMODE, 1);
}
else {
	/* modo de 320x200x256, 40 columnas, 25 filas */
	g_modovideo(G_MV_G1C256);
	modovideo=1;
	columnastxt=MODO1_COL;
	num_cols = MODO1_COL;
	num_rows = MODO1_FIL;
	setflag(FVIDEOMODE, 1);
}

rg_inicializa();
if(getflagbit(FFLAGS, FFMOUSEON)==TRUE) 
 rg_desconecta();

}




/****************************************************************************
	carga_db: carga la base de datos.
	Entrada:      'nombre' nombre de fichero de base de datos
****************************************************************************/
void carga_db(char *nombre)
{
	FILE *fdb;
	unsigned i, bytes_msg;
	long ddb_size;

	if((fdb=fopen(nombre,"rb"))==NULL)
	m_err(1,"Error de apertura fichero de entrada",1);

	/* guarda nombre de fichero de base de datos */
	strcpy(nf_base_datos,nombre);


	/* Obtiene el tamaño del DDB */
	fseek(fdb, 0, SEEK_END);
	ddb_size = ftell(fdb);
	fseek(fdb, 0, SEEK_SET);

	if(ddb_size>=0x10000)
		m_err(1,"DDB file is larger than 64K",1);

	/* lee el DDB completo*/
	frd(fdb,ddb,1,ddb_size);
	/* lee cabecera aparte, por comodidad */
	fseek(fdb, 0, SEEK_SET);
	frd(fdb,&cab,sizeof(CAB_DAAD),1);
	fclose(fdb);

	/* Coloca los datos de los objetos en la tabla que los maneja después. Se mantiene el dato original en el DDB para poder restaurarlos al reiniciar partida */
	for(i=0; i<cab.num_obj; i++) loc_obj[i] = getDDBByte(cab.obj_initially_pos + i);
}

/****************************************************************************
	INIC: inicializa diversas tablas y variables.
	  Entrada:      variables globales:-
			  		'vid' con información de sistema de vídeo
****************************************************************************/
void inic(void)
{
int i;
char *po;


/* inicializa flags */
for(i=0; i<FLAGS; i++)	setflag(i,0);
borra_SL();  /* Pone  SL flags a NO_PAL */

/* inicializa tabla de localidades actuales de los objetos */
for(i=0; i<(int)cab.num_obj; i++) 
{
	po = ddb + cab.obj_initially_pos + i;
	/* coge localidad inicial y la guarda en tabla */
	loc_obj[i]=(BYTE)*(po);
	/* si lleva objeto de inicio, incrementa contador objs. cogidos */
	if(loc_obj[i]==COGIDO) inc(FCARRIEDOBJ);
}

/* inicializa ventanas */
for(i=0; i<N_VENT; i++) vv_crea(0,0,80,25,0,7,NO_BORDE,&w[i]);

/* inicializa bancos de RAMSAVE y RAMLOAD */
for(i=0; i<BANCOS_RAM; i++) ram[i].usado=FALSE;

pro_act=0;                      /* número de proceso actual */
ptrp=0;                         /* puntero de pila */
resp_act=FALSE;                 /* NORESP */
nueva_ent=FALSE;

/* inicializa sistema de vídeo */
modo_video(modovideo);

}

/****************************************************************************
	FRD: controla la entrada de datos desde el fichero de entrada
	  mediante la función fread.
	  Entrada:      'fbd' puntero a fichero de base de datos
			'buff' puntero a buffer donde dejar datos leidos
			'tam' tamaño de datos a leer
			'cant' cantidad de datos a leer de tamaño 'tam'
****************************************************************************/
void frd(FILE *fbd, void *buff, size_t tam, size_t cant)
{

if(fread(buff,tam,cant,fbd)!=cant) {
	if(feof(fbd)) return;
	if(ferror(fbd)) {
		fclose(fbd);
		m_err(5,"Error en fichero de entrada",1);
	}
}

}

/****************************************************************************
	MAYUSCULA: convierte una letra en mayúscula.
	  Entrada:      'c' carácter a convertir
	  Salida:       mayúscula del carácter
****************************************************************************/
char mayuscula(char c)
{

if((c>='a') && (c<='z')) return(c-(char)'a'+(char)'A');

switch(c) {
	case (char)'�' :
		c=(char)'�';
		break;
	case (char)'�' :
		c='A';
		break;
	case (char)'�' :
		c='E';
		break;
	case (char)'�' :
		c='I';
		break;
	case (char)'�' :
		c='O';
		break;
	case (char)'�' :
	case (char)'�' :
		c='U';
		break;
}



return(c);
}

/****************************************************************************
	M_ERR: imprime mensajes de error en una ventana de pantalla.
	  Entrada:      'x' columna de inicio del texto dentro de la ventana
			de errores
			'm' puntero a mensaje a imprimir
			'flag' si distinto de 0 sale al sistema operativo
			con 'exit(flag)'
****************************************************************************/
void m_err(BYTE x, char *m, int flag)
{

rg_puntero(RG_OCULTA);

/* borra la pantalla */
cls();

/* crea ventana de errores */
vv_crea(WERR_FIL,WERR_COL,WERR_ANCHO,WERR_ALTO,WERR_COLORF,WERR_COLOR,BORDE_2,
  &vv_err);

/* centra ventana */
vv_err.vx=(BYTE)((columnastxt-vv_err.lx)/2);
vv_err.vxi=(BYTE)(vv_err.vx+1);

/* si no es mensaje vacío lo imprime */
if(*m) {
	vv_cls(&vv_err);        /* borra ventana para mensajes de error */
	vv_err.cvx=x*8;         /* coloca cursor */
	vv_err.cvy=0;
	vv_imps(m,&vv_err);     /* imprime mensaje */
	vv_lee_tecla();         /* espera a que se pulse una tecla */
	cls();                  /* borra la pantalla */
}

/* si flag es distinto de 0 */
if(flag) {
	g_modovideo(G_MV_T80C);
	/* restaura indicador de ruptura */
	setcbrk(ruptura);
	
	#if DEBUGGER==1
	if(img_debug!=NULL) farfree(img_debug);
	#endif

	exit(flag);             /* sale al sistema operativo */
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	ACTUALIZA_RATON: actualiza variables de estado del ratón.
****************************************************************************/
void actualiza_raton(void)
{


STC_RATONG r;

/* sale si ratón desactivado */
if(!getflagbit(FFLAGS, FFSINTAC)) return;

rg_estado(&r);

setflag(FMOUSEY,r.fil);
setflag(FMOUSEX,r.col);

if(r.boton1) set(FMOUSEB1); else clear(FMOUSEB1);
if(r.boton2) set(FMOUSEB2); else clear(FMOUSEB2);

}
