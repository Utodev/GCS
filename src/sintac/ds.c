/****************************************************************************
			 INTERPRETE-DEBUGGER SINTAC
			    (c)1995 JSJ Soft Ltd.

	NOTA: mediante la constante DEBUGGER se controla si se genera
	  el c¢digo del int‚rprete o del int‚rprete-debugger, mediante
	  la constante RUNTIME se controla si se genera m¢dulo "runtime".
	  Con: DEBUGGER=0 se genera el c¢digo del int‚rprete-debugger
	       DEBUGGER=1 se genera el c¢digo del int‚rprete
	       RUNTIME=0 genera c¢digo normal
	       RUNTIME=1 se genera m¢dulo "runtime"
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
extern BYTE objs_cogidos;       /* n£mero de objetos cogidos */
extern STC_VV w[N_VENT];        /* tabla para guardar par metros de ventanas */
extern int ptrp;                /* puntero de pila */
extern STC_BANCORAM ram[BANCOS_RAM];    /* para RAMSAVE y RAMLOAD */
extern STC_CONDACTO cd[];       /* tabla de funci¢n-tipo condactos */

/*** Variables globales ***/
#if DEBUGGER==1 && RUNTIME==0
#include "tabcond.h"            /* tabla de nombre-tipo de condactos */
BOOLEAN entorno=FALSE;          /* indica si se ejecuta desde entorno */
STC_VV vv_jsj;                  /* ventana de presentaci¢n */
STC_VV vv_deb;                  /* ventana de debugger */
BOOLEAN debugg=TRUE;            /* TRUE si paso a paso activado */
BOOLEAN pra_lin=FALSE;          /* TRUE si en primera l¡nea de una entrada */
unsigned char far *img_debug;   /* puntero buffer para fondo ventana debug. */
#endif

#if RUNTIME==1
long lng_runtime=0;             /* longitud (bytes) de m¢dulo 'runtime' */
#endif

/* cabecera de fichero de base de datos */
CAB_SINTAC cab;

/* nombre de fichero de base de datos */
char nf_base_datos[MAXPATH];

/* variables para Vocabulario */
struct palabra vocabulario[NUM_PAL];    /* para almacenar vocabulario */

/* variables para Mensajes de Sistema */
unsigned tab_desp_msy[MAX_MSY];         /* tabla de desplaz. mens. sist. */
char *tab_msy;                          /* puntero a inicio zona mens. sist. */

/* variables para Mensajes */
unsigned tab_desp_msg[MAX_MSG];         /* tabla de desplaz. de mensajes */
char *tab_msg;                          /* puntero a inicio zona de mensajes */
BYTE tabla_msg=0;                       /* tabla de mensajes cargada */

/* variables para Localidades */
unsigned tab_desp_loc[MAX_LOC];         /* tabla desplaz. textos de locs. */
char *tab_loc;                          /* puntero a inicio de texto de locs. */
/* variables para Conexiones */
unsigned tab_desp_conx[MAX_LOC];        /* tabla desplaz. lista conexiones */
BYTE *tab_conx;                         /* puntero inicio zona de conexiones */

/* variables para Objetos */
unsigned tab_desp_obj[MAX_OBJ];         /* tabla de desplaz.lista de objetos */
char *tab_obj;                          /* puntero a inicio zona de objetos */
char *tab_obj2;                         /* G3.25: copia de tabla de objetos */
					/* para restaurar al reiniciar */

/* variables para Procesos */
unsigned tab_desp_pro[MAX_PRO];         /* tabla desplazamiento de procesos */
BYTE *tab_pro;                          /* puntero a inicio zona de procesos */

STC_VV vv_err;                  /* ventana para mensajes de error */
BYTE var[VARS];                 /* variables del sistema (8 bits) */
BYTE flag[BANDS];               /* banderas del sistema, 256 banderas */
BYTE pro_act;                   /* n£mero de proceso actual */
BYTE *ptr_proc;                 /* puntero auxiliar */
unsigned sgte_ent;              /* desplazamiento de sgte. entrada */
BOOLEAN resp_act;               /* RESP (=1) o NORESP (=0) */
BOOLEAN nueva_ent;              /* indica que no debe ajustar ptr_proc para */
				/* saltar a siguiente entrada */

int ruptura;                    /* indicador de ruptura (BREAK) */

int modovideo;                  /* modo v¡deo, 0=640x480x16, 1=320x200x256 */
int columnastxt;                /* columnas de texto */

