/****************************************************************************
	Funciones para ejecutar cada uno de los condactos del SINTAC
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <bios.h>
#include <dos.h>
#include <dir.h>
#include <time.h>
#include <process.h>
#include <alloc.h>
#include "version.h"
#include "sintac.h"
#include "graf.h"
#include "aafli.h"
#include "vv.h"
#include "grfpcx.h"
#include "sonido.h"
#include "condacto.h"

/*** Variables externas ***/
#if DEBUGGER==1
extern BOOLEAN debugg;          /* TRUE si paso a paso activado */
extern BOOLEAN pra_lin;         /* TRUE si en primera l°nea de una entrada */
extern STC_VV vv_deb;           /* ventana de debugger */
extern unsigned char far *img_debug;    /* puntero buffer para fondo debug. */
#endif

#if RUNTIME==1
extern long lng_runtime;        /* longitud (bytes) de m¢dulo 'runtime' */
#endif

extern CAB_SINTAC cab;          /* cabecera de fichero de base de datos */
extern char nf_base_datos[MAXPATH];     /* nombre de fichero base de datos */
extern BYTE *ptr_proc;          /* puntero auxiliar */
extern unsigned sgte_ent;       /* desplazamiento de sgte. entrada */
extern BYTE pro_act;            /* n£mero de proceso actual */
extern unsigned tab_desp_pro[MAX_PRO];  /* tabla desplazamiento de procesos */
extern BYTE *tab_pro;           /* puntero a inicio zona de procesos */
extern unsigned tab_desp_msg[MAX_MSG];  /* tabla de desplaz. de mensajes */
extern char *tab_msg;           /* puntero a inicio zona de mensajes */
extern BYTE tabla_msg;          /* tabla de mensajes cargada */
extern unsigned tab_desp_msy[MAX_MSY];  /* tabla de desplaz. mens. sist. */
extern char *tab_msy;           /* puntero a inicio zona mens. sist. */
extern unsigned tab_desp_loc[MAX_LOC];  /* tabla desplaz. textos de locs. */
extern char *tab_loc;           /* puntero a inicio de texto de locs. */
extern unsigned tab_desp_obj[MAX_OBJ];  /* tabla de desplaz.lista de objetos */
extern char *tab_obj;           /* puntero a inicio zona de objetos */
extern char *tab_obj2;          /* G3.25: puntero a inicio copia de objetos */
extern unsigned tab_desp_conx[MAX_LOC]; /* tabla desplaz. lista conexiones */
extern BYTE *tab_conx;          /* puntero inicio zona de conexiones */
extern struct palabra vocabulario[NUM_PAL];
extern BOOLEAN nueva_ent;
extern BOOLEAN resp_act;        /* RESP (=1) o NORESP (=0) */
extern BYTE var[VARS];          /* variables del sistema (8 bits) */
extern BYTE flag[BANDS];        /* banderas del sistema, 256 banderas */
extern int modovideo;           /* modo de v°deo */

/*** Variables globales ***/
char Car_Pal[]="ABCDEFGHIJKLMNùOPQRSTUVWXYZ0123456789";
char C_No_Sig[]=" !???";        /* caracteres no significativos */
char C_Separ[]=".,;:\"\'";	/* separadores */
int ptrp;                       /* puntero de pila */
BYTE *pila1[STK];               /* para guardar direcciones de retorno */
unsigned pila2[STK];            /*   "     "    deplz. sgte. entrada */
BYTE pila3[STK];                /*   "     "    num. de proc. de llamada */
STC_VV w[N_VENT];               /* tabla para guardar par†metros de ventanas */
BYTE loc_obj[MAX_OBJ];          /* tabla de localidades act. de objetos */
BYTE objs_cogidos;              /* n£mero de objetos cogidos */
BOOLEAN ini_inp=FALSE;          /* se pone a TRUE para indicar a parse() */
				/* el inicio de frase */
char frase[MAXLONG];            /* buffer para guardar frase tecleada */
char *lin;                      /* puntero auxiliar a 'frase', usado por */
				/* parse1() y parse() */
BOOLEAN mas_texto=FALSE;        /* usada por parse1() para frases encaden. */
int ptr_nexto=-1;               /* puntero NEXTO a tabla de objetos */
BOOLEAN doall=FALSE;            /* indicador de bucle DOALL */
STC_BANCORAM ram[BANCOS_RAM];   /* para RAMSAVE y RAMLOAD */
char f_sl[MAXLONG];             /* buffer para guardar nombre fichero */
unsigned char far cardef8x16[256][16];  /* tablas con las definiciones */
unsigned char far cardef8x8[256][8];    /* de caracteres */
unsigned char anchdef8x16[256]; /* tablas con anchuras de caracteres */
unsigned char anchdef8x8[256];

STC_CONDACTO cd[]={             /* tabla funci¢n-n? par†metros de condacto */
	0       , 0,            /* condacto 0, reservado para fin entrada */
	process , 1,
	done    , 0,
	notdone , 0,
	resp    , 0,
	noresp  , 0,
	defwin  , 7,
	window  , 1,
	clw     , 1,
	let     , 2,
	eq      , 2,
	noteq   , 2,
	lt      , 2,
	gt      , 2,
	mes     , 2,
	newline , 0,
	message , 2,
	sysmess , 1,
	desc    , 1,
	add     , 2,
	sub     , 2,
	inc     , 1,
	dec     , 1,
	set     , 1,
	clear   , 1,
	zero    , 1,
	notzero , 1,
	place   , 2,
	get     , 1,
	drop    , 1,
	input   , 0,
	parse   , 0,
	skip    , 2,
	at      , 1,
	notat   , 1,
	atgt    , 1,
	atlt    , 1,
	adject1 , 1,
	noun2   , 1,
	adject2 , 1,
	listat  , 1,
	isat    , 2,
	isnotat , 2,
	present , 1,
	absent  , 1,
	worn    , 1,
	notworn , 1,
	carried , 1,
	notcarr , 1,
	wear    , 1,
	remove1 , 1,
	create  , 1,
	destroy , 1,
	swap    , 2,
	restart , 0,
	whato   , 0,
	move    , 1,
	ismov   , 0,
	goto1   , 1,
	print   , 1,
	dprint  , 1,
	cls     , 0,
	anykey  , 0,
	pause   , 1,
	listobj , 0,
	firsto  , 0,
	nexto   , 1,
	synonym , 2,
	hasat   , 1,
	hasnat  , 1,
	light   , 0,
	nolight , 0,
	random1 , 2,
	seed    , 1,
	puto    , 1,
	inkey   , 0,
	copyov  , 2,
	chance  , 1,
	ramsave , 1,
	ramload , 3,
	ability , 1,
	autog   , 0,
	autod   , 0,
	autow   , 0,
	autor   , 0,
	isdoall , 0,
	ask     , 3,
	quit    , 0,
	save    , 0,
	load    , 2,
	exit1   , 1,
	end1    , 0,
	printat , 2,
	saveat  , 0,
	backat  , 0,
	newtext , 0,
	printc  , 1,
	ink     , 1,
	paper   , 1,
	bright  , 1,
	blink   , 1,
	color   , 1,
	debug   , 1,
	wborder , 2,
	charset , 1,
	extern1 , 2,
	gtime   , 1,
	time1   , 2,
	timeout , 0,
	mode    , 1,
	graphic , 3,
	remapc  , 4,
	setat   , 1,
	clearat , 1,
	getrgb  , 4,
	playfli , 2,
	mouse   , 1,
	mousexy , 4,
	mousebt , 1,
	play    , 2,
	music   , 2,
};

/*** Prototipos de funciones externas ***/
void modo_video(int modo);
void codifica(BYTE *mem, unsigned bytes_mem);
void inic(void);
char mayuscula(char c);
void m_err(BYTE x, char *m, int flag);


/****************************************************************************
	SETVAR: Asigna un valor a una variable. Este cambio ha sido realizado
	  por Uto para tratar de compatibilizar con DAAD, ahora todas las 
	  escrituras de las variables (lo que ser°an los flags en DAAD) pasan
	  por aqu°.
****************************************************************************/

void setvar(BYTE nv, BYTE value)
{
	var[nv] = value;
}


/****************************************************************************
	GETVAR: Obtiene el valor a una variable. Este cambio ha sido realizado
	  por Uto para tratar de compatibilizar con DAAD, ahora todas las 
	  lecturas de las variables (lo que ser°an los flags en DAAD) pasan
	  por aqu°.
****************************************************************************/

BYTE getvar(BYTE nv)
{
	return var[nv];
}


/****************************************************************************
	VV_IMPS2: imprime una cadena en una ventana. Las palabras de final de
	  l°nea que no caben dentro de la ventana se pasan a la l°nea
	  siguiente. Esta versi¢n de vv_imps tiene en cuenta el car†cter de
	  subrayado que puede aparecer en los mensajes de la base de datos y
	  lo sustituye por el objeto actual.
	  Entrada:      's' cadena a imprimir
			'v' puntero a ventana
****************************************************************************/



void vv_imps2(char *s, STC_VV *v)
{
char b[MAX_PAL];                /* buffer para guardar palabras */
int i, cuenta=0, anch=0;
char *pto;

while(1) {
	/* si se encontr¢ fin de frase, espacio o avance de l°nea */
	if(!*s || (*s==' ') || (*s=='\n')) {
		/* si no hay palabra almacenada */
		if(!cuenta && *s) {
			vv_impc(*s,v);
			s++;
		}
		else {
			/* incluye espacio en palabra */
			if(*s==' ') {
				anch+=chr_anchura(*s);
				b[cuenta]=*s++;
				cuenta++;
			}
			/* si la palabra almacenada no cabe en lo que */
			/* queda de l°nea pero cabe en la siguiente, */
			/* la imprime en la siguiente l°nea, si no la */
			/* imprime sin m†s */
			if((anch>((v->lxi*8)-v->cvx)) &&
			  (anch<=(v->lxi*8))) vv_impc('\n',v);
			for(i=0; i<cuenta; i++) vv_impc(b[i],v);
			cuenta=0;
			anch=0;
			/* si fin de frase, sale */
			if(!*s) break;
		}
	}
	else {
		if(*s!='_') {
			/* si letra no es espacio ni avance de l°nea */
			/* la guarda */
			anch+=chr_anchura(*s);
			b[cuenta]=*s++;
			cuenta++;
			/* si se llena buffer */
			if(cuenta==MAX_PAL) {
				/* imprime lo que hay almacenado */
				for(i=0; i<MAX_PAL; i++) vv_impc(b[i],v);
				cuenta=0;
				anch=0;
				/* imprime resto de palabra */
				while(*s && (*s!=' ') && (*s!='\n')) {
					vv_impc(*s,v);
					s++;
				}
			}
		}
		/* si encontr¢ s°mbolo de subrayado */
		else {
			/* se salta el s°mbolo de subrayado */
			s++;
			/* puntero a texto objeto */
			pto=tab_obj+tab_desp_obj[getvar(8)]+6;
			while(*pto) {
				/* almacena texto objeto */
				anch+=chr_anchura(*pto);
				b[cuenta]=*pto++;
				cuenta++;
				 /* si se llena el buffer */
				if(cuenta==MAX_PAL) {
					/* imprime lo que hay almacenado */
					for(i=0; i<MAX_PAL; i++)
					  vv_impc(b[i],v);
					cuenta=0;
					anch=0;
					/* imprime resto de pal. de objeto */
					while(*pto && (*pto!=' ') &&
					  (*pto!='\n')) {
						vv_impc(*pto,v);
						pto++;
					}
				}
			}
		}
	}
}

}