/*** Programa principal ***/
#if RUNTIME==1
#pragma warn -par
#endif
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

#if RUNTIME==0
if(argc<2) m_err(7,"Falta nombre de fichero",1);
#endif

#if DEBUGGER==1
/* detr s del nombre del fichero espera /lxx o /Lxx (o -lxx o -Lxx) */
if(argc==3) {
	/* si introdujo /l o /L (o -l o -L) recoge los dos siguientes */
	/* d¡gitos y calcula l¡nea de la ventana del debugger */
	if(((argv[2][0]=='/') || (argv[2][0]=='-')) && ((argv[2][1]=='l') ||
	  (argv[2][1]=='L'))) {
		lin_deb=(BYTE)(((argv[2][2]-'0')*10)+(argv[2][3]-'0'));
		max_lindeb=(BYTE)(MODO0_FIL-WDEB_ALTO);
		if(lin_deb>max_lindeb) lin_deb=max_lindeb;
	}
}
/* como tercer par metro espera /e o /E (o -E o -e) */
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

/* instala 'handler' de errores cr¡ticos */
harderr(int24_hnd);

cls();

/* carga base de datos e inicializa variables */
#if RUNTIME==0
carga_bd(argv[1]);
#else
carga_bd(argv[0]);
#endif

inic();

/* guarda indicador de ruptura y lo desactiva */
ruptura=getcbrk();
setcbrk(0);

#if DEBUGGER==1
/* presentaci¢n */
cls();
if(entorno==FALSE) {
	rg_puntero(RG_OCULTA);
	vv_crea(WJSJ_FIL,WJSJ_COL,WJSJ_ANCHO,WJSJ_ALTO,1,15,BORDE_3,&vv_jsj);
	/* centra ventana */
	vv_jsj.vx=(BYTE)((MODO0_COL-vv_jsj.lx)/2);
	vv_jsj.vxi=(BYTE)(vv_jsj.vx+1);
	vv_cls(&vv_jsj);
	vv_imps("\n  Int‚rprete-Debugger versi¢n "VERSION"\n"
	       "\n  "COPYRIGHT"\n\n\n"
	       "\n           Pulsa una tecla",&vv_jsj);
	vv_lee_tecla();
	cls();
	rg_puntero(RG_MUESTRA);
}
#endif

/* inicializa puntero a proceso actual */
ptr_proc=tab_pro+tab_desp_pro[pro_act];