/****************************************************************************
	VV_INPUTT: permite introducir por teclado una l°nea de caracteres con
	  tiempo l°mite de introducci¢n.
	  S¢lo admite los caracteres que contenga la cadena 'Cod_Teclas'
	  (global).
	  Entrada:      'plin' puntero a buffer d¢nde se almacenar† la
			cadena introducida
			'maxlong' m†ximo n£mero de caracteres permitidos;
			deber† ser menor o igual a VVINP_MAXLIN
			'cursor' car†cter a usar como cursor
			'conver' indica si la cadena tecleada debe ser
			convertida a may£sculas (VVINP_CONV para convertir,
			VVINP_NOCONV para dejar como se tecle¢)
			'v' puntero a ventana
			'tiempo' tiempo l°mite (segundos) para teclear
			la frase, si es 0 no habr† tiempo l°mite
			'modot' modo de funcionamiento del temporizado;
			si es 1 el tiempo se cuenta hasta que teclea la
			primera letra, si es 0 tiene el tiempo dado para
			teclear toda la frase
	  Salida:       c¢digo de tecla de funci¢n pulsada, 0 si no se
			puls¢ ninguna, 1 si se acab¢ por falta de tiempo
****************************************************************************/
unsigned vv_inputt(char *plin, unsigned maxlong, char cursor, int conver,
  STC_VV *v, int tiempo, int modot)
{
STC_RATONG r;
static char buf_lin[VVINP_MAXLIN+1]="";
long tinicio, tactual;
unsigned c, long_lin;
BYTE color;
char *pcur, *pfinl, *i;
int antx, anty, antlxi, curx, cury, primer_car=1, segundos;

/* guarda posici¢n del cursor y habilita scroll de ventana */
antx=v->cvx;
anty=v->cvy;
v->scroll=1;

/* modifica anchura interna de ventana para evitar problemas con cursor */
antlxi=v->lxi;
v->lxi-=2;

/* puntero de la posici¢n del cursor */
pcur=plin;

/* puntero al final de la l°nea */
pfinl=plin;
long_lin=1;

/* marca final de l°nea */
*pfinl='\0';

/* imprime cursor */
curx=(v->vxi*8)+v->cvx;
cury=(v->vyi*v->chralt)+v->cvy;
rg_puntero(RG_OCULTA);
imp_chr_pos(curx,cury);
imp_chr(cursor,0xff,0,CHR_AND);
imp_chr_pos(curx,cury);
imp_chr(cursor,0,v->colort,CHR_XOR);
rg_puntero(RG_MUESTRA);

/* coge contador de reloj actual */
tinicio=biostime(0,0L);

do {
	/* si se cuenta tiempo total para teclear o se cuenta hasta primer */
	/* car†cter tecleado y es el primer car†cter, y adem†s el tiempo */
	/* l°mite no es 0 */
	if(((modot==0) || ((modot==1) && primer_car)) && tiempo) {
		/* hasta que pulse una tecla mira el tiempo que ha pasado */
		do {
			/* coge contador de reloj actual */
			tactual=biostime(0,0L);
			segundos=(int)((tactual-tinicio)/TICKS_SEG);

			/* si se ha pasado el tiempo, sale indic†ndolo */
			if(segundos>tiempo) {
				/* reimprime la l°nea de input para borrar */
				/* el cursor */
				/* posiciona el cursor para imprimir */
				if(v->scroll!=1)
				  v->cvy=anty-((v->scroll-1)*v->chralt);
				else v->cvy=anty;
				v->cvx=antx;

				i=plin;
				do {
					if(*i) vv_impc(*i,v);
					i++;
				} while(i<=pfinl);
				/* a?ade espacio al final para compensar la */
				/* desaparici¢n del cursor */
				vv_impc('\xff',v);

				/* copia l°nea en buffer */
				for(pcur=plin, i=buf_lin; *pcur; pcur++, i++) {
					*i=*pcur;
				}
				*i='\0';

				if(conver==VVINP_CONV) {
					/* convierte l°nea a may£sculas */
					i=plin;
					while(*i) {
						*i=mayuscula(*i);
						i++;
					}
				}

				/* si no se produjo scroll en ventana */
				/* mientras entrada de datos y la posici¢n */
				/* del cursor es menor que la £ltima l°nea */
				/* de la ventana desactiva indicador de */
				/* scroll */
				if((v->scroll==1) &&
				  (v->cvy<((v->lyi-1)*v->chralt))) v->scroll=0;
				v->lxi=antlxi;
				return(1);
			}
			rg_estado(&r);
		} while(!bioskey(1) && !r.boton1 && !r.boton2);
	}

	/* indica que ha tecleado ya el primer car†cter */
	primer_car=0;

	do {
		c=vv_lee_tecla();
	} while((c==COD_RAT) && zero(10));
	switch(c) {
		case BACKSPACE :        /* borrar car†cter a izquierda */
			/* comprueba que cursor no estÇ a la izquierda */
			if(pcur!=plin) {
				pcur--;
				for(i=pcur; i<pfinl; i++) *i=*(i+1);
				pfinl--;
				long_lin--;
			}
			break;
		case COD_SUP :          /* borrar car†cter a derecha */
			/* comprueba que cursor no estÇ al final */
			if(pcur!=pfinl) {
				for(i=pcur; i<pfinl; i++) *i=*(i+1);
				pfinl--;
				long_lin--;
			}
			break;
		case COD_IZQ :          /* mover cursor izquierda */
			/* comprueba que no estÇ a la izquierda */
			if(pcur!=plin) pcur--;
			break;
		case COD_DER :          /* mover cursor derecha */
			/* comprueba que no estÇ a la derecha */
			if(pcur!=pfinl) pcur++;
			break;
		case COD_ARR :          /* mover cursor arriba */
			/* si se pasa del inicio, copia l°nea anterior */
			/* y sit£a cursor al final */
			if(pcur==plin) {
				/* borra l°nea anterior */
				if(v->scroll!=1)
				  v->cvy=anty-((v->scroll-1)*v->chralt);
				else v->cvy=anty;
				v->cvx=antx;
				/* imprime con color de fondo */
				color=v->colort;
				v->colort=v->colortf;
				for(i=plin; *i; i++) vv_impc(*i,v);
				v->colort=color;

				/* por si cursor est† al final */
				vv_impc('\xff',v);

				pcur=plin;
				for(i=buf_lin, long_lin=1; *i; i++, pcur++,
				  long_lin++) *pcur=*i;
				pfinl=pcur;
				*pfinl='\0';
			}
			else {
				/* resta el ancho de la ventana */
				pcur-=v->lxi;
				if(pcur<plin) pcur=plin;
			}
			break;
		case COD_ABJ :          /* mover cursor abajo */
			/* suma el ancho de la ventana */
			pcur+=v->lxi;
			/* si se pasa del final, lo coloca al final */
			if(pcur>pfinl) pcur=pfinl;
			break;
		case COD_ORG :          /* mover cursor al inicio */
			pcur=plin;
			break;
		case COD_FIN :          /* mover cursor al final */
			pcur=pfinl;
			break;
		/* si pulsa una tecla de funci¢n o RETURN */
		/* acaba la entrada y devuelve su c¢digo (tecla funci¢n) */
		case RETURN :
		case COD_RAT :
		case COD_F1 :
		case COD_F2 :
		case COD_F3 :
		case COD_F4 :
		case COD_F5 :
		case COD_F6 :
		case COD_F7 :
		case COD_F8 :
		case COD_F9 :
		case COD_F10 :
			break;
		default :
			/* inserta car†cter si no se excede longitud m†xima */
			if(long_lin==maxlong) break;
			if(pcur!=pfinl) for(i=pfinl; i>pcur; i--) *i=*(i-1);
			*pcur++=(char)c;
			*(++pfinl)='\0';
			long_lin++;
			break;
	}
	/* si al teclear algo hubo scroll en ventana calcula l°nea */
	/* de inicio del cursor de acuerdo al n£mero de l°neas */
	/* 'scrolleadas' (v->scroll), sino coge la antigua posici¢n */
	/* del cursor 'anty' */
	if(v->scroll!=1) v->cvy=anty-((v->scroll-1)*v->chralt);
	else v->cvy=anty;
	v->cvx=antx;

	i=plin;
	do {
		curx=v->cvx+(v->vxi*8);
		cury=v->cvy+(v->vyi*v->chralt);

		/* imprime car†cter */
		if(*i) {
			g_rectangulo(curx,cury,curx+15,cury+v->chralt-1,
			  v->colortf,G_NORM,1);
			vv_impc(*i,v);
		}

		/* si es posici¢n de cursor y no est† al final de */
		/* la l°nea, lo imprime */
		if((i==pcur) && (pcur!=pfinl)) {
			/* si ha cambiado de l°nea */
			if(cury!=(v->cvy+(v->vyi*v->chralt))) {
				curx=v->cvx+(v->vxi*8)-chr_anchura(*i);
				cury=v->cvy+(v->vyi*v->chralt);
			}
			rg_puntero(RG_OCULTA);
			/* enmascara zona */
			imp_chr_pos(curx,cury);
			imp_chr(*i,0xff,0,CHR_AND);
			imp_chr_pos(curx,cury);
			imp_chr(cursor,0xff,0,CHR_AND);
			/* imprime car†cter */
			imp_chr_pos(curx,cury);
			imp_chr(*i,0,v->colort,CHR_OR);
			/* imprime cursor sobre car†cter */
			imp_chr_pos(curx,cury);
			imp_chr(cursor,0,v->colort,CHR_XOR);
			rg_puntero(RG_MUESTRA);
		}

		/* siguiente car†cter */
		i++;
	} while(i<pfinl);

	/* si cursor al final lo imprime ahora */
	if(pcur==pfinl) {
		curx=v->cvx+(v->vxi*8);
		cury=v->cvy+(v->vyi*v->chralt);
		g_rectangulo(curx,cury,curx+15,cury+v->chralt-1,v->colortf,
		  G_NORM,1);
		vv_impc(cursor,v);
	}

	/* imprime un espacio al final (por si se borr¢ un car†cter) */
	vv_impc('\xff',v);
	vv_impc('\xff',v);

} while((c!=RETURN) && (c!=COD_RAT) && (c<COD_F10));

/* reimprime la l°nea de input para borrar el cursor */
/* posiciona el cursor para imprimir */
if(v->scroll!=1) v->cvy=anty-((v->scroll-1)*v->chralt);
else v->cvy=anty;
v->cvx=antx;

i=plin;
do {
	if(*i) vv_impc(*i,v);
	i++;
} while(i<=pfinl);
/* a?ade espacio al final para compensar la desaparici¢n del cursor */
vv_impc('\xff',v);

/* copia l°nea en buffer */
for(pcur=plin, i=buf_lin; *pcur; pcur++, i++) *i=*pcur;
*i='\0';

if(conver==VVINP_CONV) {
	/* convierte l°nea a may£sculas */
	i=plin;
	while(*i) {
		*i=mayuscula(*i);
		i++;
	}
}

/* si no se produjo scroll en ventana mientras entrada de datos */
/* y la posici¢n del cursor es menor que la £ltima l°nea de la ventana */
/* desactiva indicador de scroll */
if((v->scroll==1) && (v->cvy<((v->lyi-1)*v->chralt))) v->scroll=0;

v->lxi=antlxi;

/* si termin¢ la l°nea pulsando RETURN devuelve 0, si no devuelve */
/* el c¢digo de la tecla (o bot¢n del rat¢n) con que se termin¢ */
if(c==RETURN) return(0);
else return(c);

}

/****************************************************************************
	ESTA_EN: comprueba si un car†cter est† en una cadena.
	  Entrada:      's' cadena con la que se comprobar† el car†cter
			'c' car†cter a comprobar
	  Salida:       TRUE si el car†cter est† en la cadena
			FALSE si no
****************************************************************************/
BOOLEAN esta_en(char *s, char c)
{

while(*s) {
	if(*s++==c) return(TRUE);
}

return(FALSE);
}

/****************************************************************************
	ESTA_EN_VOC: comprueba si una palabra est† en el vocabulario.
	  Entrada:      'vocab' puntero a tabla de vocabulario
			'pal_voc' n£mero de palabras en vocabulario
			'pal' puntero a palabra a buscar
	  Salida:       posici¢n dentro del vocabulario si se encontr¢, si no
			devuelve (NUM_PAL+1) que puede considerarse como
			palabra no encontrada
****************************************************************************/
int esta_en_voc(char *pal)
{
int i;

for(i=0; i<cab.pal_voc; i++) {
	if(!strcmp(pal,vocabulario[i].p)) return(i);
}

return(NUM_PAL+1);
}

/****************************************************************************
	ANALIZA: analiza una palabra.
	  < !???>   caracteres no significativos
	  <.,;:"'>  separadores
	  Entrada:      'pfrase' puntero a puntero a frase a analizar
	  Salida:       'tipo', 'num' tipo y n£mero de la palabra en vocabul.
			SEPARADOR si encontr¢ un separador
			FIN_FRASE si encontr¢ final de la frase
			PALABRA si encontr¢ palabra v†lida de vocabulario
			TERMINACION como PALABRA pero si adem†s encontr¢ una
			terminaci¢n en LA, LE o LO
			NO_PAL si encontr¢ palabra pero no est† en
			vocabulario
****************************************************************************/
int analiza(char *(*pfrase), BYTE *tipo, BYTE *num)
{
int i=0;
char palabra[LONGPAL+1];

/* salta caracteres no significativos */
for(; esta_en(C_No_Sig,*(*pfrase)); (*pfrase)++);

/* si es un separador lo salta */
if(esta_en(C_Separ,*(*pfrase))) {
	(*pfrase)++;
	/* salta caracteres no significativos */
	for(; esta_en(C_No_Sig,*(*pfrase)); (*pfrase)++);

	/* si hay otro separador detr†s lo salta tambiÇn */
	while(esta_en(C_Separ,*(*pfrase))) {
		(*pfrase)++;
		/* salta caracteres no significativos */
		for(; esta_en(C_No_Sig,*(*pfrase)); (*pfrase)++);
	}
	/* si detr†s encuentra final de la frase, sale indic†ndolo */
	/* si no, sale indicando que encontr¢ separador */
	if(!*(*pfrase)) return(FIN_FRASE);
	else return(SEPARADOR);
}

/* si encontr¢ fin de frase, lo indica */
if(!*(*pfrase)) return(FIN_FRASE);

/* salta espacios anteriores a palabra */
for(; *(*pfrase)==' '; (*pfrase)++);
/* al llegar aqu° *(*pfrase) ser† [A-Z]+ù+[0-9] */
/* repite mientras no llene palabra y encuentre car†cter v†lido de palabra */
do {
	/* mete car†cter en palabra */
	palabra[i]=*(*pfrase);
	(*pfrase)++;
	i++;
} while((i<LONGPAL) && (esta_en(Car_Pal,*(*pfrase))));

/* rellena con espacios y marca fin de palabra */
for(; i<LONGPAL; i++) palabra[i]=' ';
palabra[i]='\0';

/* desprecia resto de palabra */
while(esta_en(Car_Pal,*(*pfrase))) (*pfrase)++;

/* comprobamos si la palabra est† en vocabulario */
*num=0;
*tipo=0;
if((i=esta_en_voc(palabra))==NUM_PAL+1) return(NO_PALABRA);

/* si est† en vocabulario coge su n£mero y su tipo */
*num=vocabulario[i].num;
*tipo=vocabulario[i].tipo;

/* si es una conjunci¢n indica que encontr¢ un separador */
if(*tipo==_CONJ) return(SEPARADOR);

/* si termina en LA, LE o LO indica palabra con terminaci¢n */
if((*((*pfrase)-2)=='L') && ((*((*pfrase)-1)=='A') || (*((*pfrase)-1)=='E') ||
  (*((*pfrase)-1)=='O'))) return(TERMINACION);
/* lo mismo si termina en LAS, LES o LOS */
else if((*((*pfrase)-1)=='S') && (*((*pfrase)-3)=='L') &&
  ((*((*pfrase)-1)=='A') || (*((*pfrase)-1)=='E') || (*((*pfrase)-1)=='O')))
  return(TERMINACION);
/* en cualquier otro caso indica que encontr¢ palabra */
else return(PALABRA);

}

/****************************************************************************
	PARSE1: analiza la l°nea principal de entrada hasta un separador o
	  hasta el fin de l°nea.
	  Entrada:      'frase' conteniendo la l°nea a analizar
	  Salida:       TRUE si no hay m†s que analizar
			FALSE si queda texto por analizar
			Variables 2 a 6 actualizadas convenientemente
****************************************************************************/
BOOLEAN parse1(void)
{
BYTE num, tipo;
BYTE nombrelo, adjtlo, verbo;
int res, f[3], i;

/* inicializa banderas de palabras */
for(i=0; i<3; i++) f[i]=0;

nombrelo=getvar(3);        /* guarda nombre para verbo con terminaci¢n */
adjtlo=getvar(4);          /* guarda adjetivo para verbo con terminaci¢n */
verbo=getvar(2);           /* guarda verbo por si teclea frase sin Çl */

/* inicializa variables de sentencia l¢gica */
for(i=2; i<7; i++) setvar(i,NO_PAL);

/* repite hasta fin de frase o separador */
do {
	/* analiza una palabra */
	res=analiza(&lin,&tipo,&num);

	/* si es palabra sin terminaci¢n */
	if(res==PALABRA) {
		/* si es verbo y no cogi¢ ninguno, lo almacena */
		if((tipo==_VERB) && f[_VERB]!=1) {
			setvar(2,num);
			f[_VERB]=1;
		}
		/* s¢lo coge los dos primeros nombres */
		if((tipo==_NOMB) && (f[_NOMB]<2)) {
			/* almacena nombre 1 en variable 3 */
			if(!f[_NOMB]) {
				setvar(3,num);
				/* si es nombre convertible tambiÇn lo */
				/* almacena en verbo si no cogi¢ antes */
				/* ning£n verbo ni otro nombre conv. */
				if((num<cab.n_conv) && !f[_VERB]) {
					setvar(2,num);
					/* indica que hay nombre convert. */
					f[_VERB]=2;
					/* luego lo incrementar† */
					f[_NOMB]--;
				}
			}
			/* almacena nombre 2 en variable 5 */
			if(f[_NOMB]==1) setvar(5,num);
			/* incrementa n£mero de nombres cogidos */
			f[_NOMB]++;
		}
		/* s¢lo coge los dos primeros adjetivos */
		if((tipo==_ADJT) && (f[_ADJT]<2)) {
			/* almacena adjetivo 1 en variable 4 */
			if(!f[_ADJT]) setvar(4,num);
			/* almacena adjetivo 2 en variable 6 */
			if(f[_ADJT]==1) setvar(6,num);
			/* incrementa n£mero de adjetivos cogidos */
			f[_ADJT]++;
		}
	}
	/* si es palabra con terminaci¢n */
	if(res==TERMINACION) {
		if((tipo==_VERB) && f[_VERB]!=1) {
			setvar(2,num); /* almacena n£mero verbo en variable 2 */
			f[_VERB]=1; /* indica que ya ha cogido el verbo */
			/* si nombre anterior no era Propio */
			/* recupera el nombre anterior y su adjetivo */
			if(nombrelo>=cab.n_prop) {
				setvar(3,nombrelo);
				setvar(4,adjtlo);
				f[_NOMB]++;
				f[_ADJT]++;
			}
		}
		/* s¢lo coge los dos primeros nombres */
		if((tipo==_NOMB) && (f[_NOMB]<2)) {
			/* almacena nombre 1 en variable 3 */
			if(!f[_NOMB]) {
				setvar(3,num);
				/* si es nombre convertible tambiÇn lo */
				/* almacena en verbo si no cogi¢ antes */
				/* ning£n verbo ni otro nombre conv. */
				if((num<cab.n_conv) && !f[_VERB]) {
					setvar(2,num);
					/* indica que hay nombre convertible */
					f[_VERB]=2;
					/* luego lo incrementar† */
					f[_NOMB]--;
				}
			}
			/* almacena nombre 2 en variable 5 */
			if(f[_NOMB]==1) setvar(5,num);
			/* incrementa n£mero de nombres cogidos */
			f[_NOMB]++;
		}
		/* s¢lo coge los dos primeros adjetivos */
		if((tipo==_ADJT) && (f[_ADJT]<2)) {
			/* almacena adjetivo 1 en variable 4 */
			if(!f[_ADJT]) setvar(4,num);
			/* almacena adjetivo 2 en variable 6 */
			if(f[_ADJT]==1) setvar(6,num);
			/* incrementa n£mero de adjetivos cogidos */
			f[_ADJT]++;
		}
	}
} while((res!=FIN_FRASE) && (res!=SEPARADOR));

/* si tecle¢ una frase sin verbo pero con nombre, pone el verbo anterior */
if(!f[_VERB] && f[_NOMB]) setvar(2,verbo);

/* si es fin de frase mira si analiz¢ o no lo £ltimo cogido */
if(res==FIN_FRASE) {
	if(mas_texto==FALSE) {
		mas_texto=TRUE;         /* indicador para siguiente llamada */
		return(FALSE);          /* indica que analice lo £ltimo */
	}
	else return(TRUE);              /* no queda m†s por analizar */
}

/* si es separador, supone que hay m†s texto detr†s */
return(FALSE);
}

/****************************************************************************
	PAUSA: realiza una pausa. Si se pulsa una tecla, sale de la pausa,
	  pero no saca la tecla del buffer de teclado.
	  Entrada:      'p' tiempo de la pausa en dÇcimas de segundo,
			si es 0 se espera a que pulse una tecla
****************************************************************************/
void pausa(clock_t p)
{
STC_RATONG r;
clock_t t1, t2;

/* si pausa es 0, espera que se pulse una tecla */
if(!p) do {
	rg_estado(&r);
} while(!bioskey(1) && !r.boton1 && !r.boton2);

t1=p+(clock()/CLK_TCK*10);
do {
	t2=clock()/CLK_TCK*10;
	rg_estado(&r);
} while((t2<t1) && !bioskey(1) && !r.boton1 && !r.boton2);

}

/****************************************************************************
	CARGA_DEF: carga de un fichero las definiciones de los caracteres y
	  actualiza el generador de caracteres.
	  Entrada:      'nombre' nombre del fichero
	  Salida:       0 si hubo error o un valor distinto de 0 en
			otro caso
****************************************************************************/
int carga_def(char *nombre)
{
FILE *ffuente;
char cad_recon[LONG_RECON_F+1];
char *recon_fuente=RECON_FUENTE;

/* abre el fichero para lectura */
if((ffuente=fopen(nombre,"rb"))==NULL) return(0);

/* lee cadena de reconocimiento */
if(fread(cad_recon,sizeof(char),LONG_RECON_F+1,ffuente)<LONG_RECON_F+1) {
	fclose(ffuente);
	return(0);
}

/* comprueba la versi¢n del fichero */
if(cad_recon[LONG_RECON_F-1]!=recon_fuente[LONG_RECON_F-1]) {
	fclose(ffuente);
	return(0);
}

/* si la versi¢n ha sido v†lida lee las definiciones de los caracteres */
if(fread(&anchdef8x16[0],sizeof(unsigned char),256,ffuente)<256) {
	fclose(ffuente);
	return(0);
}
if(fread(&cardef8x16[0][0],sizeof(unsigned char),256*16,ffuente)<(256*16)) {
	fclose(ffuente);
	return(0);
}
if(fread(&anchdef8x8[0],sizeof(unsigned char),256,ffuente)<256) {
	fclose(ffuente);
	return(0);
}
if(fread(&cardef8x8[0][0],sizeof(unsigned char),256*8,ffuente)<(256*8)) {
	fclose(ffuente);
	return(0);
}

/* actualiza punteros a las fuentes de caracteres cargadas */
def_chr_set(&cardef8x16[0][0],&cardef8x8[0][0],&anchdef8x16[0],&anchdef8x8[0]);

fclose(ffuente);

return(1);
}

/****************************************************************************
	CARGA_TABLA_MES: carga una tabla de mensajes.
	  Entrada:      'nt' n£mero de tabla a cargar
	  Salida:       0 si tabla no existe o hubo error, un valor distinto
			de 0 en otro caso
****************************************************************************/
int carga_tabla_mes(BYTE nt)
{
FILE *fbd;

/* si tabla no existe, sale */
if(cab.fpos_msg[nt]==(fpos_t)0) return(0);

/* si tabla ya est† cargada, sale */
/* G3.30: lo quitamos a ver si se quita el "bug" de los mensajes descontrolados */
/*** if(nt==tabla_msg) return(1); */

/* carga tabla de mensajes */
if((fbd=fopen(nf_base_datos,"rb"))==NULL) return(0);

#if RUNTIME==0
fseek(fbd,cab.fpos_msg[nt],SEEK_SET);
#else
fseek(fbd,cab.fpos_msg[nt]+lng_runtime,SEEK_SET);
#endif
if(fread(tab_desp_msg,sizeof(unsigned),(size_t)MAX_MSG,fbd)!=MAX_MSG) {
	fclose(fbd);
	return(0);
}
if(fread(tab_msg,sizeof(char),cab.bytes_msg[nt],fbd)!=cab.bytes_msg[nt]) {
	fclose(fbd);
	return(0);
}

fclose(fbd);

codifica((BYTE *)tab_msg,cab.bytes_msg[nt]);

tabla_msg=nt;
setvar(17,nt);

return(1);
}

/*-------------------------------------------------------------------------*/
/* FUNCIONES PARA LOS CONDACTOS                                            */
/*-------------------------------------------------------------------------*/

/****************************************************************************
	PROCESS: ejecuta una llamada a un proceso.
	  Entrada:      'pro' n£mero de proceso
****************************************************************************/
BOOLEAN process(BYTE prc)
{

/* si se rebasa la capacidad de pila interna */
if(ptrp==STK) m_err(3,"Rebosamiento de la pila interna",1);

pila1[ptrp]=ptr_proc+2; /* guarda dir. sgte. condacto en proc. actual */
pila2[ptrp]=sgte_ent;   /*   "    desplazamiento de sgte. entrada */
pila3[ptrp]=pro_act;    /*   "    n£mero de proceso actual */
ptrp++;                 /* incrementa puntero de pila */
pro_act=prc;

/* direcci¢n del proceso llamado - 2 (que se sumar†) */
ptr_proc=tab_pro+tab_desp_pro[pro_act]-2;

/* indica que no debe ajustar ptr_proc para siguiente entrada */
nueva_ent=FALSE;

/* saltar† a inicio nueva entrada (la primera del proceso llamado) */
return(FALSE);
}

/****************************************************************************
	DONE: finaliza un proceso y continua en siguiente condacto.
****************************************************************************/
BOOLEAN done(void)
{

if(!ptrp) m_err(0,"",1);
ptrp--;
/* recupera dir. sgte condacto en proc. que llam¢ - 1 (que se sumar† luego) */
ptr_proc=pila1[ptrp]-1;

sgte_ent=pila2[ptrp];   /* desplazamiento de la siguiente entrada */
pro_act=pila3[ptrp];    /* n£mero de proceso que llam¢ */

/* indica que no debe ajustar ptr_proc para siguiente entrada */
nueva_ent=FALSE;

return(TRUE);
}

/****************************************************************************
	NOTDONE: finaliza un proceso y salta a siguiente entrada.
****************************************************************************/
BOOLEAN notdone(void)
{

done();                 /* ejecuta un DONE */
nueva_ent=TRUE;         /* para que ajuste a siguiente entrada */

return(FALSE);
}

/****************************************************************************
	RESP: activa comprobaci¢n de verbo-nombre al inicio de cada entrada.
****************************************************************************/
BOOLEAN resp(void)
{

resp_act=TRUE;

return(TRUE);
}

/****************************************************************************
	NORESP: desactiva comprobaci¢n de verbo-nombre.
****************************************************************************/
BOOLEAN noresp(void)
{

resp_act=FALSE;

return(TRUE);
}

/****************************************************************************
	DEFWIN: define una ventana.
	  Entrada:      'nw' n£mero de ventana
			'cwf, 'cw' colores de fondo y primer plano
			'wy', 'wx' posici¢n de esquina superior izquierda
			(fila, columna) de la ventana; si 255 se centra en
			pantalla
			'lx', 'ly' tama?o (ancho y alto) de la ventana; si
			255 se toman las m†ximas dimensiones
****************************************************************************/
BOOLEAN defwin(BYTE nw, BYTE cwf, BYTE cw, BYTE wy, BYTE wx, BYTE lx, BYTE ly)
{

/* ajusta dimensiones para que ocupe toda la pantalla */
if(lx==255) lx=getvar(14);
if(ly==255) ly=getvar(15);

/* ajusta posici¢n para centrar ventana */
if(wx==255) wx=(BYTE)((getvar(14)-lx)/2);
if(wy==255) wy=(BYTE)((getvar(15)-ly)/2);

/* ajusta para que entre en pantalla */
if(wx>(BYTE)(getvar(14)-1)) wx=(BYTE)(getvar(14)-1);
if(wy>(BYTE)(getvar(15)-1)) wy=(BYTE)(getvar(15)-1);
if((wx+lx)>getvar(14)) lx=getvar(14)-wx;
if((wy+ly)>getvar(15)) ly=getvar(15)-wy;

vv_crea(wy,wx,lx,ly,cwf,cw,NO_BORDE,&w[nw]);

return(TRUE);
}

/****************************************************************************
	WINDOW: selecciona la ventana activa.
	  Entrada:      'nw' n£mero de ventana
****************************************************************************/
BOOLEAN window(BYTE nw)
{

setvar(0,nw);

return(TRUE);
}

/****************************************************************************
	CLW: borra/inicializa una ventana.
	  Entrada:      'nw' n£mero de ventana
****************************************************************************/
BOOLEAN clw(BYTE nw)
{

vv_cls(&w[nw]);

return(TRUE);
}

/****************************************************************************
	LET: asigna un valor a una variable.
	  Entrada:      'nv' n£mero de variable
			'val' valor a asignar
****************************************************************************/
BOOLEAN let(BYTE nv, BYTE val)
{

setvar(nv,val);

return(TRUE);
}