while(1) {
	/* actualiza variables de estado de rat¢n */
	actualiza_raton();

	/* si no es fin de proceso */
	if(*ptr_proc) {
		/* si 'res_pro' es FALSE no debe ejecutar entrada */
		res_pro=!resp_act || (resp_act && ((*ptr_proc==NO_PAL) ||
		  (*ptr_proc==var[2])) && ((*(ptr_proc+1)==NO_PAL) ||
		  (*(ptr_proc+1)==var[3])));

		/* salta verbo-nombre */
		ptr_proc+=2;

		/* calcula el desplazamiento de la siguiente entrada */
		sgte_ent=(*(ptr_proc+1) << 8) | *ptr_proc;
		ptr_proc+=2;

		#if DEBUGGER==1
		/* indica que es primera l¡nea de entrada */
		pra_lin=TRUE;
		#endif

	}
	/* si fin de proceso */
	else {
		res_pro=done();
		ptr_proc++;     /* ajustamos ptr_proc */

		#if DEBUGGER==1
		pra_lin=FALSE;  /* no es primera l¡nea de entrada */
		#endif
	}

	/* indica que deber  ajustar ptr_proc a siguiente */
	/* a menos que alg£n condacto cambie esta variable */
	nueva_ent=TRUE;

	/* si 'res_pro' es TRUE y no fin de entrada ejecuta esta entrada */
	/* si no, salta a la siguiente */
	while(res_pro && *ptr_proc) {
		#if DEBUGGER==1
		/* guarda direcci¢n de condacto en curso */
		pro_d=ptr_proc;
		#endif

		/* si hay indirecci¢n */
		if(*ptr_proc==INDIR) {
			ptr_proc++;     /* salta prefijo indirecci¢n */
			indir=*ptr_proc++;
		}
		else indir=0;

		ncondacto=*ptr_proc;
		npar=cd[ncondacto].npar;

		msc_indir=0x01;
		for(i=0; i<npar; i++) {
			if(indir & msc_indir) par[i]=var[*(ptr_proc+i+1)];
			else par[i]=*(ptr_proc+i+1);
			msc_indir <<= 1;
		}

		#if DEBUGGER==1
		/* si est  activado el paso a paso */
		if(debugg==TRUE) debugger(indir,npar,pro_d);
		/* sino, activa paso a paso */
		else if(bioskey(1)==F10) {
			bioskey(1);
			debugg=TRUE;
			/* indicamos que no es primera l¡nea de entrada para */
			/* que imp_condacto() no imprima el verbo-nombre ya */
			/* que el puntero al condacto no est  ajustado */
			pra_lin=FALSE;
		}
		#endif

		/* ejecuta condacto seg£n n£mero de par metros */
		#pragma warn -pro
		switch(npar) {
			case 0 :
				res_pro=cd[ncondacto].cond();
				ptr_proc++;
				break;
			case 1 :
				res_pro=cd[ncondacto].cond(par[0]);
				ptr_proc+=2;
				break;
			case 2 :
				res_pro=cd[ncondacto].cond(par[0],par[1]);
				ptr_proc+=3;
				break;
			case 3 :
				res_pro=cd[ncondacto].cond(par[0],par[1],
				  par[2]);
				ptr_proc+=4;
				break;
			case 4 :
				res_pro=cd[ncondacto].cond(par[0],par[1],
				  par[2],par[3]);
				ptr_proc+=5;
				break;
			case 7 :
				res_pro=cd[ncondacto].cond(par[0],par[1],
				  par[2],par[3],par[4],par[5],par[6]);
				ptr_proc+=8;
				break;
		}
		#pragma warn +pro

		if(res_pro) nueva_ent=TRUE;
	}

	/* si fin entrada, pasa a la siguiente */
	if(!*ptr_proc) ptr_proc++;
	else if(nueva_ent==TRUE) {
		ptr_proc=tab_pro+tab_desp_pro[pro_act]+sgte_ent;
		nueva_ent=FALSE;
	}
}

}
#if RUNTIME==1
#pragma warn +par
#endif

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

#if DEBUGGER==1
/****************************************************************************
	SACA_PAL: devuelve el n£mero de la primera entrada en el vocabulario
	  que se corresponda con el n£mero y tipo de palabra dado.
	  Entrada:      'num_pal' n£mero de la palabra a buscar
			'tipo_pal' tipo de la palabra a buscar
	  Salida:       n£mero dentro de la tabla de vocabulario o
			(NUM_PAL+1) si no se encontr¢
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
	  Entrada:      'indir' indicadores de indirecci¢n
			'npar' n£mero de par metros
			'pro_d' direcci¢n del condacto en curso
		      variables globales:-
			'pra_lin' TRUE si es 1era l¡nea de entrada
			'ptr_proc' puntero a byte de condacto + par metros
****************************************************************************/
void imp_condacto(BYTE indir, BYTE npar, BYTE *pro_d)
{
BYTE i, *pcp, msc_indir;
int j, dirrel;
unsigned dir;
char lin_cond[LNG_LINDEB+1], par[7];
char *Pal_Nula="-     ";

/* si es la primera l¡nea de la entrada pone el puntero apuntando al */
/* campo verbo, si no apunta al condacto */
if(pra_lin==TRUE) dir=(unsigned)(pro_d-tab_pro-4);
else dir=(unsigned)(pro_d-tab_pro);

/* imprime direcci¢n del condacto */
sprintf(lin_cond,"%5u: ",dir);
vv_imps(lin_cond,&vv_deb);

if(pra_lin==TRUE) {
	pcp=ptr_proc-4;                 /* apunta a verbo-nombre */
	if(indir) pcp-=2;               /* ajuste por indirecci¢n */
	if(*pcp==NO_PAL) sprintf(lin_cond,"%s  ",Pal_Nula);
	else {
		j=saca_pal(*pcp,_VERB);
		/* si no es verbo, quiz  sea nombre */
		if(j==(NUM_PAL+1)) j=saca_pal(*pcp,_NOMB);
		sprintf(lin_cond,"%s  ",vocabulario[j].p);
	}
	vv_imps(lin_cond,&vv_deb);
	pcp++;

	if(*pcp==NO_PAL) sprintf(lin_cond,"%s",Pal_Nula);
	else sprintf(lin_cond,"%s",vocabulario[saca_pal(*pcp,_NOMB)].p);
	vv_imps(lin_cond,&vv_deb);
}

/* siguiente l¡nea de ventana */
vv_deb.cvx=1*8;
vv_deb.cvy+=vv_deb.chralt;

/* imprime condacto */
sprintf(lin_cond,"%s ",condacto[*ptr_proc].cnd);
vv_imps(lin_cond,&vv_deb);

/* imprime par metros seg£n tipo de condacto */
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
			/* si no es vebro, quiz  sea nombre convertible */
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

/* imprime par metros */
vv_imps(lin_cond,&vv_deb);

/* si estaba en primera l¡nea de entrada indica que ya no est  */
pra_lin=FALSE;

}