/****************************************************************************
	EQ: comprueba si una variable es igual a un valor.
	  Entrada:      'nv' n£mero de variable
			'val' valor
	  Salida:       TRUE si la variable es igual al valor
			FALSE si es distinta de valor
****************************************************************************/
BOOLEAN eq(BYTE nv, BYTE val)
{

if(getvar(nv)==val) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTEQ: comprueba si una variable distinta de un valor.
	  Entrada:      'nv' n£mero de variable
			'val' valor
	  Salida:       TRUE si la variable es distinta del valor
			FALSE si es igual al valor
****************************************************************************/
BOOLEAN noteq(BYTE nv, BYTE val)
{

return(TRUE-eq(nv,val));
}

/****************************************************************************
	LT: comprueba si una variable es menor que un valor.
	  Entrada:      'nv' n£mero de variable
			'val' valor
	  Salida:       TRUE si la variable es menor que valor
			FALSE si es mayor o igual que valor
****************************************************************************/
BOOLEAN lt(BYTE nv, BYTE val)
{

if(getvar(nv)<val) return(TRUE);

return(FALSE);
}

/****************************************************************************
	GT: comprueba si una variable es mayor que un valor.
	  Entrada:      'nv' n£mero de variable
			'val' valor
	  Salida:       TRUE si la variable es mayor que valor
			FALSE si es menor o igual que valor
****************************************************************************/
BOOLEAN gt(BYTE nv, BYTE val)
{

if(getvar(nv)>val) return(TRUE);

return(FALSE);
}

/****************************************************************************
	MES: imprime un mensaje.
	  Entrada:      'nt' n£mero de tabla de mensajes
			'nm' n£mero de mensaje
****************************************************************************/
BOOLEAN mes(BYTE nt, BYTE nm)
{
char *pm;

/* si tabla es distinta a la que hay cargada, la carga */
if(nt!=getvar(17)) {
	if(!carga_tabla_mes(nt)) return(TRUE);
}

/* comprueba si mensaje existe en tabla, si no sale */
if(nm>=cab.num_msg[nt]) return(TRUE);

pm=tab_msg+tab_desp_msg[nm];    /* puntero a mensaje */
vv_imps2(pm,&w[getvar(0)]);        /* imprime mensaje en ventana activa */

return(TRUE);
}

/****************************************************************************
	NEWLINE: imprime un avance de l°nea.
****************************************************************************/
BOOLEAN newline(void)
{

vv_impc('\n',&w[getvar(0)]);

return(TRUE);
}

/****************************************************************************
	MESSAGE: imprime un mensaje con avance de l°nea.
	  Entrada:      'nt' n£mero de tabla de mensajes
			'nm' n£mero de mensaje
****************************************************************************/
BOOLEAN message(BYTE nt, BYTE nm)
{

mes(nt,nm);                     /* imprime mensaje */
newline();                      /* y avance de l°nea */

return(TRUE);
}

/****************************************************************************
	SYSMESS: imprime un mensaje del sistema.
	  Entrada:      'nm' n£mero de mensaje
****************************************************************************/
BOOLEAN sysmess(BYTE nm)
{
char *pm;

pm=tab_msy+tab_desp_msy[nm];    /* puntero a mensaje */
vv_imps2(pm,&w[getvar(0)]);        /* imprime mensaje en ventana activa */

return(TRUE);
}

/****************************************************************************
	DESC: imprime la descripci¢n de una localidad y salta al inicio del
	  Proceso 0. Si est† oscuro imprime el mensaje del sistema 23.
	  Entrada:      'nl' n£mero de localidad
****************************************************************************/
BOOLEAN desc(BYTE nl)
{
char *pl;

/* puntero a descripci¢n de localidad */
pl=tab_loc+tab_desp_loc[nl];

/* si no est† oscuro o hay una fuente de luz imprime descripci¢n */
if(zero(0) || light()) vv_imps2(pl,&w[getvar(0)]);
else sysmess(23);       /* 'Est† oscuro. No puedes ver nada.' */

/* indica que hay que inicializar: borrar ventana, listar objetos... */
clear(2);

restart();              /* salta al inicio de Proceso 0 */

/* -1 a ptr_proc (y -1 del restart), luego se sumar†n 2 */
ptr_proc--;

return(FALSE);
}

/****************************************************************************
	ADD: suma un valor a una variable.
	  Entrada:      'nv' n£mero de variable
			'val' valor a sumar
****************************************************************************/
BOOLEAN add(BYTE nv, BYTE val)
{

#ifdef DAAD
if (getvar(nv)+val <= MAX_VAL)
#endif
setvar(nv, getvar(nv)+val);
#ifdef DAAD
else setvar(nv, MAX_VAL)
#endif

return(TRUE);
}

/****************************************************************************
	SUB: resta un valor a una variable.
	  Entrada:      'nv' n£mero de variable
			'val' valor a restar
****************************************************************************/
BOOLEAN sub(BYTE nv, BYTE val)
{

#ifdef DAAD
if (getvar(nv)-val >= 0)
#endif
setvar(nv, getvar(nv)-val);
#ifdef DAAD
else setvar(nv, 0)
#endif

return(TRUE);
}

/****************************************************************************
	INC: incrementa en 1 el valor de una variable.
	  Entrada:      'nv' n£mero de variable
****************************************************************************/
BOOLEAN inc(BYTE nv)
{

add(nv, 1);

return(TRUE);
}

/****************************************************************************
	DEC: decrementa en 1 el valor de una variable.
	  Entrada:      'nv' n£mero de variable
****************************************************************************/
BOOLEAN dec(BYTE nv)
{

sub(nv,1);

return(TRUE);
}

/****************************************************************************
	SET: pone a 1 una bandera.
	  Entrada:      'nf' n£mero de bandera
****************************************************************************/
BOOLEAN set(BYTE nf)
{
#ifndef DAAD
BYTE mascara_set=0x80;
int nbyte, nbit;

nbyte=nf/8;                     /* n£mero de byte */
nbit=nf%8;                      /* bit dentro del byte */

mascara_set>>=nbit;
flag[nbyte]|=mascara_set;
#else
setvar(nf, MAX_VAL);
#endif

return(TRUE);
}


/****************************************************************************
	CLEAR: pone a 0 una bandera.
	  Entrada:      'nf' n£mero de bandera
****************************************************************************/
BOOLEAN clear(BYTE nf)
{
#ifndef DAAD
BYTE mascara_clr=0x80;
int nbyte, nbit;

nbyte=nf/8;                     /* n£mero de byte */
nbit=nf%8;                      /* bit dentro del byte */

mascara_clr>>=nbit;                     /* desplaza hacia derecha (a?ade 0) */
mascara_clr=(BYTE)0xff-mascara_clr;     /* complementa mascara_clr */
flag[nbyte]&=mascara_clr;
#else
setvar(nf,0);
#endif

return(TRUE);
}

/****************************************************************************
	ZERO: comprueba si una bandera es 0.
	  Entrada:      'nf' n£mero de bandera
	  Salida:       TRUE si la bandera es 0
			FALSE si la bandera es 1
****************************************************************************/
BOOLEAN zero(BYTE nf)
{
#ifndef DAAD
BYTE mascara=0x80;
int nbyte, nbit;

nbyte=nf/8;
nbit=nf%8;
mascara>>=nbit;

if(!(flag[nbyte] & mascara)) return(TRUE);
#else
if (getvar(nf)==0) return(TRUE);
#endif

return(FALSE);
}

/****************************************************************************
	NOTZERO: comprueba si una bandera es 1.
	  Entrada:      'nf' n£mero de bandera
	  Salida:       TRUE si la bandera es 1
			FALSE si la bandera es 0
****************************************************************************/
BOOLEAN notzero(BYTE nf)
{
return(TRUE-zero(nf));
}

/****************************************************************************
	PLACE: coloca un objeto en una localidad.
	  Entrada:      'nobj' n£mero de objeto
			'nloc' n£mero de localidad
****************************************************************************/
BOOLEAN place(BYTE nobj,BYTE nloc)
{
BYTE locobj;

/* coge la localidad actual del objeto */
locobj=loc_obj[nobj];

/* si se refiere a localidad actual sustituye por su valor */
if(nloc==LOC_ACTUAL) nloc=getvar(1);

/* si localidad actual de objeto es igual a la de destino, no hace nada */
if(nloc==locobj) return(TRUE);

/* si pasa un objeto a cogido o puesto y no estaba ni cogido ni puesto */
/* incrementa el n£mero de objetos que lleva */
if(((nloc==COGIDO) || (nloc==PUESTO)) && (locobj!=COGIDO) &&
  (locobj!=PUESTO)) objs_cogidos++;

/* si el objeto estaba cogido o puesto y no lo pasa a cogido ni a puesto */
/* decrementa el n£mero de objetos que lleva */
if(((locobj==COGIDO) || (locobj==PUESTO)) && (nloc!=COGIDO) &&
  (nloc!=PUESTO)) objs_cogidos--;

/* actualiza posici¢n del objeto */
loc_obj[nobj]=nloc;

return(TRUE);
}

/****************************************************************************
	GET: coge un objeto.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si se pudo coger el objeto
			FALSE si no se pudo coger
	  NOTA: la variable 8 contendr† el n£mero del objeto a la salida
****************************************************************************/
BOOLEAN get(BYTE nobj)
{

/* si el objeto no existe */
if(nobj>=cab.num_obj) {
	sysmess(1);             /* 'Aqu° no est† eso.' */
	return(FALSE);
}

/* coloca el n£mero del objeto en la variable 8 */
setvar(8,nobj);

/* si ya ten°a el objeto */
if(carried(nobj) || worn(nobj)) {
	sysmess(3);             /* 'Ya tienes eso.' */
	return(FALSE);          /* debe hacer un DONE */
}

/* si el objeto no est† presente */
if(loc_obj[nobj]!=getvar(1)) {
	sysmess(1);             /* 'Aqu° no est† eso.' */
	return(FALSE);          /* debe hacer un DONE */
}

/* si lleva muchas cosas */
if((objs_cogidos>=getvar(7)) && (getvar(7)!=0)) {
	sysmess(2);     /* 'No puedes coger _. Llevas demasiadas cosas.' */
	return(FALSE);  /* debe hacer un DONE */
}

/* coge el objeto */
place(nobj,COGIDO);
sysmess(0);             /* 'Has cogido _.' */

return(TRUE);
}

/****************************************************************************
	DROP: deja un objeto.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si se pudo dejar el objeto
			FALSE si no se pudo dejar
	  NOTA: la variable 8 contendr† el n£mero del objeto a la salida
****************************************************************************/
BOOLEAN drop(BYTE nobj)
{

/* si el objeto no existe */
if(nobj>=cab.num_obj) {
	sysmess(5);                     /* 'No tienes eso.' */
	return(FALSE);
}

/* coloca el n£mero del objeto en la variable 8 */
setvar(8,nobj);

/* si no cogido ni puesto */
if(notcarr(nobj) && notworn(nobj)) {
	sysmess(5);                     /* 'No tienes eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si lleva el objeto cogido o puesto, lo deja en loc. actual */
place(nobj,getvar(1));
sysmess(4);                             /* 'Dejas _.' */

return(TRUE);
}

/****************************************************************************
	INPUT: recoge texto del jugador.
	  Entrada:      variable 16 con el tiempo l°mite para teclear la
			frase, bandera 6 con el modo de temporizaci¢n
	  Salida:       TRUE si no tecle¢ nada o se pas¢ tiempo l°mite
			FALSE si tecle¢ algo
	  NOTA: la variable 12 contendr† el c¢digo de la tecla de funci¢n que
	  se puls¢ durante el INPUT
****************************************************************************/
BOOLEAN input(void)
{
int i, modo_temp;
unsigned k;

/* inicializa sentencia l¢gica */
for(i=2; i<7; i++) setvar(i,NO_PAL);

ini_inp=TRUE;           /* indica a parse() inicio de frase */
mas_texto=FALSE;        /* usada por parse1() para analizar £ltima frase */

/* modo de temporizaci¢n de INPUT */
if(zero(6)) modo_temp=0;
else modo_temp=1;

/* NOTA: el cursor ser† el primer car†cter del mensaje del sistema 7 */
k=vv_inputt(frase,MAXLONG,*(tab_msy+tab_desp_msy[7]),VVINP_CONV,&w[getvar(0)],
  (int)getvar(16),modo_temp);

/* guarda c¢digo con que termin¢ INPUT */
setvar(12,(BYTE)k);

/* si no tecle¢ nada o se pas¢ el tiempo l°mite */
if(!*frase || (getvar(12)==1)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	PARSE: analiza la frase tecleada por jugador.
	  Entrada:      variables globales.-
			  'frase' conteniendo frase tecleada
			  'ini_inp' TRUE si hay que analizar desde principio,
			  FALSE si sigue donde lo dej¢ en £ltima llamada
	  Salida:       TRUE si se analiz¢ toda la frase
			FALSE si queda m†s por analizar
****************************************************************************/
BOOLEAN parse(void)
{
BOOLEAN par;

/* si inicio de frase */
if(ini_inp==TRUE) {
	/* coloca puntero al principio de l°nea tecleada */
	lin=frase;
	ini_inp=FALSE;
}

/* analiza hasta separador o fin l°nea */
par=parse1();

return(par);
}

/****************************************************************************
	SKIP: salto relativo dentro de un proceso.
	  Entrada:      'lsb', 'hsb' bytes bajo y alto del desplazamiento del
			salto (-32768 a 32767)
****************************************************************************/
BOOLEAN skip(BYTE lsb, BYTE hsb)
{
int despli;

/* calcula desplazamiento */
despli=(hsb << 8) | lsb;

/* ptr_por - 3 que se sumar†n luego */
ptr_proc=ptr_proc+despli-3;

/* para que no ajuste ptr_proc a siguiente entrada */
nueva_ent=FALSE;

return(FALSE);          /* saltar† a inicio de entrada */
}

/****************************************************************************
	AT: comprueba si est† en una localidad.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       TRUE si est† en esa localidad
			FALSE si no est† en esa localidad
****************************************************************************/
BOOLEAN at(BYTE locno)
{

if(getvar(1)==locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTAT: comprueba que no est† en una localidad.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       TRUE si no est† en esa localidad
			FALSE si est† en esa localidad
****************************************************************************/
BOOLEAN notat(BYTE locno)
{

return(TRUE-at(locno));
}

/****************************************************************************
	ATGT: comprueba si est† en una localidad superior a la dada.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       TRUE si est† en una localidad superior
			FALSE si est† en una localidad inferior o igual
****************************************************************************/
BOOLEAN atgt(BYTE locno)
{

if(getvar(1)>locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ATLT: comprueba si est† en una localidad inferior a la dada.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       TRUE si est† en una localidad inferior
			FALSE si est† en una localidad superior o igual
****************************************************************************/
BOOLEAN atlt(BYTE locno)
{

if(getvar(1)<locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ADJECT1: comprueba el primer adjetivo de la sentencia l¢gica.
	  Entrada:      'adj' n£mero de adjetivo
	  Salida:       TRUE si el adjetivo 1 en la sentencia l¢gica (var(4))
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN adject1(BYTE adj)
{

if(getvar(4)==adj) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOUN2: comprueba el segundo nombre de la sentencia l¢gica.
	  Entrada:      'nomb' n£mero de nombre
	  Salida:       TRUE si el nombre 2 en la sentencia l¢gica (var(5))
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN noun2(BYTE nomb)
{

if(getvar(5)==nomb) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ADJECT2: comprueba el segundo adjetivo de la sentencia l¢gica.
	  Entrada:      'adj' n£mero de adjetivo
	  Salida:       TRUE si el adjetivo 2 en la sentencia l¢gica (var(6))
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN adject2(BYTE adj)
{

if(getvar(6)==adj) return(TRUE);

return(FALSE);
}

/****************************************************************************
	LISTAT: lista los objetos presentes en una localidad.
	  Entrada:      'locno' n£mero de localidad
			bandera 7 a 1 si se imprime mensaje 'nada' cuando no
			hay objetos que listar
****************************************************************************/
BOOLEAN listat(BYTE locno)
{
BYTE i, j=0;
char *po;
BYTE obl[MAX_OBJ];

/* si se trata de localidad actual sustituye por su valor */
if(locno==LOC_ACTUAL) locno=getvar(1);

/* recorre toda la tabla de objetos */
for(i=0; i<cab.num_obj; i++) {
	/* almacena n£meros de objetos en localidad especificada */
	if(loc_obj[i]==locno) {
		obl[j]=i;
		j++;
	}
}

/* si no se encontr¢ ning£n objeto */
if(!j && notzero(7)) sysmess(10);       /* ' nada.' */
else {
	/* recupera objetos guardados */
	for(i=0; i<j; i++) {
		/* puntero a texto objeto */
		po=tab_obj+tab_desp_obj[obl[i]]+6;

		/* si bandera 1 est† activada imprime en formato columna */
		if(notzero(1)) {
			vv_imps(po,&w[getvar(0)]);
			vv_impc('\n',&w[getvar(0)]);
		}
		else {
			/* imprime objeto */
			vv_imps(po,&w[getvar(0)]);
			/* si final de la lista */
			if(i==(BYTE)(j-1)) sysmess(13);
			/* uni¢n entre los dos £ltimos objetos de lista */
			else if(i==(BYTE)(j-2)) sysmess(12);
			/* separaci¢n entre objs. */
			else sysmess(11);
		}
	}
}

return(TRUE);
}

/****************************************************************************
	ISAT: comprueba si un objeto est† en una localidad.
	  Entrada:      'nobj' n£mero de objeto
			'locno' n£mero de localidad
	  Salida:       TRUE si el objeto est† en la localidad
			FALSE si no est† en la localidad
****************************************************************************/
BOOLEAN isat(BYTE nobj, BYTE locno)
{

/* si se trata de localidad actual sustituye por su valor */
if(locno==LOC_ACTUAL) locno=getvar(1);

if(loc_obj[nobj]==locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ISNOTAT: comprueba si un objeto no est† en una localidad.
	  Entrada:      'nobj' n£mero de objeto
			'locno' n£mero de localidad
	  Salida:       TRUE si el objeto no est† en la localidad
			FALSE si est† en la localidad
****************************************************************************/
BOOLEAN isnotat(BYTE nobj, BYTE locno)
{

return(TRUE-isat(nobj,locno));
}

/****************************************************************************
	PRESENT: comprueba si un objeto est† presente (en localidad actual,
	  cogido o puesto).
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto est† presente
			FALSE si no est† presente
****************************************************************************/
BOOLEAN present(BYTE nobj)
{

if(isat(nobj,LOC_ACTUAL) || isat(nobj,COGIDO) || isat(nobj,PUESTO))
  return(TRUE);

return(FALSE);
}

/****************************************************************************
	ABSENT: comprueba si un objeto no est† presente.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto no est† presente
			FALSE si est† presente
****************************************************************************/
BOOLEAN absent(BYTE nobj)
{

return(TRUE-present(nobj));
}

/****************************************************************************
	WORN: comprueba si un objeto est† puesto.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto est† puesto
			FALSE si no est† puesto
****************************************************************************/
BOOLEAN worn(BYTE nobj)
{

if(isat(nobj,PUESTO)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTWORN: comprueba si un objeto no est† puesto.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto no est† puesto
			FALSE si est† puesto
****************************************************************************/
BOOLEAN notworn(BYTE nobj)
{

return(TRUE-worn(nobj));
}

/****************************************************************************
	CARRIED: comprueba si un objeto est† cogido.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto est† cogido
			FALSE si no est† cogido
****************************************************************************/
BOOLEAN carried(BYTE nobj)
{

if(isat(nobj,COGIDO)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTCARR: comprueba si un objeto no est† cogido.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si el objeto no est† cogido
			FALSE si est† cogido
****************************************************************************/
BOOLEAN notcarr(BYTE nobj)
{

return(TRUE-carried(nobj));
}

/****************************************************************************
	WEAR: pone un objeto que sea una prenda.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si se pudo poner el objeto
			FALSE si no se pudo poner
	  NOTA: la variable 8 contendr† el n£mero del objeto a la salida
****************************************************************************/
BOOLEAN wear(BYTE nobj)
{
char *po;

/* si el objeto no existe */
if(nobj>=cab.num_obj) {
	sysmess(5);                     /* 'No tienes eso.' */
	return(FALSE);
}

/* coloca el n£mero del objeto en la variable 8 */
setvar(8,nobj);

/* puntero a banderas1 de objeto */
po=tab_obj+tab_desp_obj[nobj]+3;

/* si el objeto est† puesto */
if(worn(nobj)) {
	sysmess(16);                    /* 'Ya llevas puesto _.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si el objeto no est† aqu° */
if(absent(nobj)) {
	sysmess(1);                     /* 'Aqu° no est† eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si el objeto no est† cogido */
if(notcarr(nobj)) {
	sysmess(5);                     /* 'No tienes eso.' */
	return(FALSE);
}

/* si no es una prenda */
if(!(*po & 0x01)) {
	sysmess(17);                    /* 'No puedes ponerte _.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* se pone la prenda */
place(nobj,PUESTO);
sysmess(18);                            /* 'Te pones _.' */

return(TRUE);
}

/****************************************************************************
	REMOVE: quita un objeto que sea una prenda.
	  Entrada:      'nobj' n£mero de objeto
	  Salida:       TRUE si se pudo quitar el objeto
			FALSE si no se pudo quitar
	  NOTA: la variable 8 contendr† el n£mero del objeto a la salida
****************************************************************************/
BOOLEAN remove1(BYTE nobj)
{

/* si el objeto no existe */
if(nobj>=cab.num_obj) {
	sysmess(19);                    /* 'No llevas puesto eso.' */
	return(FALSE);
}

/* coloca el n£mero del objeto en la variable 8 */
setvar(8,nobj);

/* si no lo lleva puesto */
if(notworn(nobj)) {
	sysmess(19);                    /* 'No llevas puesto eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* pasa el objeto a cogido */
place(nobj,COGIDO);
sysmess(20);                            /* 'Te quitas _.' */

return(TRUE);
}

/****************************************************************************
	CREATE: pasa un objeto no creado a la localidad actual.
	  Entrada:      'nobj' n£mero de objeto
****************************************************************************/
BOOLEAN create(BYTE nobj)
{

if(isat(nobj,NO_CREADO)) place(nobj,LOC_ACTUAL);

return(TRUE);
}

/****************************************************************************
	DESTROY: pasa un objeto a no creado.
	  Entrada:      'nobj' n£mero de objeto
****************************************************************************/
BOOLEAN destroy(BYTE nobj)
{

place(nobj,NO_CREADO);

return(TRUE);
}

/****************************************************************************
	SWAP: intercambia dos objetos.
	  Entrada:      'nobj1' n£mero de objeto 1
			'nobj2' n£mero de objeto 2
****************************************************************************/
BOOLEAN swap(BYTE nobj1,BYTE nobj2)
{
BYTE locobj2;

locobj2=loc_obj[nobj2];         /* guarda localidad de 2? objeto */
place(nobj2,loc_obj[nobj1]);    /* pasa 2? a localidad del 1? */
place(nobj1,locobj2);           /* pasa 1? a localidad del 2? */

return(TRUE);
}

/****************************************************************************
	RESTART: salta al inicio del proceso 0.
****************************************************************************/
BOOLEAN restart(void)
{

/* inicializa puntero de pila */
ptrp=0;

/* proceso actual 0 */
pro_act=0;

/* puntero a inicio Proceso 0, -1 que luego se suma */
ptr_proc=tab_pro+tab_desp_pro[0]-1;

/* no debe ajustar ptr_proc para siguiente entrada */
nueva_ent=FALSE;

return(FALSE);
}

/****************************************************************************
	WHATO: devuelve el n£mero de objeto que se corresponde con el nombre1,
	  adjetivo1 de la sentencia l¢gica actual.
	  Entrada:      var(3) nombre
					var(4) adjetivo
	  Salida:       var(8) n£mero de objeto
****************************************************************************/
BOOLEAN whato(void)
{
BYTE i;
char *po;

/* inicializa a n£mero de objeto no v†lido */
#ifdef DAAD
setvar(8,MAX_VAL);
#else
setvar(8,255);
#endif

for(i=0; i<cab.num_obj; i++) {
	/* puntero a nombre-adjetivo objeto */
	po=tab_obj+tab_desp_obj[i];

	/* si encuentra el objeto, coloca su n£mero en var(8) */
	if((getvar(3)==(BYTE)*po) && ((getvar(4)==NO_PAL) ||
	  (getvar(4)==(BYTE)*(po+1)))) {
		setvar(8,i);
		break;
	}
}

return(TRUE);
}

/****************************************************************************
	MOVE: actualiza el contenido de una variable de acuerdo a su
	  contenido actual, a la sentencia l¢gica y a la tabla de conexiones,
	  para que contenga el n£mero de localidad con la que conecta una
	  dada por medio de la palabra de movimiento de la sentencia l¢gica.
	  Entrada:      'nv' n£mero de variable (cuyo contenido es el n£mero
			de una localidad v†lida)
			var(2) y var(3) sentencia l¢gica
	  Salida:       TRUE si hay conexi¢n y 'nv' modificada para que
			contenga un n£mero de localidad que conecta con la
			dada por medio de la tabla de conexiones
			FALSE si no hay conexi¢n en esa direcci¢n y 'nv' sin
			modificar
****************************************************************************/
BOOLEAN move(BYTE nv)
{
BYTE *pc;

/* puntero a conexiones de localidad var(nv) */
pc=tab_conx+tab_desp_conx[getvar(nv)];

/* mientras haya conexiones */
while(*pc) {
	if((getvar(2)==*pc && getvar(3)==NO_PAL) || (getvar(2)==NO_PAL && getvar(3)==*pc)
	  || (getvar(2)==*pc && getvar(3)==*pc)) {
		/* guarda n£mero localidad hacia la que conecta y sale */
		setvar(nv,*(pc+1));
		return(TRUE);
	}
	/* siguiente conexi¢n */
	pc+=2;
}

/* si no encontr¢ conexi¢n */
return(FALSE);
}

/****************************************************************************
	ISMOV: comprueba si la sentencia l¢gica es de movimiento
	  (movim. - NO_PAL, NO_PAL - movim. o movim. - movim.).
	  Entrada:      var(2) y var(3) con nombre-verbo
	  Salida:       TRUE si es sentencia l¢gica de movimiento
			FALSE si no lo es
****************************************************************************/
BOOLEAN ismov(void)
{

if((getvar(2)<cab.v_mov && getvar(3)==NO_PAL) ||
  (getvar(2)==NO_PAL && getvar(3)<cab.v_mov) ||
  (getvar(2)<cab.v_mov && getvar(3)<cab.v_mov)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	GOTO: va a una localidad especificada.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       var(1) conteniendo el n£mero de nueva localidad
			(si 'locno' es localidad no v†lida, el contenido de
			var(1) no se modifica)
****************************************************************************/
BOOLEAN goto1(BYTE locno)
{

if(locno<cab.num_loc) setvar(1,locno);

return(TRUE);
}

/****************************************************************************
	PRINT: imprime el contenido de una variable en la posici¢n actual.
	  Entrada:      'nv' n£mero de variable
****************************************************************************/
BOOLEAN print(BYTE nv)
{

vv_impn((unsigned)getvar(nv),&w[getvar(0)]);

return(TRUE);
}

/****************************************************************************
	DPRINT: imprime el contenido de dos variables consecutivas como un
	  n£mero de 16 bits en la posici¢n actual.
	  Entrada:      'nv' n£mero de la primera variable
	  NOTA: si nv=255 el resultado ser† impredecible
****************************************************************************/
BOOLEAN dprint(BYTE nv)
{
unsigned num;

num=(getvar(nv)*256)+getvar(nv+1);
vv_impn(num,&w[getvar(0)]);

return(TRUE);
}

/****************************************************************************
	CLS: borra la pantalla.
****************************************************************************/
BOOLEAN cls(void)
{

g_borra_pantalla();

return(TRUE);
}

/****************************************************************************
	ANYKEY: espera hasta que se pulse una tecla.
****************************************************************************/
BOOLEAN anykey(void)
{

sysmess(22);                    /* 'Pulsa una tecla.' */
vv_lee_tecla();

return(TRUE);
}

/****************************************************************************
	PAUSE: realiza una pausa de una duraci¢n determinada o hasta que se
	  pulse una tecla.
	  Entrada:      'pau' valor de la pausa en dÇcimas de segundo
****************************************************************************/
BOOLEAN pause(BYTE pau)
{

/* hace la pausa */
pausa(pau);

/* saca tecla del buffer */
if(bioskey(1)) bioskey(0);

return(TRUE);
}

/****************************************************************************
	LISTOBJ: lista los objetos de la localidad actual.
	  Entrada:      bandera 7 a 1 si se imprime mensaje 'TambiÇn puede
			ver: nada' cuando no hay objetos que listar
****************************************************************************/
BOOLEAN listobj(void)
{
BYTE i, j=0;
BYTE obl[MAX_OBJ];

/* recorre toda la tabla de objetos */
for(i=0; i<cab.num_obj; i++) {
	/* almacena n£meros de objetos en localidad actual */
	if(loc_obj[i]==getvar(1)) {
		obl[j]=i;
		j++;
	}
}

/* si no est† oscuro o hay luz lista objetos */
if(zero(0) || light()) {
	if(j || notzero(7)) sysmess(9);         /* 'TambiÇn puedes ver: ' */
	listat(getvar(1));
}

return(TRUE);
}

/****************************************************************************
	FIRSTO: coloca el puntero NEXTO al principio de la tabla de objetos.
****************************************************************************/
BOOLEAN firsto(void)
{

/* coloca puntero al inicio de la tabla - 1 */
ptr_nexto=-1;
/* indica que est† activo bucle DOALL */
doall=TRUE;

return(TRUE);
}

/****************************************************************************
	NEXTO: mueve el puntero NEXTO al siguiente objeto que estÇ en la
	  localidad especificada.
	  Entrada:      'locno' n£mero de localidad
	  Salida:       variables 3 y 4 con el nombre y adjetivo del
			siguiente objeto encontrado (si el objeto encontrado
			es el £ltimo pone 'doall' a FALSE)
****************************************************************************/
BOOLEAN nexto(BYTE locno)
{
char *po;
int i;

/* si no est† en bucle DOALL sale */
if(doall==FALSE) return(TRUE);

/* si se refiere a la localidad actual sustituye por su valor */
if(locno==LOC_ACTUAL) locno=getvar(1);

while(1) {
	ptr_nexto++;
	/* si lleg¢ al final cancela bucle DOALL */
	if(ptr_nexto>=(int)cab.num_obj) {
		doall=FALSE;
		return(TRUE);
	}
	/* si objeto est† en localidad */
	if(loc_obj[ptr_nexto]==locno) {
		/* puntero a objeto */
		po=tab_obj+tab_desp_obj[ptr_nexto];
		/* coge nombre y adjetivo de objeto */
		setvar(3,(BYTE)*po);
		setvar(4,(BYTE)*(po+1));
		/* mira si es el £timo objeto en la localidad indicada */
		for(i=ptr_nexto+1; i<(int)cab.num_obj; i++) {
			if(loc_obj[i]==locno) break;
		}
		/* si es £ltimo objeto desactiva bucle DOALL */
		if(i>=(int)cab.num_obj) doall=FALSE;
		return(TRUE);
	}
}

}

/****************************************************************************
	SYNONYM: coloca el verbo-nombre dado en las variables 2 y 3,
	  si alguno es NO_PAL no realiza la sustituci¢n.
****************************************************************************/
BOOLEAN synonym(BYTE verb, BYTE nomb)
{

if(verb!=NO_PAL) setvar(2,verb);
if(nomb!=NO_PAL) setvar(3,nomb);

return(TRUE);
}

/****************************************************************************
	HASAT: comprueba si el objeto actual (cuyo n£mero est† en la variable
	  del sistema 8) tiene activada una bandera de usuario.
	  Entrada:      'val' n£mero de bandera de usuario a comprobar
			(0 a 17), 16 comprueba si es PRENDA y 17 si FUENTE
			DE LUZ
	  Salida:       TRUE si la bandera de usuario est† a 1
			FALSE si est† a 0
****************************************************************************/
BOOLEAN hasat(BYTE val)
{
char *po;
unsigned flags2, masc=0x8000;

/* puntero a banderas2 de objeto */
po=tab_obj+tab_desp_obj[getvar(8)]+4;

/* comprobar PRENDA o FUENTE LUZ */
if((val==16) || (val==17)) {
	/* puntero a banderas1 de objeto */
	po--;
	/* comprueba PRENDA */
	if((val==16) && (*po & 0x01)) return(TRUE);
	/* comprueba LUZ */
	if((val==17) && (*po & 0x02)) return(TRUE);
	return(FALSE);
}

/* coge las banderas de usuario */
flags2=((*po)*256)+(*(po+1));

/* desplaza m†scara */
masc>>=val;

/* si es 1 el bit correspondiente */
if(flags2 & masc) return(TRUE);

return(FALSE);
}

/****************************************************************************
	HASNAT: comprueba si el objeto actual (cuyo n£mero est† en la
	  variable del sistema 8) no tiene activada una bandera de usuario.
	  Entrada:      'val' n£mero de bandera de usuario a comprobar
			(0 a 17), 16 comprueba si es PRENDA y 17 si FUENTE
			DE LUZ
	  Salida:       TRUE si la bandera de usuario est† a 0
			FALSE si est† a 1
****************************************************************************/
BOOLEAN hasnat(BYTE val)
{

return(TRUE-hasat(val));
}

/****************************************************************************
	LIGHT: comprueba si hay presente una fuente de luz.
	  Salida:       TRUE si hay presente una fuente de luz
			FALSE si no
****************************************************************************/
BOOLEAN light(void)
{
BYTE i;
char *po;

/* recorre tabla de objetos */
for(i=0; i<cab.num_obj; i++) {
	/* puntero a banderas1 de objeto */
	po=tab_obj+tab_desp_obj[i]+3;
	/* si es fuente de luz y adem†s est† presente, sale con TRUE */
	if(*po & 0x02) if(present(i)) return(TRUE);
}

/* si no hay presente ninguna fuente de luz */
return(FALSE);
}

/****************************************************************************
	NOLIGHT: comprueba si no hay presente ninguna fuente de luz.
	  Salida:       TRUE si no hay presente ninguna fuente de luz
			FALSE si hay presente al menos una fuente de luz
****************************************************************************/
BOOLEAN nolight(void)
{

return(TRUE-light());
}

/****************************************************************************
	RANDOM1: genera n£meros aleatorios.
	  Entrada:      'varno' n£mero de variable que contendr† el n£mero
			aleatorio
			'rnd' l°mite de n£mero aleatorio
	  Salida:       'varno' conteniendo un n£mero aleatorio entre 0 y
			'rnd'-1
****************************************************************************/
BOOLEAN random1(BYTE varno, BYTE rnd)
{

setvar(varno,(BYTE)(random(rnd)));

return(TRUE);
}

/****************************************************************************
	SEED: coloca el punto de inicio del generador de n£meros aleatorios.
	  Entrada:      'seed' punto de inicio dentro de la serie de n£meros
			aleatorios. Si es 0 inicializa la serie con un valor
			aleatorio.
****************************************************************************/
BOOLEAN seed(BYTE seed)
{

if(!seed) randomize();
else srand((unsigned)seed);

return(TRUE);
}

/****************************************************************************
	PUTO: coloca el objeto actual cuyo n£mero est† en la variable 8 en
	  una localidad.
	  Entrada:      'nloc' n£mero de localidad
****************************************************************************/
BOOLEAN puto(BYTE nloc)
{

place(getvar(8),nloc);

return(TRUE);
}

/****************************************************************************
	INKEY: coloca en las variables 9 y 10 el par de c¢digos ASCII IBM
	  de la £ltima tecla pulsada (si se puls¢ alguna).
	  Salida:       TRUE si se puls¢ una tecla y adem†s...
			 -variable 9 conteniendo 1er c¢digo ASCII IBM (c¢digo
			  ASCII del car†cter, si es distinto de 0)
			 -variable 10 conteniendo 2o c¢digo ASCII IBM (c¢digo
			  de scan de la tecla pulsada)
			FALSE si no se puls¢ ninguna tecla (deja sin
			modificar las variables 9 y 10)
****************************************************************************/
BOOLEAN inkey(void)
{
unsigned tecla;

/* si hay tecla esperando */
if(bioskey(1)) {
	/* recoge c¢digos tecla pulsada */
	tecla=bioskey(0);
	/* c¢digo ASCII, byte bajo */
	setvar(9,(BYTE)(tecla & 0x00ff));
	/* c¢digo scan, byte alto */
	setvar(10,(BYTE)((tecla >> 8) & 0x00ff));
	return(TRUE);
}

return(FALSE);
}

/****************************************************************************
	COPYOV: copia el n£mero de localidad en la que est† el objeto
	  referenciado en una variable dada.
	  Entrada:      'nobj' n£mero de objeto
			'varno' n£mero de variable
****************************************************************************/
BOOLEAN copyov(BYTE nobj, BYTE varno)
{

setvar(varno,loc_obj[nobj]);

return(TRUE);
}

/****************************************************************************
	CHANCE: comprueba una probabilidad en tanto por ciento.
	  Entrada:      'rnd' probabilidad de 0 a 100
	  Salida:       TRUE si el n£mero aleatorio generado internamente es
			menor o igual que rnd
			FALSE si es mayor que rnd
****************************************************************************/
BOOLEAN chance(BYTE rnd)
{
BYTE chc;

/* n£mero aleatorio entre 0 y 100 */
chc=(BYTE)random(101);

if(chc>rnd) return(FALSE);

return(TRUE);
}

/****************************************************************************
	RAMSAVE: graba en uno de los bancos de ram disponibles el estado
	  actual (variables, banderas y posici¢n actual de objetos).
	  Entrada:      'banco' n£mero de banco de memoria a usar (0 o 1)
****************************************************************************/
BOOLEAN ramsave(BYTE banco)
{
unsigned i;

/* marca banco usado */
ram[banco].usado=TRUE;

/* guarda variables */
for(i=0; i<VARS; i++) ram[banco].bram[i]=getvar(i);
#ifndef DAAD
/* guarda banderas */
for(i=0; i<BANDS; i++) ram[banco].bram[VARS+i]=flag[i];
#endif
/* guarda localidades actuales de los objetos */
for(i=0; i<MAX_OBJ; i++) ram[banco].bram[VARS+BANDS+i]=loc_obj[i];

#ifndef DAAD
/* G3.25: guardamos la tabla de objetos para consevar las banderas de usuario */
if(ram[banco].tab_obj!=NULL) {
	free(ram[banco].tab_obj);
	ram[banco].tab_obj=NULL;
}
if((ram[banco].tab_obj=(char *)malloc((size_t)cab.bytes_obj))==NULL) {
	sysmess(28);
	return(FALSE);
}
for(i=0; i<cab.bytes_obj; i++) ram[banco].tab_obj[i]=tab_obj[i];
#endif

return(TRUE);
}

/****************************************************************************
	RAMLOAD: recupera una posici¢n guardada con RAMSAVE.
	  Entrada:      'banco' n£mero de banco de memoria a usar (0 o 1)
			'vtop'  m†ximo n£mero de variable a recuperar (se
			recuperar† desde la 0 hasta 'vtop' inclusive)
			'ftop' m†ximo n£mero de bandera a recuperar (se
			recuperar† desde la 0 hasta ftop inclusive)
	  Salida:       TRUE si se pudo ejecutar RAMLOAD
			FALSE si el banco indicado no fuÇ usado antes por
			RAMSAVE
****************************************************************************/
BOOLEAN ramload(BYTE banco, BYTE vtop, BYTE ftop)
{
int nbyte, nbit;
unsigned i;
BYTE mascara;

/* mira si el banco ha sido usado */
if(!ram[banco].usado) return(FALSE);

/* recupera variables */
for(i=0; i<=(unsigned)vtop; i++) setvar(i,ram[banco].bram[i]);
#ifndef DAAD
/* recupera banderas */
for(i=0; i<=(unsigned)ftop; i++) {
	nbyte=i/8;
	nbit=i%8;
	mascara=0x80;
	mascara>>=nbit;
	if(ram[banco].bram[VARS+nbyte] & mascara) set((BYTE)i);
	else clear((BYTE)i);
}
#endif
/* recupera localidades actuales de los objetos */
for(i=0; i<MAX_OBJ; i++) loc_obj[i]=ram[banco].bram[VARS+BANDS+i];

#ifndef DAAD
/* G3.25: recupera la tabla de objetos */
if(ram[banco].tab_obj==NULL) {
	sysmess(28);
	return(FALSE);
}
else {
	for(i=0; i<cab.bytes_obj; i++) tab_obj[i]=ram[banco].tab_obj[i];
}

/* G3.30: tabla de mensajes activa */
tabla_msg=getvar(17);
#endif

return(TRUE);
}

/****************************************************************************
	ABILITY: designa el n£mero de objetos m†ximo que puede ser llevado.
	  Entrada:      'nobjs' n£mero de objetos m†ximo (0 ilimitados)
****************************************************************************/
BOOLEAN ability(BYTE nobjs)
{

/* coloca m†ximo n£mero de objetos en variable 7 */
let(7,nobjs);

return(TRUE);
}

/****************************************************************************
	AUTOG: coge el objeto cuyo nombre-adjetivo est†n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de GET [8]).
	  Salida:       TRUE si se pudo coger el objeto
			FALSE si no se pudo coger
****************************************************************************/
BOOLEAN autog(void)
{

whato();

return(get(getvar(8)));
}

/****************************************************************************
	AUTOD: deja el objeto cuyo nombre-adjetivo est†n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de DROP [8]).
	  Salida:       TRUE si se pudo dejar el objeto
			FALSE si no se pudo dejar
****************************************************************************/
BOOLEAN autod(void)
{

whato();

return(drop(getvar(8)));
}

/****************************************************************************
	AUTOW: pone el objeto cuyo nombre-adjetivo est†n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de WEAR [8]).
	  Salida:       TRUE si se pudo poner el objeto
			FALSE si no se pudo poner
****************************************************************************/
BOOLEAN autow(void)
{

whato();

return(wear(getvar(8)));
}

/****************************************************************************
	AUTOR: quita el objeto cuyo nombre-adjetivo est†n en las variables
	  3 y 4 respectivamente (es como WHATO seguido de REMOVE [8]).
	  Salida:       TRUE si se pudo quitar el objeto
			FALSE si no se pudo quitar
****************************************************************************/
BOOLEAN autor(void)
{

whato();

return(remove1(getvar(8)));
}

/****************************************************************************
	ISDOALL: comprueba si se est† ejecutando un bucle DOALL (FIRSTO ha
	  sido ejecutado y NEXTO no ha alcanzado todav°a £ltimo objeto).
	  Salida:       TRUE si se est† ejecutando un bucle DOALL
			FALSE si no
****************************************************************************/
BOOLEAN isdoall(void)
{

if(doall==TRUE) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ASK: realiza una pregunta y espera hasta que se introduce la
	  respuesta.
	  Entrada:      'smess1' mensaje del sistema que contiene la pregunta
			'smess2'    "     "     "    con las posibles
			respuestas (cada una de una sola letra y seguidas,
			no importa si en may£sculas o min£sculas), el m†ximo
			n£mero de caracteres permitidos es 256 (0 a 255), si
			son m†s, la variable 'varno' contendr† resultados
			imprevisibles
			'varno' n£mero de variable que contendr† la respuesta
	  Salida:       'varno'=0 si se tecle¢ 1er car†cter de 'smess2', 1 si
			el 2?, 2 si el 3?, etc...
****************************************************************************/
BOOLEAN ask(BYTE smess1, BYTE smess2, BYTE varno)
{
char *pm, tecla;
int antx, anty;
BYTE i;

/* imprime la pregunta */
sysmess(smess1);
/* guarda posici¢n actual del cursor */
antx=w[getvar(0)].cvx;
anty=w[getvar(0)].cvy;
while(1) {
	/* recupera coordenadas */
	w[getvar(0)].cvx=antx;
	w[getvar(0)].cvy=anty;

	/* imprime cursor */
	vv_impc(*(tab_msy+tab_desp_msy[7]),&w[getvar(0)]);

	/* recupera coordenadas */
	w[getvar(0)].cvx=antx;
	w[getvar(0)].cvy=anty;

	do {
		tecla=mayuscula(vv_lee_tecla());
	} while((BYTE)tecla==COD_RAT);

	/* imprime tecla pulsada */
	vv_impc(tecla,&w[getvar(0)]);

	/* comienza b£squeda */
	i=0;
	/* puntero a mensaje con las respuestas */
	pm=tab_msy+tab_desp_msy[smess2];

	/* analiza hasta final de cadena */
	while(*pm) {
		if(mayuscula(*pm)==tecla) {
			/* almacena n£mero de car†cter de smess2 y sale */
			setvar(varno,i);
			return(TRUE);
		}
		/* pasa al siguiente car†cter de smess2 */
		pm++;
		i++;
	}
}

}

/****************************************************************************
	QUIT: presenta el mensaje del sistema 24 (?Est†s seguro?) y pregunta
	  para abandonar.
	  Salida:       TRUE si se responde con el 1er car†cter del mensaje
			del sistema 25
			FALSE si se responde con el 2o car†cter del mensaje
			del sistema 25
	  NOTA: se usa la variable 11
****************************************************************************/
BOOLEAN quit(void)
{

/* hace pregunta '?Est†s seguro?' */
ask(24,25,11);
newline();

/* si respondi¢ con 1er car†cter */
if(!getvar(11)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	SAVE: guarda la posici¢n actual en disco (variables, banderas y
	  posici¢n actual de objetos).
****************************************************************************/
BOOLEAN save(void)
{
BOOLEAN raton;
int h_save;

/* coge estado actual del raton */
raton=notzero(10);

sysmess(26);                    /* 'Nombre del fichero: ' */
clear(10);                      /* desactiva pero no oculta rat¢n */
vv_inputt(f_sl,MAXLONG,*(tab_msy+tab_desp_msy[7]),VVINP_CONV,&w[getvar(0)],0,0);
if(raton) set(10);
newline();

/* si el fichero ya existe */
if(!access(f_sl,0)) {
	ask(27,25,11);          /* 'Fichero ya existe. ?Quieres continuar? ' */
	newline();
	/* si respondi¢ con 2? car†cter del mensaje del sistema 25 */
	if(getvar(11)) return(TRUE);
}

h_save=open(f_sl,O_CREAT|O_TRUNC|O_BINARY|O_RDWR,S_IREAD|S_IWRITE);

/* si ocurri¢ un error en apertura */
if(h_save==-1) {
	sysmess(28);            /* 'Error de apertura de fichero.' */
	return(TRUE);
}

/* escribe cadena de reconocimiento */
if(write(h_save,SRECON,L_RECON+1)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

/* guarda variables */
if(write(h_save,var,VARS)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

/* guarda banderas */
if(write(h_save,flag,BANDS)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

/* guarda localidades actuales de los objetos */
if(write(h_save,loc_obj,MAX_OBJ)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

/* G3.25: guarda la tabla de objetos completa, esto incluye las */
/* banderas de objetos */
if(write(h_save,tab_obj,cab.bytes_obj)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

close(h_save);

return(TRUE);
}

/****************************************************************************
	LOAD: recupera de disco una posici¢n grabada con SAVE.
	  Entrada:      'vtop' m†ximo n£mero de variable a recuperar (se
			recuperar† desde la 0 hasta 'vtop' inclusive)
			'ftop' m†ximo n£mero de bandera a recuperar (se
			recuperar† desde la 0 hasta 'ftop' inclusive)
****************************************************************************/
BOOLEAN load(BYTE vtop, BYTE ftop)
{
int h_load, i, nbyte, nbit;
BOOLEAN raton;
BYTE mascara;
char rec_ld[L_RECON+1];
BYTE var_l[VARS];
BYTE flag_l[BANDS];

/* coge estado actual del rat¢n */
raton=notzero(10);

sysmess(26);                    /* 'Nombre del fichero: ' */
clear(10);                      /* desactiva pero no oculta rat¢n */
vv_inputt(f_sl,MAXLONG,*(tab_msy+tab_desp_msy[7]),VVINP_CONV,&w[getvar(0)],0,0);
if(raton) set(10);
newline();

h_load=open(f_sl,O_BINARY|O_RDONLY);

/* si ocurri¢ un error en apertura */
if(h_load==-1) {
	sysmess(28);            /* 'Error de apertura de fichero.' */
	return(TRUE);
}

/* recoge cadena de reconocimiento */
if(read(h_load,rec_ld,L_RECON+1)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}

/* comprueba cadena de reconocimiento */
if(strcmp(rec_ld,SRECON)) {
	sysmess(30);            /* 'Fichero no v†lido.' */
	close(h_load);
	return(TRUE);
}

/* recoge variables */
if(read(h_load,var_l,VARS)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}

#ifndef DAAD
/* recoge banderas */
if(read(h_load,flag_l,BANDS)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}
#endif

/* recoge localidad actual de los objetos */
if(read(h_load,loc_obj,MAX_OBJ)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}

#ifndef DAAD
/* G3.25: recoge la tabla de objetos */
if(read(h_load,tab_obj,cab.bytes_obj)==-1) {
	sysmess(29);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}
#endif

/* recupera variables */
for(i=0; i<=(int)vtop; i++) setvar(i,var_l[i]);

#ifndef DAAD
/* recupera banderas */
for(i=0; i<=(int)ftop; i++) {
	nbyte=i/8;
	nbit=i%8;
	mascara=0x80;
	mascara>>=nbit;
	if(flag_l[nbyte] & mascara) set((BYTE)i);
	else clear((BYTE)i);
}

/* G3.30: tabla de mensajes activa */
tabla_msg=getvar(17);
#endif

close(h_load);


return(TRUE);
}

/****************************************************************************
	EXIT1: sale al sistema operativo o reinicia.
	  Entrada:      'ex' si es 0 reinicia, si es 1 sale al sistema
			operativo
****************************************************************************/
BOOLEAN exit1(BYTE ex)
{
unsigned i;

/* sale al sistema operativo si 'ex' es 1 */
if(ex==1) m_err(0,"",1);

/* reinicializaci¢n */
/* borra la pantalla */
cls();

/* reinicializa variables */
inic();

/* G3.25: recupera los objetos */
for(i=0; i<cab.bytes_obj; i++) tab_obj[i]=tab_obj2[i];

/* se pone al inicio de proceso 0 */
restart();

/* -1 a ptr_proc (y -1 del restart), luego se sumar†n 2 */
ptr_proc--;

return(FALSE);
}

#pragma warn -rvl
/****************************************************************************
	END: pregunta para otra partida o salir. Si se responde con el primer
	  car†cter del mensaje del sistema 25, ejecuta un EXIT 0 (reiniciali-
	  zaci¢n). Si no sale al sistema operativo.
****************************************************************************/
BOOLEAN end1(void)
{

ask(31,25,11);                          /* '?Lo intentas de nuevo? ' */
newline();

/* si respondi¢ con 1er car†cter del mensaje del sistema 25 */
if(!getvar(11)) {
	/* reinicializaci¢n */
	exit1(0);
	/* +1 a ptr_proc (-2 del exit1), luego se sumar† 1 */
	ptr_proc++;
	return(FALSE);
}

/* sale al sistema operativo */
exit1(1);

}
#pragma warn +rvl

/****************************************************************************
	PRINTAT: coloca el cursor en una posici¢n dada de la ventana actual.
	  Entrada:      'y', 'x' coordenada del cursor (fila, columna); si
			salen de los l°mites de la ventana el cursor se
			colocar† en la esquina superior izquierda (0,0)
****************************************************************************/
BOOLEAN printat(BYTE y, BYTE x)
{

/* si se sale de ventana coloca cursor en esquina superior izquierda */
if((y>(BYTE)(w[getvar(0)].lyi-1)) || (x>(BYTE)(w[getvar(0)].lxi-1))) y=x=0;

w[getvar(0)].cvy=y*w[getvar(0)].chralt;
w[getvar(0)].cvx=x*8;

return(TRUE);
}

/****************************************************************************
	SAVEAT: almacena la posici¢n de impresi¢n de la ventana actual. Cada
	  ventana tiene sus propias posiciones de impresi¢n almacenadas.
****************************************************************************/
BOOLEAN saveat(void)
{

w[getvar(0)].cvxs=w[getvar(0)].cvx;
w[getvar(0)].cvys=w[getvar(0)].cvy;

return(TRUE);
}

/****************************************************************************
	BACKAT: recupera la posici¢n de impresi¢n guardada por el £ltimo
	  SAVEAT en la ventana actual.
	  NOTA: si no se ejecut¢ ning£n SAVEAT, la posici¢n de impresi¢n
	  recuperada ser† la (0,0).
****************************************************************************/
BOOLEAN backat(void)
{

w[getvar(0)].cvx=w[getvar(0)].cvxs;
w[getvar(0)].cvy=w[getvar(0)].cvys;

return(TRUE);
}

/****************************************************************************
	NEWTEXT: deshecha el resto de la l°nea de input que a£n queda por
	  analizar y coloca el puntero al final de la misma.
****************************************************************************/
BOOLEAN newtext(void)
{

/* coloca puntero al final de la frase */
for(; *lin; lin++);
/* indica que no queda m†s por analizar */
mas_texto=TRUE;

return(TRUE);
}

/****************************************************************************
	PRINTC: imprime un car†cter en la posici¢n actual del cursor y dentro
	  de la ventana activa.
	  Entrada:      'car' c¢digo ASCII del car†cter a imprimir
****************************************************************************/
BOOLEAN printc(BYTE car)
{

/* imprime el car†cter en la ventana activa */
vv_impc(car,&w[getvar(0)]);

return(TRUE);
}

/****************************************************************************
	INK: selecciona el color temporal del primer plano en la ventana
	  activa.
	  Todos los textos de esa ventana se imprimir†n con ese color de
	  tinta.
	  Entrada:      'color' °ndice de color
****************************************************************************/
BOOLEAN ink(BYTE color)
{

w[getvar(0)].colort=color;

return(TRUE);
}

/****************************************************************************
	PAPER: selecciona el color temporal del fondo en la ventana activa.
	  Todos los textos de esa ventana se imprimir†n con ese color de
	  fondo.
	  Entrada:      'color' °ndice de color
****************************************************************************/
BOOLEAN paper(BYTE color)
{

w[getvar(0)].colortf=color;

return(TRUE);
}

#pragma warn -par
/****************************************************************************
	BRIGHT: selecciona el atributo de brillo temporal para la ventana
	  activa. Todos los textos de esa ventana se imprimir†n con ese
	  brillo.
	  Entrada:      'b' brillo (0=sin brillo, 1=con brillo)
	  NOTA: este condacto ya no sirve para nada, se mantiene por compa-
	  tibilidad con versiones anteriores
****************************************************************************/
BOOLEAN bright(BYTE b)
{

return(TRUE);
}

/****************************************************************************
	BLINK: selecciona el atributo de parpadeo temporal para la ventana
	  activa. Todos los textos de esa ventana se imprimir†n con ese
	  parpadeo.
	  Entrada:      'b' parpadeo (0=sin parpadeo, 1=con parpadeo)
	  NOTA: este condacto ya no sirve para nada, se mantiene por compa-
	  tibilidad con versiones anteriores
****************************************************************************/
BOOLEAN blink(BYTE b)
{

return(TRUE);
}
#pragma warn +par

/****************************************************************************
	COLOR: selecciona un color (fondo y primer plano ) temporal para la
	  ventana activa.
	  Entrada:      'col' color, los 4 bits altos indican el °ndice del
	    color de fondo y los 4 bits bajos el del color de primer plano
****************************************************************************/
BOOLEAN color(BYTE col)
{

/* color de fondo */
w[getvar(0)].colortf=(col >> 4);
/* color de primer plano */
w[getvar(0)].colort=(col & 0x0f);

return(TRUE);
}

#pragma warn -par
/****************************************************************************
	DEBUG: activa o desctiva el paso a paso.
	  Entrada:      'modo' indica si el paso a paso est† activado (1)
			o desactivado (0)
	  NOTA: este condacto s¢lo es activo en el intÇrprete-debugger
****************************************************************************/
BOOLEAN debug(BYTE modo)
{

#if DEBUGGER==1
if(modo) {
	debugg=TRUE;
	/* indicamos que no es primera l°nea de */
	/* entrada para que imp_condacto() no */
	/* imprima el verbo-nombre ya que el puntero */
	/* al condacto puede no estar ajustado */
	pra_lin=FALSE;
}
else debugg=FALSE;
#endif

return(TRUE);
}
#pragma warn +par

/****************************************************************************
	WBORDER: define tipo de borde de una ventana.
	  Entrada:      'nw' n£mero de ventana
			'borde' tipo de borde de la ventana
****************************************************************************/
BOOLEAN wborder(BYTE nw, BYTE borde)
{

vv_crea(w[nw].vy,w[nw].vx,w[nw].lx,w[nw].ly,w[nw].colorf,w[nw].color,borde,
  &w[nw]);

return(TRUE);
}

/****************************************************************************
	CHARSET: carga y selecciona un nuevo juego de caracteres.
	  Entrada:      'set' n£mero del juego de caracteres (0-255), si se
			introduce 0, se seleccionar† el juego de caracteres
			por defecto.
	  Salida:       TRUE si se carg¢ la fuente con Çxito
			FALSE si hubo alg£n error o la tarjeta gr†fica no
			soporta cambio del juego de caracteres
	  NOTA: los juegos de caracteres deben estar en ficheros cuyo nombre
	  sea F#??????.ext siendo 'ext' el n£mero identificador de 1 a 255,
	  por ejemplo: F#SIMPLEF.1, F#EPICO1.2, ...
****************************************************************************/
BOOLEAN charset(BYTE set)
{
struct ffblk info;
char nombre[13]="F#*.";
char num[4];

/* si selecciona juego de caracteres 0, activa las definiciones por defecto */
if(!set) {
	def_chr_set(NULL,NULL,NULL,NULL);
	return(TRUE);
}

/* construye nombre de fichero a?adiendo extensi¢n */
itoa((int)set,num,10);
strcat(nombre,num);

/* busca el fichero y sale si no lo encontr¢ */
if(findfirst(nombre,&info,0)) return(FALSE);

/* el nombre verdadero del fichero estar† en 'info.name' */
/* lo carga y actualiza generador de caracteres */
if(!carga_def(info.ff_name)) return(FALSE);

return(TRUE);
}

/****************************************************************************
	EXTERN1: ejecuta un programa externo.
	  Entrada:      'prg' n£mero identificativo del programa a ejecutar
			'par' par†metro que se pasar† al programa externo
			(se pasar† el n£mero 0-255 como una cadena de carac-
			teres)
	  Salida:       TRUE si se ejecut¢ con Çxito
			FALSE si no se pudo ejecutar
			NOTA: la variable 13 contendr† el c¢digo de salida
			del proceso externo que se ejecut¢ (0 si no pudo
			ejecutarse)
	  NOTA: este condacto ejecutar† un fichero cuyo nombre sea 'EXTERn'
	  siendo 'n' un n£mero de 0 a 255; as° una llamada a EXTERN 0
	  ejecutar† el fichero EXTER0, EXTERN 128 ejecutar† el fichero
	  EXTER128,...
	  Se buscar† primero un fichero con extensi¢n .COM, si no se encuentra
	  se buscar† un .EXE y finalmente un .BAT.
	  Se pasar† el par†metro 'par' como si se hubiese tecleado en la
	  l°nea del DOS: EXTERn par
****************************************************************************/
BOOLEAN extern1(BYTE prg, BYTE par)
{
char nombre[9]="EXTER";
char num[4], dir_var[10], dir_flag[10], com[128];
int cod;

/* construye nombre de fichero a?adiendo n£mero identificativo */
itoa((int)prg,num,10);
strcat(nombre,num);

/* convierte par†metro en cadena ASCII */
itoa((int)par,num,10);

/* segmento:desplazamiento de tabla de variables */
sprintf(dir_var,"%04X:%04X",FP_SEG((BYTE far *)&var[0]),
  FP_OFF((BYTE far *)&var[0]));

/* segmento:desplazamiento de tabla de banderas */
sprintf(dir_flag,"%04X:%04X",FP_SEG((BYTE far *)&flag[0]),
  FP_OFF((BYTE far *)&flag[0]));

/* intenta ejecutar el programa */
cod=spawnl(P_WAIT,nombre,nombre,num,dir_var,dir_flag,NULL);
/* si no pudo ejecutar COM o EXE intenta ejecutar a travÇs del COMMAND.COM */
/* por si es un fichero BAT */
if(cod==-1) {
	strcpy(com,nombre);
	strcat(com," ");
	strcat(com,num);
	strcat(com," ");
	strcat(com,dir_var);
	strcat(com," ");
	strcat(com,dir_flag);
	cod=system(com);
}

/* pone a cero la variable 13 */
setvar(13,0);

if(cod==-1) return(FALSE);
else {
	setvar(13,(BYTE)cod);
	return(TRUE);
}

}

/****************************************************************************
	GTIME: devuelve la hora real del sistema.
	  Entrada:      'nv' n£mero de variable inicial d¢nde se colocar†
			la hora; 'var' contendr† los segundos, 'var+1' los
			minutos y 'var+2' la hora
	  NOTA: si 'nv' es mayor de 253 el condacto no har† nada
****************************************************************************/
BOOLEAN gtime(BYTE nv)
{
struct time hora;

/* comprueba que el n£mero de variable no sea mayor de 253 */
if(nv>253) return(TRUE);

/* recoge hora del sistema */
gettime(&hora);

/* guarda en variables los segundos, minutos y la hora en este orden */
/* segundos de 0 a 59 */
setvar(nv,hora.ti_sec);
/* minutos de 0 a 59 */
setvar(nv+1,hora.ti_min);
/* hora de 0 a 23 */
setvar(nv+2,hora.ti_hour);

return(TRUE);
}

/****************************************************************************
	TIME: selecciona tiempo muerto y modo de temporizaci¢n del condacto
	  INPUT.
	  Entrada:      'tiempo' tiempo muerto para INPUT en segundos
			'modo' modo de temporizaci¢n; 0 si el tiempo se
			da para teclear la frase completa, 1 si se da hasta
			teclear primer car†cter
****************************************************************************/
BOOLEAN time1(BYTE tiempo, BYTE modo)
{

/* coloca el tiempo en la variable 16 */
setvar(16,tiempo);

/* coloca el modo en la bandera 6 */
if(modo==0) clear(6);
else set(6);

return(TRUE);
}

/****************************************************************************
	TIMEOUT: comprueba si se produjo un tiempo muerto en el £ltimo
	  condacto INPUT.
	  Salida:       TRUE si se produjo un tiempo muerto
			FALSE si no
****************************************************************************/
BOOLEAN timeout(void)
{

/* si la variable 12 es 1 se produjo un tiempo muerto */
if(getvar(12)==1) return(TRUE);
else return(FALSE);

}

#pragma warn -use
/****************************************************************************
	MODE: selecciona el modo de v°deo.
	  Entrada:      'modo' modo a seleccionar, 0 para 640x480x16, 1 para
			320x200x256
****************************************************************************/
BOOLEAN mode(BYTE modo)
{
unsigned long tam_img_debug;

modo_video((int)modo);

#if DEBUGGER==1

/* crea la ventana del debugger ya que ha cambiado el modo de pantalla */
vv_crea(vv_deb.vy,vv_deb.vx,vv_deb.lx,vv_deb.ly,vv_deb.colorf,vv_deb.color,
  vv_deb.borde,&vv_deb);

/* reajusta buffer para guardar fondo */
if(img_debug!=NULL) farfree(img_debug);
tam_img_debug=blq_tam(0,0,(vv_deb.lx*8)-1,(vv_deb.ly*vv_deb.chralt)-1);
img_debug=farmalloc(tam_img_debug);

#endif

return(TRUE);
}
#pragma warn +use

/****************************************************************************
	GRAPHIC: presenta un gr†fico en la ventana actual.
	  Entrada:      'graf' n£mero de fichero GRF (librer°a)
			'img' n£mero de imagen
			'modo' modo de dibujo
			  0 - dibuja la imagen seg£n la lee del disco
			  1 - dibuja la imagen al final
			  2 - como 0 pero sin alterar paleta de colores
			  3 - como 1 pero sin alterar paleta de colores
			  otros valores se consideran igual a 0
	  Salida:       TRUE si se carg¢ el gr†fico
			FALSE si hubo alg£n error
	  NOTA: los gr†ficos deben estar agrupados en ficheros GRF cuyo
	  nombre sea G#??????.ext siendo 'ext' el n£mero identificador de 1
	  a 255, por ejemplo: G#OBJET.1, G#PERSO.203, ...
****************************************************************************/
BOOLEAN graphic(BYTE graf, BYTE img, BYTE modo)
{
STC_IMGRF imagen;
struct ffblk info;
char grafico[8]="G#*.", num[4];
int x, y, ancho, alto, err, paleta=0;

/* comprueba si modo es v†lido, si no le asigna 0 */
if(modo>3) modo=0;

/* si es modo 2 o 3 indica que hay que conservar paleta */
if((modo==2) || (modo==3)) paleta=1;

/* construye nombre de fichero a?adiendo extensi¢n */
itoa((int)graf,num,10);
strcat(grafico,num);

/* busca el fichero y sale si no lo encuentra */
/* el nombre verdadero del fichero estar† en 'info.ff_name' */
if(findfirst(grafico,&info,0)) return(FALSE);

/* calcula posici¢n y dimensiones de la ventana actual, en pixels */
x=w[getvar(0)].vxi*8;
y=w[getvar(0)].vyi*w[getvar(0)].chralt;
ancho=w[getvar(0)].lxi*8;
alto=w[getvar(0)].lyi*w[getvar(0)].chralt;

rg_puntero(RG_OCULTA);
if((modo==0) || (modo==2)) err=grf_visualiza(info.ff_name,img,x,y,ancho,alto,
  paleta);
else {
	err=grf_decodifica(info.ff_name,img,&imagen);
	if(!err) {
		err=grf_dibuja(&imagen,x,y,ancho,alto,paleta);
		grf_libera_mem(&imagen);
	}
	else err=grf_visualiza(info.ff_name,img,x,y,ancho,alto,paleta);
}
rg_puntero(RG_MUESTRA);

/* si hubo error */
if(err) return(FALSE);

return(TRUE);
}

/****************************************************************************
	REMAPC: permite cambiar uno de los colores de la paleta.
	  Entrada:      'color' °ndice del color a cambiar
			'r', 'g', 'b' valor de las componentes RGB del color
****************************************************************************/
BOOLEAN remapc(BYTE color, BYTE r, BYTE g, BYTE b)
{

/* si en modo de 16 colores ajusta el °ndice de color para que no */
/* sobrepase el m†ximo permitido */
if(zero(5) && (color>15)) color=15;

asm {
	mov ax,1010h
	mov bh,0
	mov bl,color
	mov dh,r
	mov ch,g
	mov cl,b
	int 10h
}

return(TRUE);
}

/****************************************************************************
	SETAT: activa una bandera de usuario del objeto actual (cuyo n£mero
	  est† en la variable del sistema 8).
	  Entrada:      'val' n£mero de bandera de usuario a activar
			(0 a 17), 16 activa PRENDA y 17 activa FUENTE DE LUZ
****************************************************************************/
BOOLEAN setat(BYTE val)
{
char *po;
unsigned flags2, masc=0x8000;

/* puntero a banderas2 de objeto */
po=tab_obj+tab_desp_obj[getvar(8)]+4;

/* activa PRENDA */
if(val==16) {
	*(po-1) |= 0x01;
	return(TRUE);
}
/* activa FUENTE DE LUZ */
if(val==17) {
	*(po-1) |= 0x02;
	return(TRUE);
}

/* coge las banderas de usuario */
flags2=((*po)*256)+(*(po+1));

/* desplaza m†scara */
masc >>= val;

/* pone a 1 el bit correspondiente */
flags2 |= masc;

/* guarda nuevas banderas de usuario */
*po=(char)((flags2 >> 8) & 0x00ff);
*(po+1)=(char)(flags2 & 0x00ff);

return(TRUE);
}

/****************************************************************************
	CLEARAT: desactiva una bandera de usuario del objeto actual (cuyo
	  n£mero est† en la variable del sistema 8).
	  Entrada:      'val' n£mero de bandera de usuario a desactivar
			(0 a 17), 16 activa PRENDA y 17 activa FUENTE DE LUZ
****************************************************************************/
BOOLEAN clearat(BYTE val)
{
char *po;
unsigned flags2, masc=0x8000;

/* puntero a banderas2 de objeto */
po=tab_obj+tab_desp_obj[getvar(8)]+4;

/* desactiva PRENDA */
if(val==16) {
	*(po-1) &= 0xfe;
	return(TRUE);
}
/* deactiva FUENTE DE LUZ */
if(val==17) {
	*(po-1) &= 0xfd;
	return(TRUE);
}

/* coge las banderas de usuario */
flags2=((*po)*256)+(*(po+1));

/* desplaza m†scara */
masc >>= val;
masc ^= 0xffff;

/* pone a 0 el bit correspondiente */
flags2 &= masc;

/* guarda nuevas banderas de usuario */
*po=(char)((flags2 >> 8) & 0x00ff);
*(po+1)=(char)(flags2 & 0x00ff);

return(TRUE);
}

/****************************************************************************
	GETRGB: devuelve las componentes RGB de un color de la paleta.
	  Entrada:      'color' °ndice del color
			'var1', 'var2', 'var3' variables donde se
			almacenar†n las 3 componentes RGB en este orden
****************************************************************************/
BOOLEAN getrgb(BYTE color, BYTE var1, BYTE var2, BYTE var3)
{
BYTE r, g, b;

/* si en modo de 16 colores ajusta el °ndice de color para que no */
/* sobrepase el m†ximo permitido */
if(zero(5) && (color>15)) color=15;

asm {
	mov ax,1015h
	mov bh,0
	mov bl,color
	int 10h
	mov r,dh
	mov g,ch
	mov b,cl
}

setvar(var1,r);
setvar(var2,g);
setvar(var3,b);

return(TRUE);
}

/****************************************************************************
	PLAYFLI: visualiza una animaci¢n FLI.
	  Entrada:      'fli' n£mero de la animaci¢n
			'modo' modo de animaci¢n
			 0 - hasta que se pulse una tecla
			 1 - s¢lo una vez
	  Salida:       TRUE si pudo ejecutar la animaci¢n
			FALSE si hubo alg£n error
	  NOTA: las animaciones deben estar en ficheros cuyo nombre sea
	  A#??????.ext siendo 'ext' el n£mero identificador de 1 a 255,
	  por ejemplo: A#INTRO.1, A#AVION.203, ...
****************************************************************************/
BOOLEAN playfli(BYTE fli, BYTE modo)
{
Errval err;
struct ffblk info;
char flianim[8]="A#*.", num[4];

/* comprueba que modo de pantalla sea v†lido */
if(zero(5)) return(FALSE);

/* comprueba si modo es v†lido, si no le asigna 0 */
if(modo>1) modo=0;

/* construye nombre de fichero a?adiendo extensi¢n */
itoa((int)fli,num,10);
strcat(flianim,num);

/* busca el fichero y sale si no lo encuentra */
/* el nombre verdadero del fichero estar† en 'info.ff_name' */
if(findfirst(flianim,&info,0)) return(FALSE);

rg_puntero(RG_OCULTA);
if(modo==0) err=fli_play(info.ff_name);
else err=fli_once(info.ff_name);
rg_puntero(RG_MUESTRA);

if(err!=AA_SUCCESS) return(FALSE);

return(TRUE);
}

/****************************************************************************
	MOUSE: activa/desactiva el rat¢n.
	  Entrada:      'modo' 0 desactiva, 1 activa
****************************************************************************/
BOOLEAN mouse(BYTE modo)
{

if(modo==0) {
	rg_desconecta();
	clear(10);
}
else if(rg_inicializa()) set(10);

return(TRUE);
}

/****************************************************************************
	MOUSEXY: comprueba coordenadas del rat¢n.
	  Entrada:      'fil0', 'col0' esquina superior izquierda
			'fil1', 'col1' esquina inferior derecha
	  Salida:       TRUE si rat¢n se encuentra dentro del †rea definida
			por las coordenadas, FALSE si no o rat¢n desactivado
****************************************************************************/
BOOLEAN mousexy(BYTE fil0, BYTE col0, BYTE fil1, BYTE col1)
{
STC_RATONG r;

/* sale si rat¢n desactivado */
if(zero(10)) return(FALSE);

rg_estado(&r);

if(((BYTE)r.fil>=fil0) && ((BYTE)r.fil<=fil1) && ((BYTE)r.col>=col0) &&
  ((BYTE)r.col<=col1)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	MOUSEBT: comprueba estado de botones del rat¢n.
	  Entrada:      'btn' bot¢n a comprobar
			 0 - izquierdo
			 1 - derecho
			 2 - ambos
			 3 - cualquiera de los dos
	  Salida:       TRUE si bot¢n/es pulsado/s, FALSE si no o rat¢n
			desactivado
****************************************************************************/
BOOLEAN mousebt(BYTE btn)
{
STC_RATONG r;

/* sale si rat¢n desactivado */
if(zero(10)) return(FALSE);

rg_estado(&r);

if((btn==0) && r.boton1) return(TRUE);
else if((btn==1) && r.boton2) return(TRUE);
else if((btn==2) && r.boton1 && r.boton2) return(TRUE);
else if((btn==3) && (r.boton1 || r.boton2)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	PLAY: toca una nota.
	  Entrada:      'nota' n£mero de nota a tocar
			'dur' duraci¢n en dÇcimas de segundo
****************************************************************************/
BOOLEAN play(BYTE nota, BYTE dur)
{

if(nota<NUM_NOTAS) bpr_nota(nota,dur);

return(TRUE);
}

/****************************************************************************
	MUSIC: reproduce una melod°a de un fichero SND.
	  Entrada:      'mus' n£mero de la melod°a
			'modo' modo de reproduci¢n
			 0 - s¢lo una vez
			 1 - c°clicamente
	  Salida:       TRUE si pudo reproducir la melod°a
			FALSE si hubo alg£n error
	  NOTA: las melod°as deben estar en ficheros cuyo nombre sea
	  M#??????.ext siendo 'ext' el n£mero identificador de 1 a 255,
	  por ejemplo: M#INIC.1, M#MUSI.203, ...
****************************************************************************/
BOOLEAN music(BYTE mus, BYTE modo)
{
struct ffblk info;
char musica[8]="M#*.", num[4];
int err;

/* comprueba si modo es v†lido, si no le asigna 0 */
if(modo>1) modo=0;

/* construye nombre de fichero a?adiendo extensi¢n */
itoa((int)mus,num,10);
strcat(musica,num);

/* busca el fichero y sale si no lo encuentra */
/* el nombre verdadero del fichero estar† en 'info.ff_name' */
if(findfirst(musica,&info,0)) return(FALSE);

if(modo==0) err=bpr_musica(info.ff_name,SND_SOLO);
else err=bpr_musica(info.ff_name,SND_MULT);

if(err) return(FALSE);

return(TRUE);
}