/****************************************************************************
	INP_DEB: rutina de introducci¢n por teclado de n£meros para
	  debugger.
	  Salida:       n£mero introducido en el rango 0-255 (BYTE).
****************************************************************************/
BYTE inp_deb(void)
{
unsigned k;
char numero[4];
int i, antcwx, anch;

/* guarda antigua posici¢n cursor */
antcwx=vv_deb.cvx;

/* repite hasta que sea un n£mero v lido */
do {
	i=0;
	/* restaura posici¢n cursor */
	vv_deb.cvx=antcwx;

	/* repite mientras no introduzca 3 d¡gitos o no pulse RETURN */
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

	/* pasa cadena ASCII a n£mero */
	i=atoi(numero);
} while((i<0) || (i>255));

return((BYTE)i);
}

/****************************************************************************
	IMP_VARBAND: imprime en la l¡nea del debugger la variable y bandera
	  actuales.
	  Entrada:      'variable' n£mero de variable a imprimir
			'bandera' n£mero de bandera a imprimir
****************************************************************************/
void imp_varband(BYTE variable, BYTE bandera)
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
sprintf(lin_deb,"%3u\xff\xff",var[variable]);
vv_deb.cvx=17*8;
vv_imps(lin_deb,&vv_deb);

/* si es una variable de sentencia l¢gica imprime adem s la */
/* palabra del vocabulario correspondiente */
if(variable==2) {
	palabra=saca_pal(var[2],_VERB);
	/* puede ser nombre convertible */
	if(palabra==(NUM_PAL+1)) palabra=saca_pal(var[2],_NOMB);
}
else if(variable==3) palabra=saca_pal(var[3],_NOMB);
else if(variable==4) palabra=saca_pal(var[4],_ADJT);
else if(variable==5) palabra=saca_pal(var[5],_NOMB);
else if(variable==6) palabra=saca_pal(var[6],_ADJT);
else palabra=NUM_PAL+1;

sprintf(lin_deb,"\xff\xff\xff\xff\xff\xff");
vv_deb.cvx=21*8;
vv_imps(lin_deb,&vv_deb);

if(palabra==(NUM_PAL+1)) sprintf(lin_deb,"------");
else sprintf(lin_deb,"%s",vocabulario[palabra].p);

vv_deb.cvx=21*8;
vv_imps(lin_deb,&vv_deb);

/* imprime bandera */
vv_deb.cvx=28*8;
vv_deb.cvy=0;
vv_imps("Band ",&vv_deb);
sprintf(lin_deb,"%3u=\xff",bandera);
vv_deb.cvx=33*8;
vv_imps(lin_deb,&vv_deb);
sprintf(lin_deb,"%1u\xff",notzero(bandera));
vv_deb.cvx=37*8;
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
	  Entrada:      'indir' indicadores de indirecci¢n
			'npar' n£mero de par metros
			'pro_d' direcci¢n del condacto en curso
			'variable', 'bandera' n£mero de variable y bandera
			que se mostrar n
			'txt_deb' texto a imprimir en la £ltima l¡nea
****************************************************************************/
void imp_debugger(BYTE indir, BYTE npar, BYTE *pro_d, BYTE variable,
  BYTE bandera, char *txt_deb)
{
char lin_deb[LNG_LINDEB+1];

/* borra la ventana del debugger e imprime informaci¢n */
vv_cls(&vv_deb);

vv_deb.cvx=1*8;
vv_deb.cvy=0;
sprintf(lin_deb,"PRO %3u",(unsigned)pro_act);
vv_imps(lin_deb,&vv_deb);
imp_varband(variable,bandera);

vv_deb.cvx=1*8;
vv_deb.cvy=1*vv_deb.chralt;
imp_condacto(indir,npar,pro_d);

vv_deb.cvx=1*8;
vv_deb.cvy=3*vv_deb.chralt;
vv_imps(txt_deb,&vv_deb);

}

/****************************************************************************
	DEBUGGER: funci¢n principal del debugger.
	  Entrada:      'indir' indicadores de indirecci¢n
			'npar' n£mero de par metros
			'pro_d' direcci¢n del condacto en curso
****************************************************************************/
void debugger(BYTE indir, BYTE npar, BYTE *pro_d)
{
unsigned tecla;
static BYTE variable=0;
static BYTE bandera=0;
BYTE valor;
char *txt_deb1="Var.  Band.  Pant.  Desact.  Salir";
char *txt_deb2="Otra  Modificar  Fin"
  "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";

guarda_debugger();

/* imprime ventana del debugger */
imp_debugger(indir,npar,pro_d,variable,bandera,txt_deb1);

tecla=mayuscula((char)vv_lee_tecla());

while(esta_en("VBPDS",(char)tecla)) {
	switch(tecla) {
		case 'V' :      /* variables */
			vv_deb.cvx=1*8;
			vv_deb.cvy=3*vv_deb.chralt;
			vv_imps(txt_deb2,&vv_deb);
			do {
				imp_varband(variable,bandera);
				tecla=mayuscula((char)vv_lee_tecla());
				switch((BYTE)tecla) {
				case 'M' :              /* modificar */
					vv_deb.cvx=17*8;
					vv_deb.cvy=0;
					valor=inp_deb();
					var[variable]=valor;
					break;
				case 'O' :              /* otra variable */
					vv_deb.cvx=13*8;
					vv_deb.cvy=0;
					variable=inp_deb();
					break;
				case COD_ARR :
					variable--;
					break;
				case COD_ABJ :
					variable++;
					break;
				}
			} while(tecla!='F');
			break;
		case 'B' :      /* banderas */
			vv_deb.cvx=1*8;
			vv_deb.cvy=3*vv_deb.chralt;
			vv_imps(txt_deb2,&vv_deb);
			do {
				imp_varband(variable,bandera);
				tecla=mayuscula((char)vv_lee_tecla());
				switch((BYTE)tecla) {
				case 'M' :              /* modificar */
					vv_deb.cvx=37*8;
					vv_deb.cvy=0;
					vv_impc(CUR_DEBUG,&vv_deb);
					do {
						tecla=vv_lee_tecla();
					} while((tecla!='0') && (tecla!='1'));
					if(tecla=='0') clear(bandera);
					else set(bandera);
					break;
				case 'O' :      /* otra bandera */
					vv_deb.cvx=33*8;
					vv_deb.cvy=0;
					bandera=inp_deb();
					break;
				case COD_ARR :
					bandera--;
					break;
				case COD_ABJ :
					bandera++;
					break;
				}
			} while(tecla!='F');
			break;
		case 'P' :      /* pantalla */
			recupera_debugger();
			vv_lee_tecla();
			imp_debugger(indir,npar,pro_d,variable,bandera,
			  txt_deb1);
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
			'modovideo' £ltimo modo seleccionado
			'columnastxt' n£mero de columnas de texto
****************************************************************************/
void modo_video(int modo)
{

rg_puntero(RG_OCULTA);

/* selecciona modo de v¡deo y actualiza variables 14 y 15 */
if(modo==0) {
	/* modo de 640x480x16, 80 columnas, 30 filas */
	g_modovideo(G_MV_G3C16);
	modovideo=0;
	columnastxt=MODO0_COL;
	var[14]=MODO0_COL;
	var[15]=MODO0_FIL;
	clear(5);
}
else {
	/* modo de 320x200x256, 40 columnas, 25 filas */
	g_modovideo(G_MV_G1C256);
	modovideo=1;
	columnastxt=MODO1_COL;
	var[14]=MODO1_COL;
	var[15]=MODO1_FIL;
	set(5);
}

rg_inicializa();
if(zero(10)) rg_desconecta();

}

#if RUNTIME==0
/****************************************************************************
	CARGA_BD: carga la base de datos.
	  Entrada:      'nombre' nombre de fichero de base de datos
****************************************************************************/
void carga_bd(char *nombre)
{
FILE *fbd;
char *errmem="No hay suficiente memoria";
char *srecon=SRECON;
unsigned i, bytes_msg;

if((fbd=fopen(nombre,"rb"))==NULL)
  m_err(1,"Error de apertura fichero de entrada",1);

/* guarda nombre de fichero de base de datos */
strcpy(nf_base_datos,nombre);

/* lee cabecera */
frd(fbd,&cab,sizeof(CAB_SINTAC),1);

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1]))
  m_err(5,"Fichero de entrada no v lido",1);

/* Reserva de memoria para las distintas secciones */
/* Mensajes del Sistema */
if((tab_msy=(char *)malloc((size_t)cab.bytes_msy))==NULL) m_err(6,errmem,1);

/* Mensajes */
/* reserva memoria para tabla de mensajes m s grande */
bytes_msg=0;
for(i=0; i<MAX_TMSG; i++) {
	if(cab.fpos_msg[i]!=(fpos_t)0) {
		if(cab.bytes_msg[i]>bytes_msg) bytes_msg=cab.bytes_msg[i];
	}
}
if((tab_msg=(char *)malloc((size_t)bytes_msg))==NULL) m_err(6,errmem,1);

/* Localidades */
if((tab_loc=(char *)malloc((size_t)cab.bytes_loc))==NULL) m_err(6,errmem,1);
/* Conexiones */
if((tab_conx=(BYTE *)malloc((size_t)cab.bytes_conx))==NULL) m_err(6,errmem,1);

/* Objetos */
if((tab_obj=(char *)malloc((size_t)cab.bytes_obj))==NULL) m_err(6,errmem,1);
/* G3.25: para guardar una copia de los objetos */
if((tab_obj2=(char *)malloc((size_t)cab.bytes_obj))==NULL) m_err(6,errmem,1);

/* Procesos */
if((tab_pro=(BYTE *)malloc((size_t)cab.bytes_pro))==NULL) m_err(6,errmem,1);

fseek(fbd,cab.fpos_voc,SEEK_SET);
frd(fbd,vocabulario,sizeof(struct palabra),cab.pal_voc);

fseek(fbd,cab.fpos_msy,SEEK_SET);
frd(fbd,tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY);
frd(fbd,tab_msy,sizeof(char),cab.bytes_msy);

/* busca primera tabla de mensajes disponible y la carga */
for(i=0; i<MAX_TMSG; i++) if(cab.fpos_msg[i]!=(fpos_t)0) break;
tabla_msg=(BYTE)i;
fseek(fbd,cab.fpos_msg[i],SEEK_SET);
frd(fbd,tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG);
frd(fbd,tab_msg,sizeof(char),cab.bytes_msg[i]);

fseek(fbd,cab.fpos_loc,SEEK_SET);
frd(fbd,tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC);
frd(fbd,tab_loc,sizeof(char),cab.bytes_loc);
frd(fbd,tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC);
frd(fbd,tab_conx,sizeof(BYTE),cab.bytes_conx);

fseek(fbd,cab.fpos_obj,SEEK_SET);
frd(fbd,tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ);
frd(fbd,tab_obj,sizeof(char),cab.bytes_obj);

fseek(fbd,cab.fpos_pro,SEEK_SET);
frd(fbd,tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO);
frd(fbd,tab_pro,sizeof(BYTE),cab.bytes_pro);

fclose(fbd);

/* decodifica las secciones */
codifica((BYTE *)tab_msy,cab.bytes_msy);
codifica((BYTE *)tab_msg,cab.bytes_msg[tabla_msg]);
codifica((BYTE *)tab_loc,cab.bytes_loc);
codifica(tab_conx,cab.bytes_conx);
codifica((BYTE *)tab_obj,cab.bytes_obj);
codifica(tab_pro,cab.bytes_pro);

/* G3.25: guarda copia de los objetos */
for(i=0; i<cab.bytes_obj; i++) tab_obj2[i]=tab_obj[i];

}
#else
/****************************************************************************
	CARGA_BD: carga la base de datos (m¢dulo runtime).
	  Entrada:      'nombre' nombre de fichero EXE con base de
			datos 'linkada'
****************************************************************************/
void carga_bd(char *nombre)
{
FILE *fbd;
char *errmem="No hay suficiente memoria";
char *srecon=SRECON;
unsigned bytes_msg;
long pos;
int i;

if((fbd=fopen(nombre,"rb"))==NULL)
  m_err(1,"Error de apertura fichero de entrada",1);

/* guarda nombre de fichero de base de datos */
strcpy(nf_base_datos,nombre);

/* al final de fichero 'linkado' (runtime+base de datos) debe estar */
/* la longitud de m¢dulo 'runtime' */
pos=0L-sizeof(long);
fseek(fbd,pos,SEEK_END);
frd(fbd,&lng_runtime,sizeof(long),1);

/* posiciona puntero de fichero en inicio de base de datos */
fseek(fbd,lng_runtime,SEEK_SET);

/* lee cabecera */
frd(fbd,&cab,sizeof(CAB_SINTAC),1);

/* comprueba que la versi¢n de la base de datos sea correcta */
if((cab.srecon[L_RECON-2]!=srecon[L_RECON-2]) ||
  (cab.srecon[L_RECON-1]!=srecon[L_RECON-1]))
  m_err(5,"Fichero de entrada no v lido",1);

/* Reserva de memoria para las distintas secciones */
/* Mensajes del Sistema */
if((tab_msy=(char *)malloc((size_t)cab.bytes_msy))==NULL) m_err(6,errmem,1);

/* Mensajes */
/* reserva memoria para tabla de mensajes m s grande */
bytes_msg=0;
for(i=0; i<MAX_TMSG; i++) {
	if(cab.fpos_msg[i]!=(fpos_t)0) {
		if(cab.bytes_msg[i]>bytes_msg) bytes_msg=cab.bytes_msg[i];
	}
}
if((tab_msg=(char *)malloc((size_t)bytes_msg))==NULL) m_err(6,errmem,1);

/* Localidades */
if((tab_loc=(char *)malloc((size_t)cab.bytes_loc))==NULL) m_err(6,errmem,1);
/* Conexiones */
if((tab_conx=(BYTE *)malloc((size_t)cab.bytes_conx))==NULL) m_err(6,errmem,1);

/* Objetos */
if((tab_obj=(char *)malloc((size_t)cab.bytes_obj))==NULL) m_err(6,errmem,1);
/* G3.25: para guardar una copia de los objetos */
if((tab_obj2=(char *)malloc((size_t)cab.bytes_obj))==NULL) m_err(6,errmem,1);

/* Procesos */
if((tab_pro=(BYTE *)malloc((size_t)cab.bytes_pro))==NULL) m_err(6,errmem,1);

fseek(fbd,cab.fpos_voc+lng_runtime,SEEK_SET);
frd(fbd,vocabulario,sizeof(struct palabra),cab.pal_voc);

fseek(fbd,cab.fpos_msy+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_msy,sizeof(unsigned),(size_t)MAX_MSY);
frd(fbd,tab_msy,sizeof(char),cab.bytes_msy);

/* busca primera tabla de mensajes disponible y la carga */
for(i=0; i<MAX_TMSG; i++) if(cab.fpos_msg[i]!=(fpos_t)0) break;
tabla_msg=(BYTE)i;
fseek(fbd,cab.fpos_msg[i]+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG);
frd(fbd,tab_msg,sizeof(char),cab.bytes_msg[i]);

fseek(fbd,cab.fpos_loc+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_loc,sizeof(unsigned),(size_t)MAX_LOC);
frd(fbd,tab_loc,sizeof(char),cab.bytes_loc);
frd(fbd,tab_desp_conx,sizeof(unsigned),(size_t)MAX_LOC);
frd(fbd,tab_conx,sizeof(BYTE),cab.bytes_conx);

fseek(fbd,cab.fpos_obj+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_obj,sizeof(unsigned),(size_t)MAX_OBJ);
frd(fbd,tab_obj,sizeof(char),cab.bytes_obj);

fseek(fbd,cab.fpos_pro+lng_runtime,SEEK_SET);
frd(fbd,tab_desp_pro,sizeof(unsigned),(size_t)MAX_PRO);
frd(fbd,tab_pro,sizeof(BYTE),cab.bytes_pro);

fclose(fbd);

/* decodifica las secciones */
codifica((BYTE *)tab_msy,cab.bytes_msy);
codifica((BYTE *)tab_msg,cab.bytes_msg[tabla_msg]);
codifica((BYTE *)tab_loc,cab.bytes_loc);
codifica(tab_conx,cab.bytes_conx);
codifica((BYTE *)tab_obj,cab.bytes_obj);
codifica(tab_pro,cab.bytes_pro);

/* G3.25: guarda copia de los objetos */
for(i=0; i<cab.bytes_obj; i++) tab_obj2[i]=tab_obj[i];

}
#endif

/****************************************************************************
	CODIFICA: codifica/decodifica una tabla de secci¢n.
	  Entrada:      'mem' puntero a la tabla a codificar/decodificar
			'bytes_mem' tama¤o de la tabla
****************************************************************************/
void codifica(BYTE *mem, unsigned bytes_mem)
{
BYTE *p, *ult_p;

p=mem;
ult_p=p+bytes_mem;

for(; p<ult_p; p++) *p=CODIGO(*p);

}

/****************************************************************************
	INIC: inicializa diversas tablas y variables.
	  Entrada:      variables globales:-
			  'vid' con informaci¢n de sistema de v¡deo
****************************************************************************/
void inic(void)
{
int i;
char *po;

/* n£mero de objetos cogidos */
objs_cogidos=0;

/* inicializa tabla de localidades actuales de los objetos */
for(i=0; i<(int)cab.num_obj; i++) {
	po=tab_obj+tab_desp_obj[i];
	/* coge localidad inicial y la guarda en tabla */
	loc_obj[i]=(BYTE)*(po+2);
	/* si lleva objeto de inicio, incrementa contador objs. cogidos */
	if((loc_obj[i]==PUESTO) || (loc_obj[i]==COGIDO)) objs_cogidos++;
}

/* inicializa variables */
for(i=0; i<VARS; i++) {
	/* variables de sentencia l¢gica inicializadas a NO_PAL */
	if((i>1) && (i<7)) var[i]=NO_PAL;
	else var[i]=0;
}

/* inicializa banderas */
for(i=0; i<BANDS; i++) flag[i]=0;

/* inicializa ventanas */
for(i=0; i<N_VENT; i++) vv_crea(0,0,80,25,0,7,NO_BORDE,&w[i]);

/* inicializa bancos de RAMSAVE y RAMLOAD */
for(i=0; i<BANCOS_RAM; i++) ram[i].usado=FALSE;

pro_act=0;                      /* n£mero de proceso actual */
ptrp=0;                         /* puntero de pila */
resp_act=FALSE;                 /* NORESP */
nueva_ent=FALSE;

/* como siempre se deber  ejecutar en VGA pone a 1 la bandera 4 */
/* por compatibilidad con versiones anteriores */
set(4);

/* inicializa sistema de v¡deo */
modo_video(modovideo);

/* tabla de mensajes cargada inicialmente */
var[17]=tabla_msg;

}

/****************************************************************************
	FRD: controla la entrada de datos desde el fichero de entrada
	  mediante la funci¢n fread.
	  Entrada:      'fbd' puntero a fichero de base de datos
			'buff' puntero a buffer donde dejar datos leidos
			'tam' tama¤o de datos a leer
			'cant' cantidad de datos a leer de tama¤o 'tam'
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
	MAYUSCULA: convierte una letra en may£scula.
	  Entrada:      'c' car cter a convertir
	  Salida:       may£scula del car cter
****************************************************************************/
char mayuscula(char c)
{

if((c>='a') && (c<='z')) return(c-(char)'a'+(char)'A');

switch(c) {
	case (char)'¤' :
		c=(char)'¥';
		break;
	case (char)' ' :
		c='A';
		break;
	case (char)'‚' :
		c='E';
		break;
	case (char)'¡' :
		c='I';
		break;
	case (char)'¢' :
		c='O';
		break;
	case (char)'£' :
	case (char)'' :
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

/* si no es mensaje vac¡o lo imprime */
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
	/* libera memoria */
	free(tab_msy);
	free(tab_msg);
	free(tab_loc);
	free(tab_conx);
	free(tab_obj);
	free(tab_obj2);         /* G3.25 */
	free(tab_pro);

	#if DEBUGGER==1
	if(img_debug!=NULL) farfree(img_debug);
	#endif

	exit(flag);             /* sale al sistema operativo */
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	ACTUALIZA_RATON: actualiza variables de estado del rat¢n.
****************************************************************************/
void actualiza_raton(void)
{
STC_RATONG r;

/* sale si rat¢n desactivado */
if(zero(10)) return;

rg_estado(&r);

var[18]=r.fil;
var[19]=r.col;

if(r.boton1) set(8);
else clear(8);

if(r.boton2) set(9);
else clear(9);

}
