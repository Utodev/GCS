/*****************************************************************************
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
#include "vv.h"
#include "grfpcx.h"
#include "sonido.h"
#include "condacto.h"

/*** Variables externas ***/
#if DEBUGGER==1
extern BOOLEAN debugg;          /* TRUE si paso a paso activado */
extern BOOLEAN pra_lin;         /* TRUE si en primera línea de una entrada */
extern STC_VV vv_deb;           /* ventana de debugger */
extern unsigned char far *img_debug;    /* puntero buffer para fondo debug. */
#endif


extern CAB_DAAD cab;          /* cabecera de fichero de base de datos */
extern char nf_base_datos[MAXPATH];     /* nombre de fichero base de datos */
extern BYTE *ptr_proc;          /* puntero auxiliar */
extern unsigned sgte_ent;       /* desplazamiento de sgte. entrada */
extern BYTE pro_act;            /* número de proceso actual */
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
extern BYTE flag[FLAGS];          /* variables del sistema (8 bits) */
extern int modovideo;           /* modo de vídeo */
extern BYTE num_cols;				/* número de columnas del modo */
extern BYTE num_rows;				/* número de filas del modo */

/*** Variables globales ***/
char Car_Pal[]="ABCDEFGHIJKLMN¥OPQRSTUVWXYZ0123456789";
char C_No_Sig[]=" !???";        /* caracteres no significativos */
char C_Separ[]=".,;:\"\'";	/* separadores */
int ptrp;                       /* puntero de pila */
BYTE *pila1[STK];               /* para guardar direcciones de retorno */
unsigned pila2[STK];            /*   "     "    deplz. sgte. entrada */
BYTE pila3[STK];                /*   "     "    num. de proc. de llamada */
STC_VV w[N_VENT];               /* tabla para guardar parámetros de ventanas */
BYTE loc_obj[MAX_OBJ];          /* tabla de localidades act. de objetos */
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




DAAD_CONDACTO cd[]={            
1,at, /*  0 $00 */
1,notat, /*  1 $01 */
1,atgt, /*  2 $$02 */
1,atlt, /*  3 $03 */
1,present, /*  4 $04 */
1,absent, /*  5 $05 */
1,worn, /*  6 $06 */
1,notworn, /*  7 $07 */
1,carried, /*  8 $08 */
1,notcarr, /*  9 $09 */
1,chance, /* 10 $0a */
1,zero, /* 11 $0b */
1,notzero, /* 12 $0c */
2,eq, /* 13 $0d */
2,gt, /* 14 $0e */
2,lt, /* 15 $0f */
1,adject1, /* 16 $10 */
1,adverb, /* 17 $11 PENDIENTE */
2,sfx, /* 18 $12 */
1,desc, /* 19 $13 */
0,quit, /* 20 $14 */
0,end1, /* 21 $15 */
0,done, /* 22 $16 PENDIENTE DE REPASAR*/
0,ok, /* 23 $17 */
0,anykey, /* 24 $18 */
1,save, /* 25 $19 */
1,load, /* 26 $1a */
1,dprint, /* 27 $1b */
1,display, /* 28 $1c  PENDIENTE, NO IMPLEMENTADO */
0,cls, /* 29 $1d */
0,dropall, /* 30 $1e */
0,autog, /* 31 $1f */
0,autod, /* 32 $20 */
0,autow, /* 33 $21 */
0,autor, /* 34 $22 */
1,pause, /* 35 $23 */
2,synonym, /* 36 $24  FALTA VER SI ESTE Y OTROS ACTUALIZAN EL STATUS DE "DONE" */
1,goto1, /* 37 $25 */
1,message, /* 38 $26 */
1,remove1, /* 39 $27 */
1,get, /* 40 $28 */
1,drop, /* 41 $29 */
1,wear, /* 42 $2a */
1,destroy, /* 43 $2b */
1,create, /* 44 $2c */
2,swap, /* 45 $2d */
2,place, /* 46 $2e */
1,set, /* 47 $2f */
1,clear, /* 48 $30 */
2,plus, /* 49 $31 */
2,minus, /* 50 $32 */
2,let, /* 51 $33 */
0,newline, /* 52 $34 */
1,print, /* 53 $35 */
1,sysmess, /* 54 $36 */
2,isat, /* 55 $37 */
1,setco, /* 56 $38  COMPROBAR QUE REALMENTE HAY QUE ACTUALIZAR TODOS LOS FLAGS Y NO SOLO EL 51*/
0,space, /* 57 $39 */
1,hasat, /* 58 $3a */
1,hasnat, /* 59 $3b */
0,listobj, /* 60 $3c */  
2,extern1, /* 61 $3d */
0,ramsave, /* 62 $3e */
1,ramload, /* 63 $3f */
2,beep, /* 64 $40 */
1,paper, /* 65 $41 */
1,ink, /* 66 $42 */
1,border, /* 67 $43 */
1,prep, /* 68 $44 FALTA IMPLEMENTAR*/
1,noun2, /* 69 $45 */
1,adject2, /* 70 $46 */
2,add, /* 71 $47 */
2,sub, /* 72 $48 */
1,parse, /* 73 $49 pendiente de juntar cin INPUT*/
1,listat, /* 74 $4a */
1,process, /* 75 $4b falta poner todo el sistema de pila, doalls, etc.*/
2,same, /* 76 $4c */
1,mes, /* 77 $4d */
1,window, /* 78 $4e */
2,noteq, /* 79 $4f */
2,notsame, /* 80 $50 */
1,mode, /* 81 $51 PENDIENTE PORQUE NO TIENE NADA QUE VER CON EL MODO DE VIDEO*/
2,winat, /* 82 $52 */  
2,time, /* 83 $53 */
1,picture, /* 84 $54 FALTA IMPLEMENTAR*/
1,doall, /* 85 $55  FALTA */
1,mouse, /* 86 $56 FALTA: ESTá IMPLEMENTADA como que si el parámetro es 1 se activa y si no se apaga, documentar*/
2,gfx, /* 87 $57 */
2,isnotat, /* 88 $58 */
2,weigh, /* 89 $59 */
2,putin, /* 90 $5a FALTA */
2,takeout, /* 91 $5b FALTA */
0,newtext, /* 92 $5c */
2,ability, /* 93 $5d */
1,weight, /* 94 $5e */
1,random, /* 95 $5f */
2,input, /* 96 $60 PENDIENTE */
0,saveat, /* 97 $61 */
0,backat, /* 98 $62 */
2,printat, /* 99 $63 */
0,whato, /*100 $64 */
1,call, /*101 $65 PENDIENTE*/
1,puto, /*102 $66 */
0,notdone, /*103 $67  FALTA PORQUE NO ESTA HACIENDO EL NOTDONE DE DAAD*/
1,autop, /*104 $68 FALTA*/
1,autot, /*105 $69 FALTA*/
1,move, /*106 $6a PENDIENTE*/
2,winsize, /*107 $6b */
0,redo, /*108 $6c */
0,centre, /*109 $6d */
1,exit1, /*110 $6e */
0,inkey, /*111 $6f */
2,bigger, /*112 $70 */
2,smaller, /*113 $71 */
0,isdone, /*114 $72 */
0,isndone, /*115 $73 */
1,skip, /*116 $74 FALTA IMPLEMENTAR*/
0,restart, /*117 $75 */
1,tab, /*118 $76 */
2,copyof, /*119 $77 */
0,dumb1, /*120 $78 */
2,copyoo, /*121 $79  */
0,dumb2, /*122 $7a */
2,copyfo, /*123 $7b */
0,dumb3, /*124 $7c */
2,copyff, /*125 $7d */
2,copybf, /*126 $7e */
0,reset  /*127 $7f FALTA */
};

/*** Prototipos de funciones externas ***/
void modo_video(int modo);
void codifica(BYTE *mem, unsigned bytes_mem);
void inic(void);
char mayuscula(char c);
void m_err(BYTE x, char *m, int flag);


/****************************************************************************
	setflag: Asigna un valor a un flag
****************************************************************************/

void setflag(BYTE flagno, BYTE value)
{
	flag[flagno] = value;
}


/****************************************************************************
	setflagbit: Activa un bit de un flag
****************************************************************************/
void setflagbit(BYTE nflagnof, BYTE nb)
{
	setflag(flagno, getflag(flagno) | (1 << nb));
}

/****************************************************************************
	clearflagbit: Desactiva un bit de un flag
****************************************************************************/
void clearflagbit(BYTE flagno, BYTE nb)
{
	setflag(flagno, getflag(flagno) & (255 - (1 << nb)));
}


/****************************************************************************
	getflagbit: Obtiene el valor de un bit de un flag 
****************************************************************************/
BOOLEAN getflagbit(BYTE flagno, BYTE nb)
{
	if (getflag(flagno) & (1 << nb)) return(TRUE); else return(FALSE);
}


/****************************************************************************
	getflag: Obtiene el valor de un flag
****************************************************************************/

BYTE getflag(BYTE flagno)
{
	return flag[flagno];
}


/****************************************************************************
	VV_IMPS2: imprime una cadena en una ventana. Las palabras de final de
	  línea que no caben dentro de la ventana se pasan a la línea
	  siguiente. Esta versión de vv_imps tiene en cuenta el carácter de
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
	/* si se encontró fin de frase, espacio o avance de línea */
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
			/* queda de línea pero cabe en la siguiente, */
			/* la imprime en la siguiente línea, si no la */
			/* imprime sin más */
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
			/* si letra no es espacio ni avance de línea */
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
		/* si encontró símbolo de subrayado */
		else {
			/* se salta el símbolo de subrayado */
			s++;
			/* puntero a texto objeto */
			pto=tab_obj+tab_desp_obj[getflag(FRFOBJ)]+6;
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
	VV_INPUTT: permite introducir por teclado una línea de caracteres con
	  tiempo límite de introducción.
	  Sólo admite los caracteres que contenga la cadena 'Cod_Teclas'
	  (global).
	  Entrada:      'plin' puntero a buffer dónde se almacenará la
			cadena introducida
			'maxlong' máximo número de caracteres permitidos;
			deberá ser menor o igual a VVINP_MAXLIN
			'cursor' carácter a usar como cursor
			'conver' indica si la cadena tecleada debe ser
			convertida a mayúsculas (VVINP_CONV para convertir,
			VVINP_NOCONV para dejar como se tecleó)
			'v' puntero a ventana
			'tiempo' tiempo límite (segundos) para teclear
			la frase, si es 0 no habrá tiempo límite
			'modot' modo de funcionamiento del temporizado;
			si es 1 el tiempo se cuenta hasta que teclea la
			primera letra, si es 0 tiene el tiempo dado para
			teclear toda la frase
	  Salida:       código de tecla de función pulsada, 0 si no se
			pulsó ninguna, 1 si se acabó por falta de tiempo
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

/* guarda posición del cursor y habilita scroll de ventana */
antx=v->cvx;
anty=v->cvy;
v->scroll=1;

/* modifica anchura interna de ventana para evitar problemas con cursor */
antlxi=v->lxi;
v->lxi-=2;

/* puntero de la posición del cursor */
pcur=plin;

/* puntero al final de la línea */
pfinl=plin;
long_lin=1;

/* marca final de línea */
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
	/* carácter tecleado y es el primer carácter, y además el tiempo */
	/* límite no es 0 */
	if(((modot==0) || ((modot==1) && primer_car)) && tiempo) {
		/* hasta que pulse una tecla mira el tiempo que ha pasado */
		do {
			/* coge contador de reloj actual */
			tactual=biostime(0,0L);
			segundos=(int)((tactual-tinicio)/TICKS_SEG);

			/* si se ha pasado el tiempo, sale indicándolo */
			if(segundos>tiempo) {
				/* reimprime la línea de input para borrar */
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
				/* desaparición del cursor */
				vv_impc('\xff',v);

				/* copia línea en buffer */
				for(pcur=plin, i=buf_lin; *pcur; pcur++, i++) {
					*i=*pcur;
				}
				*i='\0';

				if(conver==VVINP_CONV) {
					/* convierte línea a mayúsculas */
					i=plin;
					while(*i) {
						*i=mayuscula(*i);
						i++;
					}
				}

				/* si no se produjo scroll en ventana */
				/* mientras entrada de datos y la posición */
				/* del cursor es menor que la última línea */
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

	/* indica que ha tecleado ya el primer carácter */
	primer_car=0;

	do {
		c=vv_lee_tecla();
	} while((c==COD_RAT) && !getflagbit(FFLAGS, FFMOUSEON));
	switch(c) {
		case BACKSPACE :        /* borrar carácter a izquierda */
			/* comprueba que cursor no esté a la izquierda */
			if(pcur!=plin) {
				pcur--;
				for(i=pcur; i<pfinl; i++) *i=*(i+1);
				pfinl--;
				long_lin--;
			}
			break;
		case COD_SUP :          /* borrar carácter a derecha */
			/* comprueba que cursor no esté al final */
			if(pcur!=pfinl) {
				for(i=pcur; i<pfinl; i++) *i=*(i+1);
				pfinl--;
				long_lin--;
			}
			break;
		case COD_IZQ :          /* mover cursor izquierda */
			/* comprueba que no esté a la izquierda */
			if(pcur!=plin) pcur--;
			break;
		case COD_DER :          /* mover cursor derecha */
			/* comprueba que no esté a la derecha */
			if(pcur!=pfinl) pcur++;
			break;
		case COD_ARR :          /* mover cursor arriba */
			/* si se pasa del inicio, copia línea anterior */
			/* y sitúa cursor al final */
			if(pcur==plin) {
				/* borra línea anterior */
				if(v->scroll!=1)
				  v->cvy=anty-((v->scroll-1)*v->chralt);
				else v->cvy=anty;
				v->cvx=antx;
				/* imprime con color de fondo */
				color=v->colort;
				v->colort=v->colortf;
				for(i=plin; *i; i++) vv_impc(*i,v);
				v->colort=color;

				/* por si cursor está al final */
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
		/* si pulsa una tecla de función o RETURN */
		/* acaba la entrada y devuelve su código (tecla función) */
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
			/* inserta carácter si no se excede longitud máxima */
			if(long_lin==maxlong) break;
			if(pcur!=pfinl) for(i=pfinl; i>pcur; i--) *i=*(i-1);
			*pcur++=(char)c;
			*(++pfinl)='\0';
			long_lin++;
			break;
	}
	/* si al teclear algo hubo scroll en ventana calcula línea */
	/* de inicio del cursor de acuerdo al número de líneas */
	/* 'scrolleadas' (v->scroll), sino coge la antigua posición */
	/* del cursor 'anty' */
	if(v->scroll!=1) v->cvy=anty-((v->scroll-1)*v->chralt);
	else v->cvy=anty;
	v->cvx=antx;

	i=plin;
	do {
		curx=v->cvx+(v->vxi*8);
		cury=v->cvy+(v->vyi*v->chralt);

		/* imprime carácter */
		if(*i) {
			g_rectangulo(curx,cury,curx+15,cury+v->chralt-1,
			  v->colortf,G_NORM,1);
			vv_impc(*i,v);
		}

		/* si es posición de cursor y no está al final de */
		/* la línea, lo imprime */
		if((i==pcur) && (pcur!=pfinl)) {
			/* si ha cambiado de línea */
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
			/* imprime carácter */
			imp_chr_pos(curx,cury);
			imp_chr(*i,0,v->colort,CHR_OR);
			/* imprime cursor sobre carácter */
			imp_chr_pos(curx,cury);
			imp_chr(cursor,0,v->colort,CHR_XOR);
			rg_puntero(RG_MUESTRA);
		}

		/* siguiente carácter */
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

	/* imprime un espacio al final (por si se borró un carácter) */
	vv_impc('\xff',v);
	vv_impc('\xff',v);

} while((c!=RETURN) && (c!=COD_RAT) && (c<COD_F10));

/* reimprime la línea de input para borrar el cursor */
/* posiciona el cursor para imprimir */
if(v->scroll!=1) v->cvy=anty-((v->scroll-1)*v->chralt);
else v->cvy=anty;
v->cvx=antx;

i=plin;
do {
	if(*i) vv_impc(*i,v);
	i++;
} while(i<=pfinl);
/* a?ade espacio al final para compensar la desaparición del cursor */
vv_impc('\xff',v);

/* copia línea en buffer */
for(pcur=plin, i=buf_lin; *pcur; pcur++, i++) *i=*pcur;
*i='\0';

if(conver==VVINP_CONV) {
	/* convierte línea a mayúsculas */
	i=plin;
	while(*i) {
		*i=mayuscula(*i);
		i++;
	}
}

/* si no se produjo scroll en ventana mientras entrada de datos */
/* y la posición del cursor es menor que la última línea de la ventana */
/* desactiva indicador de scroll */
if((v->scroll==1) && (v->cvy<((v->lyi-1)*v->chralt))) v->scroll=0;

v->lxi=antlxi;

/* si terminó la línea pulsando RETURN devuelve 0, si no devuelve */
/* el código de la tecla (o botón del ratón) con que se terminó */
if(c==RETURN) return(0);
else return(c);

}

/****************************************************************************
	ESTA_EN: comprueba si un carácter está en una cadena.
	  Entrada:      's' cadena con la que se comprobará el carácter
			'c' carácter a comprobar
	  Salida:       TRUE si el carácter está en la cadena
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
	ESTA_EN_VOC: comprueba si una palabra está en el vocabulario.
	  Entrada:      'vocab' puntero a tabla de vocabulario
			'pal_voc' número de palabras en vocabulario
			'pal' puntero a palabra a buscar
	  Salida:       posición dentro del vocabulario si se encontró, si no
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
	  Salida:       'tipo', 'num' tipo y número de la palabra en vocabul.
			SEPARADOR si encontró un separador
			FIN_FRASE si encontró final de la frase
			PALABRA si encontró palabra válida de vocabulario
			TERMINACION como PALABRA pero si además encontró una
			terminación en LA, LE o LO
			NO_PAL si encontró palabra pero no está en
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

	/* si hay otro separador detrás lo salta también */
	while(esta_en(C_Separ,*(*pfrase))) {
		(*pfrase)++;
		/* salta caracteres no significativos */
		for(; esta_en(C_No_Sig,*(*pfrase)); (*pfrase)++);
	}
	/* si detrás encuentra final de la frase, sale indicándolo */
	/* si no, sale indicando que encontró separador */
	if(!*(*pfrase)) return(FIN_FRASE);
	else return(SEPARADOR);
}

/* si encontró fin de frase, lo indica */
if(!*(*pfrase)) return(FIN_FRASE);

/* salta espacios anteriores a palabra */
for(; *(*pfrase)==' '; (*pfrase)++);
/* al llegar aquí *(*pfrase) será [A-Z]+¥+[0-9] */
/* repite mientras no llene palabra y encuentre carácter válido de palabra */
do {
	/* mete carácter en palabra */
	palabra[i]=*(*pfrase);
	(*pfrase)++;
	i++;
} while((i<LONGPAL) && (esta_en(Car_Pal,*(*pfrase))));

/* rellena con espacios y marca fin de palabra */
for(; i<LONGPAL; i++) palabra[i]=' ';
palabra[i]='\0';

/* desprecia resto de palabra */
while(esta_en(Car_Pal,*(*pfrase))) (*pfrase)++;

/* comprobamos si la palabra está en vocabulario */
*num=0;
*tipo=0;
if((i=esta_en_voc(palabra))==NUM_PAL+1) return(NO_PALABRA);

/* si está en vocabulario coge su número y su tipo */
*num=vocabulario[i].num;
*tipo=vocabulario[i].tipo;

/* si es una conjunción indica que encontró un separador */
if(*tipo==_CONJ) return(SEPARADOR);

/* si termina en LA, LE o LO indica palabra con terminación */
if((*((*pfrase)-2)=='L') && ((*((*pfrase)-1)=='A') || (*((*pfrase)-1)=='E') ||
  (*((*pfrase)-1)=='O'))) return(TERMINACION);
/* lo mismo si termina en LAS, LES o LOS */
else if((*((*pfrase)-1)=='S') && (*((*pfrase)-3)=='L') &&
  ((*((*pfrase)-1)=='A') || (*((*pfrase)-1)=='E') || (*((*pfrase)-1)=='O')))
  return(TERMINACION);
/* en cualquier otro caso indica que encontró palabra */
else return(PALABRA);

}

/****************************************************************************
	PARSE1: analiza la línea principal de entrada hasta un separador o
	  hasta el fin de línea.
	  Entrada:      'frase' conteniendo la línea a analizar
	  Salida:       TRUE si no hay más que analizar
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

nombrelo=getflag(FNOUN);        /* guarda nombre para verbo con terminación */
adjtlo=getflag(FADJECT);          /* guarda adjetivo para verbo con terminación */
verbo=getflag(FVERB);           /* guarda verbo por si teclea frase sin él */

/* inicializa variables de sentencia lógica */
for(i=2; i<7; i++) setflag(i,NO_PAL);

/* repite hasta fin de frase o separador */
do {
	/* analiza una palabra */
	res=analiza(&lin,&tipo,&num);

	/* si es palabra sin terminación */
	if(res==PALABRA) {
		/* si es verbo y no cogió ninguno, lo almacena */
		if((tipo==_VERB) && f[_VERB]!=1) {
			setflag(FVERB,num);
			f[_VERB]=1;
		}
		/* sólo coge los dos primeros nombres */
		if((tipo==_NOMB) && (f[_NOMB]<2)) {
			/* almacena nombre 1 en variable 3 */
			if(!f[_NOMB]) {
				setflag(FNOUN,num);
				/* si es nombre convertible también lo */
				/* almacena en verbo si no cogió antes */
				/* ningún verbo ni otro nombre conv. */
				if((num<cab.n_conv) && !f[_VERB]) {
					setflag(FVERB,num);
					/* indica que hay nombre convert. */
					f[_VERB]=2;
					/* luego lo incrementará */
					f[_NOMB]--;
				}
			}
			/* almacena nombre 2 en variable 5 */
			if(f[_NOMB]==1) setflag(FNOUN2,num);
			/* incrementa número de nombres cogidos */
			f[_NOMB]++;
		}
		/* sólo coge los dos primeros adjetivos */
		if((tipo==_ADJT) && (f[_ADJT]<2)) {
			/* almacena adjetivo 1 en variable 4 */
			if(!f[_ADJT]) setflag(FADJECT,num);
			/* almacena adjetivo 2 en variable 6 */
			if(f[_ADJT]==1) setflag(FADJECT2,num);
			/* incrementa número de adjetivos cogidos */
			f[_ADJT]++;
		}
	}
	/* si es palabra con terminación */
	if(res==TERMINACION) {
		if((tipo==_VERB) && f[_VERB]!=1) {
			setflag(FVERB,num); /* almacena número verbo en variable 2 */
			f[_VERB]=1; /* indica que ya ha cogido el verbo */
			/* si nombre anterior no era Propio */
			/* recupera el nombre anterior y su adjetivo */
			if(nombrelo>=cab.n_prop) {
				setflag(FNOUN,nombrelo);
				setflag(FADJECT,adjtlo);
				f[_NOMB]++;
				f[_ADJT]++;
			}
		}
		/* sólo coge los dos primeros nombres */
		if((tipo==_NOMB) && (f[_NOMB]<2)) {
			/* almacena nombre 1 en variable 3 */
			if(!f[_NOMB]) {
				setflag(FNOUN,num);
				/* si es nombre convertible también lo */
				/* almacena en verbo si no cogió antes */
				/* ningún verbo ni otro nombre conv. */
				if((num<cab.n_conv) && !f[_VERB]) {
					setflag(FVERB,num);
					/* indica que hay nombre convertible */
					f[_VERB]=2;
					/* luego lo incrementará */
					f[_NOMB]--;
				}
			}
			/* almacena nombre 2 en variable 5 */
			if(f[_NOMB]==1) setflag(FNOUN2,num);
			/* incrementa número de nombres cogidos */
			f[_NOMB]++;
		}
		/* sólo coge los dos primeros adjetivos */
		if((tipo==_ADJT) && (f[_ADJT]<2)) {
			/* almacena adjetivo 1 en variable 4 */
			if(!f[_ADJT]) setflag(FADJECT,num);
			/* almacena adjetivo 2 en variable 6 */
			if(f[_ADJT]==1) setflag(FADJECT2,num);
			/* incrementa número de adjetivos cogidos */
			f[_ADJT]++;
		}
	}
} while((res!=FIN_FRASE) && (res!=SEPARADOR));

/* si tecleó una frase sin verbo pero con nombre, pone el verbo anterior */
if(!f[_VERB] && f[_NOMB]) setflag(FVERB,verbo);

/* si es fin de frase mira si analizó o no lo último cogido */
if(res==FIN_FRASE) {
	if(mas_texto==FALSE) {
		mas_texto=TRUE;         /* indicador para siguiente llamada */
		return(FALSE);          /* indica que analice lo último */
	}
	else return(TRUE);              /* no queda más por analizar */
}

/* si es separador, supone que hay más texto detrás */
return(FALSE);
}

/****************************************************************************
	PAUSA: realiza una pausa. Si se pulsa una tecla, sale de la pausa,
	  pero no saca la tecla del buffer de teclado.
	  Entrada:      'p' tiempo de la pausa en décimas de segundo,
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

/* comprueba la versión del fichero */
if(cad_recon[LONG_RECON_F-1]!=recon_fuente[LONG_RECON_F-1]) {
	fclose(ffuente);
	return(0);
}

/* si la versión ha sido válida lee las definiciones de los caracteres */
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
	  Entrada:      'nt' número de tabla a cargar
	  Salida:       0 si tabla no existe o hubo error, un valor distinto
			de 0 en otro caso
****************************************************************************/
int carga_tabla_mes(BYTE nt)
{
FILE *fbd;

/* si tabla no existe, sale */
if(cab.fpos_msg[nt]==(fpos_t)0) return(0);

/* si tabla ya está cargada, sale */
/* G3.30: lo quitamos a ver si se quita el "bug" de los mensajes descontrolados */
/*** if(nt==tabla_msg) return(1); */

/* carga tabla de mensajes */
if((fbd=fopen(nf_base_datos,"rb"))==NULL) return(0);

fseek(fbd,cab.fpos_msg[nt],SEEK_SET);
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


return(1);
}


/****************************************************************************
	ISCONTAINER: Devuelve si un objeto es contenedor
	  Entrada: 'objno' numero de objeto
	  Salida: 1 si lo es, 0 si no
****************************************************************************/
BOOLEAN iscontainer(BYTE objno)
{
	BYTE std_attrs = getDDBByte(cab.obj_weight_cont_wear_pos + objno);
	if (std_attrs & 0x40) return (1); 
	return (0);
	
}

/****************************************************************************
	ISWEARABLE: Devuelve si un objeto es prenda
	  Entrada: 'objno' numero de objeto
	  Salida: 1 si lo es, 0 si no
****************************************************************************/
BOOLEAN iswearable(BYTE objno)
{
	BYTE std_attrs = getDDBByte(cab.obj_weight_cont_wear_pos + objno);
	if (std_attrs & 0x80) return (1); 
	return (0);
	
}


/****************************************************************************
	GETOBJECTWEIGHT: Devuelve el peso de un objeto
	  Entrada: 'objno' numero de objeto
	  Salida: el peso 
****************************************************************************/
BOOLEAN getobjectweight(BYTE objno)
{
	BYTE std_attrs = getDDBByte(cab.obj_weight_cont_wear_pos + objno);
	return (std_attrs & 0x3F);
	
}

/****************************************************************************
	GETREALOBJECTWEIGHT: Devuelve el peso de un objeto y si es contenedor
						  de todo lo que haya dentro recursivamente
	  Entrada: 'objno' numero de objeto
	  Salida: el peso 
****************************************************************************/
BOOLEAN getrealobjectweight(BYTE objno)
{
	BYTE std_attrs = getDDBByte(cab.obj_weight_cont_wear_pos + objno);
	WORD weight = std_attrs & 0x3F;
	if (iscontainer(objno) && weight) /* container objects with weight=0 are infinite containers, they can hold as many items as player wants) */
	  for (int i=0;i<cab.max_obj;i++)
	    if (loc_obj[i]==objno) weight += getrealobjectweight(i);

	if (weight > 255) return(255); else return(weight);
}

/****************************************************************************
	DEFWIN: define una ventana.
	  Entrada:      'nw' número de ventana
			'cwf, 'cw' colores de fondo y primer plano
			'wy', 'wx' posición de esquina superior izquierda
			(fila, columna) de la ventana; si 255 se centra en
			pantalla
			'lx', 'ly' tama?o (ancho y alto) de la ventana; si
			255 se toman las máximas dimensiones
****************************************************************************/
BOOLEAN defwin(BYTE nw, BYTE cwf, BYTE cw, BYTE wy, BYTE wx, BYTE lx, BYTE ly)
{

	/* ajusta dimensiones para que ocupe toda la pantalla */
	if(lx==255) lx=num_cols;
	if(ly==255) ly=num_rows;

	/* ajusta posición para centrar ventana */
	if(wx==255) wx=(BYTE)((num_cols-lx)/2);
	if(wy==255) wy=(BYTE)((num_rows-ly)/2);

	/* ajusta para que entre en pantalla */
	if(wx>(BYTE)((num_cols)-1)) wx=(BYTE)((num_cols)-1);
	if(wy>(BYTE)((num_rows)-1)) wy=(BYTE)((num_rows)-1);
	if((wx+lx)>(num_cols)) lx=(num_cols)-wx;
	if((wy+ly)>(num_rows)) ly=(num_rows)-wy;

	vv_crea(wy,wx,lx,ly,cwf,cw,NO_BORDE,&w[nw]);

	return(TRUE);
}



/*-------------------------------------------------------------------------*/
/* FUNCIONES PARA LOS CONDACTOS                                            */
/*-------------------------------------------------------------------------*/

/****************************************************************************
	WEIGH: calcula el peso real de un objeto, con sus contenedores, y lo pone en un flag
	  Entrada: 'objno' el objeto
	           'flagno' número de flag
****************************************************************************/
BOOLEAN weigh(BYTE objno, BYTE flagno)
{
	setflag(flagno, getrealobjectweight(objno));
}

/****************************************************************************
	WEIGHT: calcula el peso real de los objetos que lleva el jugador y lo pone 
	        en un flag, nunca superará MAX_VAL.
	  Entrada: 'flagno' número de flag
****************************************************************************/
BOOLEAN weight(BYTE flagno)
{
	WORD w = 0;
	for (int i=0;i<cab.num_obj;obj++)
	  if ((loc_obj[i]==LLEVADO) || (loc_obj[i]==COGIDO))
	  {
	   w += getrealobjectweight(i);
	   if (w > MAX_VAL) 
	   {
		   setflag(flagno, MAX_VAL);
		  return(TRUE);
	   }
	  }
	  setflag(nf, w & 0xFF);
	  return(TRUE);
}


/****************************************************************************
	RANDOM: pone un valor aleatorio del 1 al 100 en el flag indicado
	  Entrada: 'flagno' número de flag
****************************************************************************/
BOOLEAN random(BYTE flagno)
{
	setflag(flagno, random(100)+ 1);
	return(TRUE);
}


/****************************************************************************
	CALL: llama a una subrutina ensambladorm, pero no está soportado
	  Entrada: 'lsb', 'msb' : address to call, pero no está soportado
****************************************************************************/
BOOLEAN call(BYTE lsb, BYTE msb)
{
	return(TRUE);
}

/****************************************************************************
	REDO: reinicia el proceso en curso
****************************************************************************/
BOOLEAN redo(void)
{
	ptr_entry = getDDBWord(ptr_proc) - 2;
	return(FALSE); /* Esto hace que se salga de la entrada actual y con lo de la linea anterior al avanzar a la siguiente entrada, volverá a estar en la primer */
}

/****************************************************************************
	CENTRE: debería centrar el texto, pero no está soportado
****************************************************************************/
BOOLEAN centre(void)
{
	return(TRUE);
}



/****************************************************************************
	BEEP: emite un sonido por el speaker
	  Entrada: 'dur' duración en milisegundos
	           'frec' frencuencia del sonido
****************************************************************************/
BOOLEAN beep(BYTE dur, BYTE freq)
{
	sound(freq);
	delay(dur);
	nosound();
	return(TRUE);
}


/****************************************************************************
	BORDER: no hace nada en este intérprete
	  Entrada: tiene un parámetro pero no hace nada
****************************************************************************/
BOOLEAN border(BYTE p1)
{
	return(TRUE);
}





/****************************************************************************
	SFX: no hace nada en este intérprete
	  Entrada: tiene dos parámetros pero no hace nada
****************************************************************************/
BOOLEAN sfx(BYTE p1, p2)
{
	return(TRUE);
}


/****************************************************************************
	GFX: no hace nada en este intérprete
	  Entrada: tiene dos parámetros pero no hace nada
****************************************************************************/
BOOLEAN gfx(BYTE p1, p2)
{
	return(TRUE);
}

/****************************************************************************
	DISPLAY: no hace nada en este intérprete
	  Entrada: value
****************************************************************************/
BOOLEAN display(BYTE value)
{
	return(TRUE);
}

/****************************************************************************
	MODE: no hace nada en este intérprete
	Entrada: value
****************************************************************************/
BOOLEAN mode(BYTE value)
{
	return(TRUE);
}

/****************************************************************************
	DROPALL: Deja todos los objetos llevados
	  
****************************************************************************/
BOOLEAN dropall(void)
{	
	for (int i=0;i<cab.num_obj;i++)
	 if (loc_obj[i]==COGIDO) drop(i);
	return(TRUE);
}


/****************************************************************************
	OK : muestra el mensaje "OK" y hace un DONE
****************************************************************************/
BOOLEAN ok(void)
{
	sysmess(15);
	return(done());
}


/****************************************************************************
	PROCESS: ejecuta una llamada a un proceso.
	  Entrada:      'procno' número de proceso
****************************************************************************/
BOOLEAN process(BYTE procno)
{

/* si se rebasa la capacidad de pila interna */
if(ptrp==STK) m_err(3,"Rebosamiento de la pila interna",1);

pila1[ptrp]=ptr_proc+2; /* guarda dir. sgte. condacto en proc. actual */
pila2[ptrp]=sgte_ent;   /*   "    desplazamiento de sgte. entrada */
pila3[ptrp]=pro_act;    /*   "    número de proceso actual */
ptrp++;                 /* incrementa puntero de pila */
pro_act=procno;

/* dirección del proceso llamado - 2 (que se sumará) */
ptr_proc=tab_pro+tab_desp_pro[pro_act]-2;

/* indica que no debe ajustar ptr_proc para siguiente entrada */
nueva_ent=FALSE;

/* saltará a inicio nueva entrada (la primera del proceso llamado) */
return(FALSE);
}

/****************************************************************************
	DONE: finaliza un proceso y continua en siguiente condacto.
****************************************************************************/
BOOLEAN done(void)
{

if(!ptrp) m_err(0,"",1);
ptrp--;
/* recupera dir. sgte condacto en proc. que llamó - 1 (que se sumará luego) */
ptr_proc=pila1[ptrp]-1;

sgte_ent=pila2[ptrp];   /* desplazamiento de la siguiente entrada */
pro_act=pila3[ptrp];    /* número de proceso que llamó */

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


/* PENDIENTE: esto de resp y no resp no es daad */
/****************************************************************************
	RESP: activa comprobación de verbo-nombre al inicio de cada entrada.
****************************************************************************/
BOOLEAN resp(void)
{

resp_act=TRUE;

return(TRUE);
}

/****************************************************************************
	NORESP: desactiva comprobación de verbo-nombre.
****************************************************************************/
BOOLEAN noresp(void)
{

resp_act=FALSE;

return(TRUE);
}


/****************************************************************************
	WINAT: define la esquina superior izquierda de la ventana actual
	  Entrada:  'wy', 'wx' posición de esquina superior izquierda
			(fila, columna) de la ventana; si 255 se centra en
			pantalla
****************************************************************************/
BOOLEAN winat(BYTE line, BYTE col)
{
	defwin(getflag(FACTWINDOW), 0, 7, line, col, 255, 255);
}

/****************************************************************************
	WINSIZE: define ancho y alto de  ventana actual
	  Entrada:  'wy', 'wx' posición de esquina superior izquierda
			(fila, columna) de la ventana; si 255 se centra en
			pantalla
****************************************************************************/
BOOLEAN winat(BYTE height, BYTE width)
{

	BYTE current = getflag(FACTWINDOW);

	w[current].colortf
	
	defwin(current, w[current].colortf, w[current].colort, w[current].y, w[current].x, width, height);
}



/****************************************************************************
	WINDOW: selecciona la ventana activa.
	  Entrada:      'window' número de ventana
****************************************************************************/
BOOLEAN window(BYTE window)
{
	setflag(FACTWINDOW,window);
	return(TRUE);
}

/****************************************************************************
	CLS: borra/inicializa la ventana actual
	  Entrada:      'nw' número de ventana
****************************************************************************/
BOOLEAN cls()
{
	vv_cls(&w[getflag(FACTWINDOW)]);
	return(TRUE);
}

/****************************************************************************
	LET: asigna un valor a un flag.
	  Entrada:      'flagno' número de flag
					'val' valor a asignar
****************************************************************************/
BOOLEAN let(BYTE flagno, BYTE val)
{
	setflag(flagno,val);
	return(TRUE);
}

/****************************************************************************
	EQ: comprueba si una flag es igual a un valor.
	  Entrada:      'flagno' número de variable
					'val' valor
	  Salida:       TRUE si lel flag es igual al valor
					FALSE si es distinto de valor
****************************************************************************/
BOOLEAN eq(BYTE flagno, BYTE val)
{
	if(getflag(flagno)==val) return(TRUE);
	return(FALSE);
}

/****************************************************************************
	NOTEQ: comprueba si un flag distinto de un valor.
	  Entrada:      'flagno' número de variable
					'val' valor
	  Salida:       TRUE si el flag es distinto del valor
					FALSE si es igual al valor
****************************************************************************/
BOOLEAN noteq(BYTE flagno, BYTE val)
{
	return(TRUE-eq(nf,val));
}


/****************************************************************************
	BIGGER: comprueba si una flag es mayor que otro
	  Entrada:      'flagno1' primer flag
					'flagno2' segundo flag
	  Salida:       TRUE si el primer flags tienen  valor mayor que el segundo
					FALSE si no
****************************************************************************/
BOOLEAN bigger(BYTE flagno1, BYTE flagno2)
{
	if(getflag(flagno1)>getflag(flagno2)) return(TRUE);
	return(FALSE);
}

/****************************************************************************
	BIGGER: comprueba si una flag es mayor que otro
	  Entrada:      'flagno1' primer flag
					'flagno2' segundo flag
	  Salida:       TRUE si el primer flags tienen  valor menor que el segundo
					FALSE si no
****************************************************************************/
BOOLEAN bigger(BYTE flagno1, BYTE flagno2)
{
	if(getflag(flagno1)<getflag(flagno2)) return(TRUE);
	return(FALSE);
}

/****************************************************************************
	SMALLER: comprueba si una flag es menor que otro
	  Entrada:      'flagno1' primer flag
					'flagno2' segundo flag
	  Salida:       TRUE si el primer flag tiene valor menor que el segundo
					FALSE si no
****************************************************************************/
BOOLEAN smaller(BYTE flagno1, BYTE flagno2)
{
	if(getflag(flagno1)<getflag(flagno2)) return(TRUE);
	return(FALSE);
}


/****************************************************************************
	SAME: comprueba si una flag es igual otro
	  Entrada:      'flagno1' primer flag
					'flagno2' segundo flag
	  Salida:       TRUE si los flags tienen igual valor
					FALSE si no
****************************************************************************/
BOOLEAN same(BYTE flagno1, BYTE flagno2)
{
	if(getflag(flagno1)==getflag(flagno2)) return(TRUE);
	return(FALSE);
}

/****************************************************************************
	NOTSAME: comprueba si una flag es igual otro
	  Entrada:      'flagno1' primer flag
					'flagno2' segundo flag
	  Salida:       FALSE si los flags tienen igual valor
					TRUE si no
****************************************************************************/
BOOLEAN notsame(BYTE flagno1, BYTE flagno2)
{
	return(TRUE-same(flagno1,flagno2));
}

/****************************************************************************
	COPYFO: la posición del objeto objno es asignada al valor del flag flagno
			Si el flag vale LOC_ACTUAL se coloca en localidad actual. Si la localidad
			no existe aun se asigna.

	  Entrada:      'flagno' número de flag
					'objno' objeto
****************************************************************************/
BOOLEAN copyfo(BYTE flagno, BYTE objno)
{
	if (objno>=cab.num_obj) return(TRUE);

	BYTE old_loc = loc_obj[objno];
	BYTE new_loc;

	new_loc = getFlag(flagno);
	if (new_loc == LOC_ACTUAL) new_loc = getflag(FPLAYER);

	if (new_loc ==  old_loc) return(TRUE);

	if (new_loc == COGIDO) setflag(FCARRIEDOBJ, getflag(FCARRIEDOBJ) + 1);
	if (old_loc == COGIDO) setflag(FCARRIEDOBJ, getflag(FCARRIEDOBJ) - 1);

	loc_obj[objno] = new_loc;

	return(TRUE);
}

/****************************************************************************
	COPYOO: la posición del objeto objno1 es asignada al objeto objno 2

	  Entrada:      'objno1', 'objno2' : los objetos
****************************************************************************/
BOOLEAN copyoo(BYTE objno1, BYTE objno2)
{
	if ((objno1>=cab.num_obj) || (objno2>=cab.num_obj)) return(TRUE);

	BYTE old_loc = loc_obj[objno2];
	BYTE new_loc = loc_obj[objno1];

	if (new_loc ==  old_loc) return(TRUE);

	if (new_loc == COGIDO) plus(FCARRIEDOBJ, 1);
	if (old_loc == COGIDO) minus(FCARRIEDOBJ,1);

	loc_obj[objno] = new_loc;

	return(TRUE);
}


/****************************************************************************
	COPYFF: copya el contenido del primer flag en el segundo
	  Entrada:      'flagno1' número de flag
					'flagno2' segundo flag
****************************************************************************/
BOOLEAN copyff(BYTE flagno1, BYTE flagno2)
{
	setflag(flagno2, getflag(flagno14));
	return(TRUE);
}

/****************************************************************************
	COPYBF: copya el contenido del segundo flag en el primero
	  Entrada:      'flagno1' número de flag
					'flagno2' segundo flag
****************************************************************************/
BOOLEAN copybf(BYTE flagno1, BYTE flagno2)
{
	return copyff(flagno2, flagno1)
}


/****************************************************************************
	LT: comprueba si un flag es menor que un valor.
	  Entrada:      'flagno' número de flag
					'val' valor
	  Salida:       TRUE si el flag es menor que valor
					FALSE si es mayor o igual que valor
****************************************************************************/
BOOLEAN lt(BYTE flagno, BYTE val)
{

if(getflag(flagno)<val) return(TRUE);

return(FALSE);
}

/****************************************************************************
	GT: comprueba si un flag es mayor que un valor.
	  Entrada:      'flagno' número de flag
					'val' valor
	  Salida:       TRUE si el flag es mayor que valor
					FALSE si es menor o igual que valor
****************************************************************************/
BOOLEAN gt(BYTE flagno, BYTE val)
{

if(getflag(flagno)>val) return(TRUE);

return(FALSE);
}

/****************************************************************************
	MES: imprime un mensaje.
	  Entrada:     'nm' número de mensaje
****************************************************************************/
BOOLEAN mes(BYTE mesno)
{
char *pm;
BYTE nt = 0; /* Tabla 0 de mensajes */

/* si tabla es distinta a la que hay cargada, la carga */
if(nt!=tabla_msg) {
	if(!carga_tabla_mes(nt)) return(TRUE);
}

/* comprueba si mensaje existe en tabla, si no sale */
if(nm>=cab.num_msg[nt]) return(TRUE);

pm=tab_msg+tab_desp_msg[mesno];    /* puntero a mensaje */
vv_imps2(pm,&w[getflag(FDARK)]);        /* imprime mensaje en ventana activa */

return(TRUE);
}

/****************************************************************************
	NEWLINE: imprime un avance de línea.
****************************************************************************/
BOOLEAN newline(void)
{

vv_impc('\n',&w[getflag(0)]);

return(TRUE);
}

/****************************************************************************
	MESSAGE: imprime un mensaje con avance de línea.
	  Entrada:      
			'mesno' número de mensaje
****************************************************************************/
BOOLEAN message(BYTE mesno)
{
	mes(mesno);                     /* imprime mensaje */
	newline();                      /* y avance de línea */
	return(TRUE);
}

/****************************************************************************
	SYSMESS: imprime un mensaje del sistema.
	  Entrada:      'nm' número de mensaje
****************************************************************************/
BOOLEAN sysmess(BYTE nm)
{
char *pm;

pm &DDB[cab.pos_sys + 2 * nm]; /* puntero a mensaje */
vv_imps2(pm,&w[getflag(FDARK)]);        /* imprime mensaje en ventana activa */

return(TRUE);
}

/****************************************************************************
	DESC: imprime la descripción de una localidad 
	 	  Entrada:      'locno' número de localidad
****************************************************************************/
BOOLEAN desc(BYTE locno)
{
char *pl;

/* puntero a descripción de localidad */
pl = &DDB[getDDBWord(cab.pos_loc+2*locno)];

/* escribe la descripción */
vv_imps2(pl,&w[getflag(FDARK)]);

return(TRUE);
}

/****************************************************************************
	ADD: suma el valor del flag 1 al flag 2, el tope es 255.
	Entrada:      'flagno1' número de flag
				'flagno2' bnumero de flag 2
****************************************************************************/
BOOLEAN add(BYTE flagno1, BYTE flagno2)
{
	WORD total = getflag(flagno1) + getflag(flagno2);

	if (total < MAX_VAL) setflag(flagno2, total & 0xFF);
					else setflag(flagno2, MAX_VAL);

return(TRUE);
}

/****************************************************************************
	SUB: resta el valor del primer flag al segundo. El tope es 0.
	  Entrada:      'flagno1' número de flag
					'flagno2' número de segundo flag
****************************************************************************/
BOOLEAN sub(BYTE flagno1, BYTE flagno2)
{

if (getflag(flagno2)<=getflag(flagno1)) setflag(flagno2, 0);
		else setflag(flagno2, getflag(flagno1) - getflag(flagno2));

return(TRUE);
}

/****************************************************************************
	PLUS: incrementa en value el valor de un flag. Si pasa de 255 se queda en 
	      255
	  	Entrada: 'flagno' número de flag
				 'value' : valor a incrementar.
****************************************************************************/
BOOLEAN plus(BYTE flagno, BYTE value)
{

if (getflag(flagno) + value > MAX_VAL) setflag(flagno, MAX_VAL);
	else setflag(flagno, getflag(flagno) + value);
return(TRUE);
}

/****************************************************************************
	MINUS: decrementa en value el valor de un flag. Si baja de 0 se queda en 0
	  	Entrada: 'flagno' número de flag
				 'value' : valor a restar.
****************************************************************************/
BOOLEAN minus(BYTE flagno, BYTE value)
{

if (getflag(flagno) <= value) setflag(flagno, 0);
	else setflag(flagno, getflag(flagno) - value);
return(TRUE);
}


/****************************************************************************
	SETCO: pone el objeto referenciado
	  	Entrada: 'objno' objeto referenciado
****************************************************************************/
BOOLEAN setco(BYTE objno)
{
	BYTE std_attrs;
	BYTE user_attrs;

	if (objno >= cab.num_obj) return(TRUE);
	setflag(FRFOBJ, objno);
	setflag(FRFOBJLOC,loc_obj[objno]);
	if (iswearable(objno)) setflag(FRFOBJWEAR,128); else setflag(FRFOBJWEAR,0);
	if (iscontainer(objno)) setflag(FRFOBJCONT,128); else setflag(FRFOBJCONT,0);
	setflag(FRFOBJWEIG, getobjectweight());
	user_attrs = cab.obj_att_pos + 2 * objno;
	setflag(FRFOBJATTRL) = getDDByte(user_attrs);
	setflag(FRFOBJATTRH) = getDDByte(user_attrs + 1);
	
return(TRUE);
}

/****************************************************************************
	PICTURE: pinta una imagen, si no existe la imagen es una condición que falla
	   	  	Entrada: 'picno': número de imagen.
		  			 
****************************************************************************/
BOOLEAN PICTURE(picno)
{
	return(TRUE);
}

/****************************************************************************
	TIME: gestiona el timeout del input
	   	  Entrada: 'duration' en segundos antes de hacer timeout
		  			'opcion': bit 1: solo timeout si no hemos tecleado nada
					          bit 2: puede ocurrir mientras se espera por una tecla en More...
							  bit 3: puede ocurrir en un ANYKEY
****************************************************************************/
BOOLEAN time(BYTE duration, BYTE option)
{
	setflag(FTIMEOUT, duration);
	BYTE timeout_flag = getflag(FTIMEOUTFLG) & 0xF8;
	option = option & 0x07;
	setflag(FTIMEOUTFLG,timeout_flag | option);
	return(TRUE);
}

/****************************************************************************
	SPACE: escribe un espacio
****************************************************************************/
BOOLEAN space(void)
{
	vv_impc(' ',&w[getflag(0)]);
	return(TRUE);
}


/****************************************************************************
	SET: pone a MAX_VAL un flag.
	  Entrada:      'flagno' número de flag
****************************************************************************/
BOOLEAN set(BYTE flagno)
{
	setflag(flagno, MAX_VAL);
	return(TRUE);
}



/****************************************************************************
	CLEAR: pone a 0 un flag.
	  Entrada:   'flagno' número de flag
****************************************************************************/
BOOLEAN clear(BYTE flagno)
{
	setflag(flagno,0);
	return(TRUE);
}


/****************************************************************************
	ZERO: comprueba si un flag es 0.
	  Entrada:      'flagno' número de flag
	  Salida:       TRUE si el flag es 0
					FALSE si el flag no es 0
****************************************************************************/
BOOLEAN zero(BYTE flagno)
{

if (!getflag(flagno)) return (TRUE);
return(FALSE);
}




/****************************************************************************
	NOTZERO: comprueba si una flag es 1.
	  Entrada:      'flagno' número de flag
	  Salida:       TRUE si el flag no es 0 
					FALSE si el flag es 0
****************************************************************************/
BOOLEAN notzero(BYTE flagno)
{
return(TRUE-zero(flagno));
}



/****************************************************************************
	PLACE: coloca un objeto en una localidad.
	  Entrada:      'objno' número de objeto
			'nloc' número de localidad
****************************************************************************/
BOOLEAN place(BYTE objno,BYTE nloc)
{
BYTE locobj;

/* coge la localidad actual del objeto */
locobj=loc_obj[objno];

/* si se refiere a localidad actual sustituye por su valor */
if(nloc==LOC_ACTUAL) nloc=getflag(FPLAYER);

/* si localidad actual de objeto es igual a la de destino, no hace nada */
if(nloc==locobj) return(TRUE);

/* si pasa un objeto a cogido incrementa el número de objetos que lleva */
if (nloc==COGIDO)
  if (getflag(FCARRIEDOBJ)<MAX_VAL) 
    setflag(FCARRIEDOBJ, getflag(FCARRIEDOBJ)+1);

/* si un objeto pasa a no cogido incrementa el número de objetos que lleva */
if ((nloc!=COGIDO) && (locobj==COGIDO)) 
    if (getflag(FCARRIEDOBJ)) 
       setflag(FCARRIEDOBJ, getflag(FCARRIEDOBJ)-1);

/* actualiza posición del objeto */
loc_obj[nobj]=nloc;

return(TRUE);
}

/****************************************************************************
	GET: coge un objeto.
	  Entrada:      'nobj' número de objeto
	  Salida:       TRUE si se pudo coger el objeto
			FALSE si no se pudo coger
	  NOTA: el flag 8 contendrá el número del objeto a la salida
****************************************************************************/
BOOLEAN get(BYTE objno)
{

/* si el objeto no existe */
if(objno>=cab.num_obj) {
	sysmess(26);             /* 'No veo eso por aquí.' */
	return(FALSE);
}

/* coloca el número del objeto en el flag 8 */
setflag(FRFOBJ,objno);

/* si ya tenía el objeto */
if(carried(objno) || worn(objno)) {
	sysmess(25);             /* 'Ya tengo _.' */
	return(FALSE);          /* debe hacer un DONE */
}

/* si el objeto no está presente */
if(loc_obj[objno]!=getflag(FPLAYER)) {
	sysmess(26);             /* 'No veo eso por aquí.' */
	return(FALSE);          /* debe hacer un DONE */
}

/* si lleva muchas cosas */
if((getflag(FCARRIEDOBJ)>=getflag(FABILITY)) && (getflag(FABILITY)!=0)) {
	sysmess(27);     /* 'No puedo llevar nada más.' */
	return(FALSE);  /* debe hacer un DONE */
}

/* coge el objeto */
place(objno,COGIDO);
sysmess(26);             /* He cogido _. */

return(TRUE);
}

/****************************************************************************
	DROP: deja un objeto.
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si se pudo dejar el objeto
			FALSE si no se pudo dejar
	  NOTA: el flag 8 contendrá el número del objeto a la salida
****************************************************************************/
BOOLEAN drop(BYTE objno)
{

/* si el objeto no existe */
if(objno>=cab.num_obj) {
	sysmess(28);                     /* 'No tengo eso.' */
	return(FALSE);
}

/* coloca el número del objeto en el flag 8 */
setflag(FRFOBJ,objno);

/* puesto */
if(wron(objno))) 
{
	sysmess(24);                     /* No puedo. Tengo puesto _. */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si no cogido */
if(notcarr(objno))) 
{
	sysmess(28);                     /* 'No tengo eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si lleva el objeto cogido, lo deja en loc. actual */
place(objno,getflag(FPLAYER));
sysmess(39);                             /* 'He dejado _.' */

return(TRUE);
}


/****************************************************************************
	BORRA_SL: pone todos los flags de vocabulario a NO_PAL
/****************************************************************************/
void borra_SL(void)
{
setflag(FVERB, NO_PAL);
setflag(FNOUN, NO_PAL);
setflag(FADJECT, NO_PAL);
setflag(FNOUN2, NO_PAL);
setflag(FADJECT2, NO_PAL);
setflag(FADVERB, NO_PAL);
setflag(FPREP, NO_PAL);
}


/****************************************************************************
    INPUT: no implementado
****************************************************************************/	
BOOLEAN input(BYTE stream, BYTE option)
{
	return (TRUE);
}


/****************************************************************************
	INPUTPARSE: recoge texto del jugador.
	  Entrada:      variable 16 con el tiempo límite para teclear la
			frase, FTIMEOUT-FFSTART con el modo de temporización
	  Salida:       TRUE si no tecleó nada o se pasó tiempo límite
			FALSE si tecleó algo
	  NOTA: el flag 12 contendrá el código de la tecla de función que
	  se pulsó durante el INPUT
****************************************************************************/
BOOLEAN inputparse(void)
{
int i, modo_temp;
unsigned k;

/* inicializa sentencia lógica */
borra_SL();
ini_inp=TRUE;           /* indica a parse() inicio de frase */
mas_texto=FALSE;        /* usada por parse1() para analizar última frase */

/* modo de temporización de INPUT */
if(getflagbit(FTIMEOUTFLG, FTSTART)) modo_temp=0;
else modo_temp=1;

/* NOTA: el cursor será el primer carácter del mensaje del sistema 7 */
k=vv_inputt(frase,MAXLONG,*(tab_msy+tab_desp_msy[7]),VVINP_CONV,&w[getflag(FDARK)],
  (int)getflag(FTIMEOUT),modo_temp);

/* guarda código con que terminó INPUT si estamos en modo SINTAC*/
if(getflagbit(FFLAGS, 0)) setflag(FINPUT,(BYTE)k);

/* si no tecleó nada o se pasó el tiempo límite */
if(getflagbit(FFLAGS, 0)) /* Si modo SINTAC */
{
	if(!*frase || (getflag(12)==1)) return(TRUE);
}
else
{
	if(!*frase) return(TRUE);
}




return(FALSE);
}

/****************************************************************************
	PARSE: analiza la frase tecleada por jugador.
	  Entrada:      variables globales.-
			  'frase' conteniendo frase tecleada
			  'ini_inp' TRUE si hay que analizar desde principio,
			  FALSE si sigue donde lo dejó en última llamada
	  Salida:       TRUE si se analizó toda la frase
			FALSE si queda más por analizar
****************************************************************************/
BOOLEAN parse(void)
{
BOOLEAN par;

/* si inicio de frase */
if(ini_inp==TRUE) {
	/* coloca puntero al principio de línea tecleada */
	lin=frase;
	ini_inp=FALSE;
}

/* analiza hasta separador o fin línea */
par=parse1();

return(par);
}

/****************************************************************************
	SKIP: salto relativo dentro de un proceso.
	  Entrada:      offset de la entrada
****************************************************************************/
BOOLEAN skip(BYTE offset)
{
int despli;

return(FALSE);          /* saltará a inicio de entrada */
}

/****************************************************************************
	AT: comprueba si está en una localidad.
	  Entrada:      'locno' número de localidad
	  Salida:       TRUE si está en esa localidad
			FALSE si no está en esa localidad
****************************************************************************/
BOOLEAN at(BYTE locno)
{

if(getflag(FPLAYER)==locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTAT: comprueba que no está en una localidad.
	  Entrada:      'locno' número de localidad
	  Salida:       TRUE si no está en esa localidad
			FALSE si está en esa localidad
****************************************************************************/
BOOLEAN notat(BYTE locno)
{

return(TRUE-at(locno));
}

/****************************************************************************
	ATGT: comprueba si está en una localidad superior a la dada.
	  Entrada:      'locno' número de localidad
	  Salida:       TRUE si está en una localidad superior
			FALSE si está en una localidad inferior o igual
****************************************************************************/
BOOLEAN atgt(BYTE locno)
{

if(getflag(FPLAYER)>locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ATLT: comprueba si está en una localidad inferior a la dada.
	  Entrada:      'locno' número de localidad
	  Salida:       TRUE si está en una localidad inferior
			FALSE si está en una localidad superior o igual
****************************************************************************/
BOOLEAN atlt(BYTE locno)
{

if(getflag(FPLAYER)<locno) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ADJECT1: comprueba el primer adjetivo de la sentencia lógica.
	  Entrada:      'adj' número de adjetivo
	  Salida:       TRUE si el adjetivo 1 en la sentencia lógica (var(4))
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN adject1(BYTE adj)
{

if(getflag(FADJECT)==adj) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOUN2: comprueba el segundo nombre de la sentencia lógica.
	  Entrada:      'nomb' número de nombre
	  Salida:       TRUE si el nombre 2 en la sentencia lógica (var(5))
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN noun2(BYTE nomb)
{

if(getflag(FNOUN2)==nomb) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ADJECT2: comprueba el segundo adjetivo de la sentencia lógica.
	  Entrada:      'adj' número de adjetivo
	  Salida:       TRUE si el adjetivo 2 en la sentencia lógica (var(6))
			es el dado
			FALSE si no
****************************************************************************/
BOOLEAN adject2(BYTE adj)
{

if(getflag(FADJECT2)==adj) return(TRUE);

return(FALSE);
}

/****************************************************************************
	LISTAT: lista los objetos presentes en una localidad.
	  Entrada:      'locno' número de localidad
			bandera 7 a 1 si se imprime mensaje 'nada' cuando no
			hay objetos que listar
****************************************************************************/
BOOLEAN listat(BYTE locno)
{
BYTE i, j=0;
char *po;
BYTE obl[MAX_OBJ];

/* si se trata de localidad actual sustituye por su valor */
if(locno==LOC_ACTUAL) locno=getflag(FPLAYER);

/* recorre toda la tabla de objetos */
for(i=0; i<cab.num_obj; i++) {
	/* almacena números de objetos en localidad especificada */
	if(loc_obj[i]==locno) {
		obl[j]=i;
		j++;
	}
}

/* si no se encontró ningún objeto */
if(!j)
{
	clearflagbit(FFLAGS, FFOBJLISTED); /* Pone a 0 el bit 7 del flag FFLAGS, ningún objeto se ha listado */
	sysmess(53);       /* ' nada.' */
} 
else {
	/* recupera objetos guardados */
	setflagbit(FFLAGS, FFOBJLISTED);  /* Pone a 1 el bit 7 del flag FFLAGS, algun objeto se ha listado */
	for(i=0; i<j; i++) {
		/* puntero a texto objeto */
		po=tab_obj+tab_desp_obj[obl[i]]+6;

		/* si bit 6 de FFLAGS está activado imprime en formato columna */
		if(getflagbit(FFLAGS, FFLISTCOL)) {
			vv_imps(po,&w[getflag(FDARK)]);
			vv_impc('\n',&w[getflag(FDARK)]);
		}
		else {
			/* imprime objeto */
			vv_imps(po,&w[getflag(FDARK)]);
			/* si final de la lista */
			if(i==(BYTE)(j-1)) sysmess(48);
			/* unión entre los dos últimos objetos de lista */
			else if(i==(BYTE)(j-2)) sysmess(47);
			/* separación entre objs. */
			else sysmess(46);
		}
	}
}

return(TRUE);
}

/****************************************************************************
	ISAT: comprueba si un objeto está en una localidad.
	  Entrada:      'objno' número de objeto
			'locno' número de localidad
	  Salida:       TRUE si el objeto está en la localidad
			FALSE si no está en la localidad
****************************************************************************/
BOOLEAN isat(BYTE objno, BYTE locno)
{
	/* si se trata de localidad actual sustituye por su valor */
	if(locno==LOC_ACTUAL) locno=getflag(FPLAYER);
	if(loc_obj[objno]==locno) return(TRUE);
	return(FALSE);
}

/****************************************************************************
	ISNOTAT: comprueba si un objeto no está en una localidad.
	  Entrada:      'objno' número de objeto
			'locno' número de localidad
	  Salida:       TRUE si el objeto no está en la localidad
			FALSE si está en la localidad
****************************************************************************/
BOOLEAN isnotat(BYTE objno, BYTE locno)
{
	return(TRUE-isat(objno,locno));
}

/****************************************************************************
	PRESENT: comprueba si un objeto está presente (en localidad actual,
	  cogido o puesto).
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si el objeto está presente
			FALSE si no está presente
****************************************************************************/
BOOLEAN present(BYTE objno)
{

if(isat(noobjnobj,LOC_ACTUAL) || isat(objno,COGIDO) || isat(objno,PUESTO))
  return(TRUE);

return(FALSE);
}

/****************************************************************************
	ABSENT: comprueba si un objeto no está presente.
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si el objeto no está presente
			FALSE si está presente
****************************************************************************/
BOOLEAN absent(BYTE objno)
{

return(TRUE-present(objno));
}

/****************************************************************************
	WORN: comprueba si un objeto está puesto.
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si el objeto está puesto
			FALSE si no está puesto
****************************************************************************/
BOOLEAN worn(BYTE objno)
{

if(isat(objno,PUESTO)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTWORN: comprueba si un objeto no está puesto.
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si el objeto no está puesto
			FALSE si está puesto
****************************************************************************/
BOOLEAN notworn(BYTE objno)
{

return(TRUE-worn(objno));
}

/****************************************************************************
	CARRIED: comprueba si un objeto está cogido.
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si el objeto está cogido
			FALSE si no está cogido
****************************************************************************/
BOOLEAN carried(BYTE objno)
{

if(isat(objno,COGIDO)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	NOTCARR: comprueba si un objeto no está cogido.
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si el objeto no está cogido
			FALSE si está cogido
****************************************************************************/
BOOLEAN notcarr(BYTE objno)
{

return(TRUE-carried(objno));
}

/****************************************************************************
	WEAR: pone un objeto que sea una prenda.
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si se pudo poner el objeto
			FALSE si no se pudo poner
	  NOTA: el flag FRFOBJ contendrá el número del objeto a la salida
****************************************************************************/
BOOLEAN wear(BYTE objno)
{
char *po;

/* si el objeto no existe */
if(objno>=cab.num_obj) {
	sysmess(28);                     /* 'No tengo eso.' */
	return(FALSE);
}

/* coloca el número del objeto en el flag FRFOBJ */
setflag(FRFOBJ,objno);

/* puntero a banderas1 de objeto */
po=tab_obj+tab_desp_obj[objno]+3;

/* si el objeto está puesto */
if(worn(objno)) {
	sysmess(29);                    /* 'Ya llevo puesto _.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si el objeto no está aquí */
if(absent(objno)) {
	sysmess(26);                     /* 'Aquí no está eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* si el objeto no está cogido */
if(notcarr(objno)) {
	sysmess(28);                     /* 'No tienes eso.' */
	return(FALSE);
}

/* si no es una prenda */
if(!(*po & 0x01)) {
	sysmess(40);                    /* 'No puedes ponerte _.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* se pone la prenda */
place(objno,PUESTO);
sysmess(37);                            /* 'Te pones _.' */

return(TRUE);
}

/****************************************************************************
	REMOVE: quita un objeto que sea una prenda.
	  Entrada:      'objno' número de objeto
	  Salida:       TRUE si se pudo quitar el objeto
			FALSE si no se pudo quitar
	  NOTA: el flag 8 contendrá el número del objeto a la salida
****************************************************************************/
BOOLEAN remove1(BYTE objno)
{

/* si el objeto no existe */
if(objno>=cab.num_obj) {
	sysmess(41);                    /* 'No llevas puesto eso.' */
	return(FALSE);
}

/* coloca el número del objeto en el flag 8 */
setflag(FRFOBJ,objno);

/* si no lo lleva puesto */
if(notworn(objno)) {
	sysmess(41);                    /* 'No llevas puesto eso.' */
	return(FALSE);                  /* debe hacer un DONE */
}

/* pasa el objeto a cogido */
place(objno,COGIDO);
sysmess(38);                            /* 'Te quitas _.' */

return(TRUE);
}

/****************************************************************************
	CREATE: pasa un objeto no creado a la localidad actual.
	  Entrada:      'objno' número de objeto
****************************************************************************/
BOOLEAN create(BYTE objno)
{
	place(objno,getflag(FPLAYER));
	return(TRUE);
}

/****************************************************************************
	DESTROY: pasa un objeto a no creado.
	  Entrada:      'nobj' número de objeto
****************************************************************************/
BOOLEAN destroy(BYTE objno)
{

place(objno,NO_CREADO);

return(TRUE);
}

/****************************************************************************
	SWAP: intercambia dos objetos.
	  Entrada:      'objno1' número de objeto 1
			'objno2' número de objeto 2
****************************************************************************/
BOOLEAN swap(BYTE objno1,BYTE objno2)
{
BYTE locobj2;

locobj2=loc_obj[objno2];         /* guarda localidad de 2 objeto */
place(objno2,loc_obj[objno1]);    /* pasa 2 a localidad del 1? */
place(objno1,locobj2);           /* pasa 1 a localidad del 2? */

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
	WHATO: devuelve el número de objeto que se corresponde con el nombre1,
	  adjetivo1 de la sentencia lógica actual.
	  Entrada:      var(3) nombre
					var(4) adjetivo
	  Salida:       var(8) número de objeto
****************************************************************************/
BOOLEAN whato(void)
{
BYTE i;
WORD po;

/* inicializa a número de objeto no válido */
setflag(FRFOBJ,MAX_VAL);

for(i=0; i<cab.num_obj; i++) {
	/* puntero a nombre-adjetivo objeto */
	po = cab.obj_names_pos+ i * 2;

	/* si encuentra el objeto, coloca su número en var(8) */
	if((getflag(FNOUN)==getDDBByte(po)) && ((getflag(FADJECT)==NO_PAL) ||
	   (getflag(FNOUN)==getDDBByte(po)) && ((getflag(FADJECT)==getDDBByte(po+1)))
	   {
		setco(i); /* Asignar todos los flags de objeto referenciado */
		break;
	  }
}

return(TRUE);
}

/****************************************************************************
	MOVE: actualiza el contenido de un flag de acuerdo a su
	  contenido actual, a la sentencia lógica y a la tabla de conexiones,
	  para que contenga el número de localidad con la que conecta una
	  dada por medio de la palabra de movimiento de la sentencia lógica.
	  Entrada:      'nv' número de variable (cuyo contenido es el número
			de una localidad válida)
			var(2) y var(3) sentencia lógica
	  Salida:       TRUE si hay conexión y 'nv' modificada para que
			contenga un número de localidad que conecta con la
			dada por medio de la tabla de conexiones
			FALSE si no hay conexión en esa dirección y 'nv' sin
			modificar
****************************************************************************/
BOOLEAN move(BYTE nv)
{
BYTE *pc;

/* puntero a conexiones de localidad var(nv) */
pc=tab_conx+tab_desp_conx[getflag(nv)];

/* mientras haya conexiones */
while(*pc) {
	if((getflag(FVERB)==*pc && getflag(FNOUN)==NO_PAL) || (getflag(FVERB)==NO_PAL && getflag(FNOUN)==*pc)
	  || (getflag(FVERB)==*pc && getflag(FNOUN)==*pc)) {
		/* guarda número localidad hacia la que conecta y sale */
		setflag(nv,*(pc+1));
		return(TRUE);
	}
	/* siguiente conexión */
	pc+=2;
}

/* si no encontró conexión */
return(FALSE);
}

/* FALTA: ESTO NO ES DAAD */
/****************************************************************************
	ISMOV: comprueba si la sentencia lógica es de movimiento
	  (movim. - NO_PAL, NO_PAL - movim. o movim. - movim.).
	  Entrada:      var(2) y var(3) con nombre-verbo
	  Salida:       TRUE si es sentencia lógica de movimiento
			FALSE si no lo es
****************************************************************************/
BOOLEAN ismov(void)
{

if((getflag(FVERB)<cab.v_mov && getflag(FNOUN)==NO_PAL) ||
  (getflag(FVERB)==NO_PAL && getflag(FNOUN)<cab.v_mov) ||
  (getflag(FVERB)<cab.v_mov && getflag(FNOUN)<cab.v_mov)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	GOTO: va a una localidad especificada.
	  Entrada:      'locno' número de localidad
	  Salida:       var(1) conteniendo el número de nueva localidad
			(si 'locno' es localidad no válida, el contenido de
			var(1) no se modifica)
****************************************************************************/
BOOLEAN goto1(BYTE locno)
{
	if(locno<cab.num_loc) setflag(FPLAYER,locno);
	return(TRUE);
}

/****************************************************************************
	PRINT: imprime el contenido de un flag en la posición actual.
	  Entrada:      'flagno' número de flag
****************************************************************************/
BOOLEAN print(BYTE flagno)
{
	vv_impn((unsigned)getflag(nflagnof),&w[getflag(FDARK)]);
	return(TRUE);
}

/****************************************************************************
	DPRINT: imprime el contenido de dos flags consecutivos como un
	  número de 16 bits en la posición actual.
	  Entrada:      'flagno' número de la primera variable
	  Nota: si el flag flagno es el último, el resultado será 0
****************************************************************************/
BOOLEAN dprint(BYTE flagno)
{
	unsigned num;

	if (flagno==FLAGS - 1) num = 0; else num=(getflag(flagno+1)*256)+getflag(flagno);
	vv_impn(num,&w[getflag(FDARK)]);
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

sysmess(16);                    /* 'Pulsa una tecla.' */
vv_lee_tecla();

return(TRUE);
}

/****************************************************************************
	PAUSE: realiza una pausa de una duración determinada o hasta que se
	  pulse una tecla.
	  Entrada:      'duration' valor de la pausa en décimas de segundo
****************************************************************************/
BOOLEAN pause(BYTE duration)
{
	/* hace la pausa */
	pausa(duration);
	/* saca tecla del buffer */
	if(bioskey(1)) bioskey(0);
	return(TRUE);
}

/****************************************************************************
	LISTOBJ: lista los objetos de la localidad actual. Si no hay no se pinta
	nada.
****************************************************************************/
BOOLEAN listobj(void)
{
	BYTE i, j=0;
	BYTE obl[MAX_OBJ];

	/* recorre toda la tabla de objetos */
	for(i=0; i<cab.num_obj; i++) {
		/* almacena números de objetos en localidad actual */
		if(loc_obj[i]==getflag(FPLAYER)) 
		{
			obl[j]=i;
			j++;
		}
	}

	/* si no está oscuro o hay luz lista objetos */
	if(j)
	{
		sysmess(1);         /* 'También puedes ver: ' */
		listat(getflag(FPLAYER));
	}

		return(TRUE);
}

/* FALTA: ni este ni el de abajo son DAAD */
/****************************************************************************
	FIRSTO: coloca el puntero NEXTO al principio de la tabla de objetos.
****************************************************************************/
BOOLEAN firsto(void)
{

/* coloca puntero al inicio de la tabla - 1 */
ptr_nexto=-1;
/* indica que está activo bucle DOALL */
doall=TRUE;

return(TRUE);
}

/****************************************************************************
	NEXTO: mueve el puntero NEXTO al siguiente objeto que esté en la
	  localidad especificada.
	  Entrada:      'locno' número de localidad
	  Salida:       variables 3 y 4 con el nombre y adjetivo del
			siguiente objeto encontrado (si el objeto encontrado
			es el último pone 'doall' a FALSE)
****************************************************************************/
BOOLEAN nexto(BYTE locno)
{
char *po;
int i;

/* si no está en bucle DOALL sale */
if(doall==FALSE) return(TRUE);

/* si se refiere a la localidad actual sustituye por su valor */
if(locno==LOC_ACTUAL) locno=getflag(FPLAYER);

while(1) {
	ptr_nexto++;
	/* si llegó al final cancela bucle DOALL */
	if(ptr_nexto>=(int)cab.num_obj) {
		doall=FALSE;
		return(TRUE);
	}
	/* si objeto está en localidad */
	if(loc_obj[ptr_nexto]==locno) {
		/* puntero a objeto */
		po=tab_obj+tab_desp_obj[ptr_nexto];
		/* coge nombre y adjetivo de objeto */
		setflag(FNOUN,(BYTE)*po);
		setflag(FADJECT,(BYTE)*(po+1));
		/* mira si es el útimo objeto en la localidad indicada */
		for(i=ptr_nexto+1; i<(int)cab.num_obj; i++) {
			if(loc_obj[i]==locno) break;
		}
		/* si es último objeto desactiva bucle DOALL */
		if(i>=(int)cab.num_obj) doall=FALSE;
		return(TRUE);
	}
}

}

/****************************************************************************
	SYNONYM: coloca el verbo-nombre dado en las variables 2 y 3,
	  si alguno es NO_PAL no realiza la sustitución.
****************************************************************************/
BOOLEAN synonym(BYTE verb, BYTE nomb)
{
	if(verb!=NO_PAL) setflag(FVERB,verb);
	if(nomb!=NO_PAL) setflag(FNOUN,nomb);
	return(TRUE);
}

/****************************************************************************
	HASAT: comprueba si el objeto actual (cuyo número está en el flag
	  del sistema 8) tiene activada una bandera de usuario.
	  Entrada:      'val' número de bandera de usuario a comprobar
			(0 a 17), 16 comprueba si es PRENDA y 17 si FUENTE
			DE LUZ
	  Salida:       TRUE si la bandera de usuario está a 1
			FALSE si está a 0
****************************************************************************/
BOOLEAN hasat(BYTE val)
{

BYTE objno = getflag(FRFOBJ);

if (val == 31)  /* CONTAINER */
{
	if iscontainer(objno) return(TRUE); else return(FALSE);
}

if (val == 23) /* WEARABLE */
{
	if iswearable(objno) return(TRUE); else return(FALSE);
}

if (val == 55)  /* LISTED */
{
	if (getflagbit(FFLAGS, FFOBJLISTED)==TRUE) return(TRUE); else return(FALSE);
}

if (val == 87 )  /* TIMEOUT */
{
	if (getflagbit(FTIMEOUTFLG, FTTIMEOUT)==TRUE) return(TRUE); else return(FALSE);
}

if (val ==247) return(TRUE);   else return(FALSE); /* GRAPHICS */

if (val ==240) return(TRUE);  else return(FALSE);  /* MOUSE */

if (val>15) return (FALSE);

WORD objflags = getDDBWord(obj_att_pos + objno * 2);

if (objflags & (1<<val)) return(TRUE);  else return(FALSE); /* real user attrs */

}

/****************************************************************************
	HASNAT: comprueba si el objeto actual (cuyo número está en la
	  variable del sistema 8) no tiene activada una bandera de usuario.
	  Entrada:      'val' número de bandera de usuario a comprobar
			(0 a 17), 16 comprueba si es PRENDA y 17 si FUENTE
			DE LUZ
	  Salida:       TRUE si la bandera de usuario está a 0
			FALSE si está a 1
****************************************************************************/
BOOLEAN hasnat(BYTE val)
{
	return(TRUE-hasat(val));
}

/****************************************************************************
	RANDOM1: genera números aleatorios.
	  Entrada:      'varno' número de variable que contendrá el número
			aleatorio
			'rnd' límite de número aleatorio
	  Salida:       'varno' conteniendo un número aleatorio entre 0 y
			'rnd'-1
****************************************************************************/
BOOLEAN random1(BYTE varno, BYTE rnd)
{

setflag(varno,(BYTE)(random(rnd)));

return(TRUE);
}

/****************************************************************************
	SEED: coloca el punto de inicio del generador de números aleatorios.
	  Entrada:      'seed' punto de inicio dentro de la serie de números
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
	PUTO: coloca el objeto actual cuyo número está en el flag 8 en
	  una localidad.
	  Entrada:      'nloc' número de localidad
****************************************************************************/
BOOLEAN puto(BYTE nloc)
{
	place(getflag(FRFOBJ),nloc);
	return(TRUE);
}

/****************************************************************************
	INKEY: coloca en las variables 9 y 10 el par de códigos ASCII IBM
	  de la última tecla pulsada (si se pulsó alguna).
	  Salida:       TRUE si se pulsó una tecla y además...
			 -variable 9 conteniendo 1er código ASCII IBM (código
			  ASCII del carácter, si es distinto de 0)
			 -variable 10 conteniendo 2o código ASCII IBM (código
			  de scan de la tecla pulsada)
			FALSE si no se pulsó ninguna tecla (deja sin
			modificar las variables 9 y 10)
****************************************************************************/
BOOLEAN inkey(void)
{
unsigned tecla;

/* si hay tecla esperando */
if(bioskey(1)) {
	/* recoge códigos tecla pulsada */
	tecla=bioskey(0);
	/* código ASCII, byte bajo */
	setflag(FKEY1,(BYTE)(tecla & 0x00ff));
	/* código scan, byte alto */
	setflag(FKEY2,(BYTE)((tecla >> 8) & 0x00ff));
	return(TRUE);
}

return(FALSE);
}

/****************************************************************************
	COPYOF: copia el número de localidad en la que está el objeto
	  		referenciado en un flag dado.
	  Entrada:      'nobj' número de objeto
					'flagno' número de flag
****************************************************************************/
BOOLEAN copyof(BYTE objno, BYTE flagno)
{

setflag(flagno,loc_obj[objno]);

return(TRUE);
}

/****************************************************************************
	CHANCE: comprueba una probabilidad en tanto por ciento.
	  Entrada:      'val' probabilidad de 0 a 100
	  Salida:       TRUE si el número aleatorio generado internamente es
			menor o igual que rnd
			FALSE si es mayor que rnd
****************************************************************************/
BOOLEAN chance(BYTE val)
{
BYTE chc;

/* número aleatorio entre 0 y 100 */
chc=(BYTE)random(101);

if(chc>val) return(FALSE);

return(TRUE);
}

/****************************************************************************
	RAMSAVE: graba en uno de los bancos de ram disponibles el estado
	  actual (variables, banderas y posición actual de objetos).
****************************************************************************/
BOOLEAN ramsave()
{
unsigned i;
BYTE banco = 0;

/* marca banco usado */
ram[banco].usado=TRUE;

/* guarda flags */
for(i=0; i<FLAGS; i++) ram[banco].bram[i]=getflag(i);

/* guarda localidades actuales de los objetos */
for(i=0; i<MAX_OBJ; i++) ram[banco].bram[FLAGS+BANDS+i]=loc_obj[i];

return(TRUE);
}

/****************************************************************************
	RAMLOAD: recupera una posición guardada con RAMSAVE.
	  Entrada:      'ftop'  máximo número de flag a recuperar (se
					recuperará desde la 0 hasta 'ftop' inclusive)
	  Salida:       TRUE si se pudo ejecutar RAMLOAD
					FALSE si el banco indicado no fué usado antes por RAMSAVE
****************************************************************************/
BOOLEAN ramload( BYTE ftop)
{
unsigned i;
BYTE banco = 0; /* FALTA: LIMPIAR PORQUE SINTACT TENIA DOS BANCOS DE RAMSAVE PERO NO LOS USAMOS */

/* mira si el banco ha sido usado */
if(!ram[banco].usado) return(FALSE);

/* recupera flags */
for(i=0; i<=(unsigned)ftop; i++) setflag(i,ram[banco].bram[i]);

/* recupera localidades actuales de los objetos */
for(i=0; i<MAX_OBJ; i++) loc_obj[i]=ram[banco].bram[FLAGS+BANDS+i];


return(TRUE);
}

/****************************************************************************
	ABILITY: designa el número de objetos máximo que puede ser llevado.
	  Entrada:      'max_objs' número de objetos máximo (0 ilimitados)
	  				'max_weight' máximo peso llevable
****************************************************************************/
BOOLEAN ability(BYTE max_objs, BYTE max_weight)
{
	setflag(FABILITY,max_objs);
	setflag(FABILITY2, max_weight);
	return(TRUE);
}

/****************************************************************************
	AUTOG: coge el objeto cuyo nombre-adjetivo están en las variables
	  34 y 35 respectivamente (es como WHATO seguido de GET [8]).
	  Salida:       TRUE si se pudo coger el objeto
					FALSE si no se pudo coger
****************************************************************************/
BOOLEAN autog(void)
{
	whato();
	return(get(getflag(FRFOBJ)));
}

/****************************************************************************
	AUTOD: deja el objeto cuyo nombre-adjetivo están en las variables
	  34 y 35 respectivamente (es como WHATO seguido de DROP [8]).
	  Salida:       TRUE si se pudo dejar el objeto
			FALSE si no se pudo dejar
****************************************************************************/
BOOLEAN autod(void)
{
	whato();
	return(drop(getflag(FRFOBJ)));
}

/****************************************************************************
	AUTOW: pone el objeto cuyo nombre-adjetivo están en las variables
	  34 y 35 respectivamente (es como WHATO seguido de WEAR [8]).
	  Salida:       TRUE si se pudo poner el objeto
			FALSE si no se pudo poner
****************************************************************************/
BOOLEAN autow(void)
{
	whato();
	return(wear(getflag(FRFOBJ)));
}

/****************************************************************************
	AUTOR: quita el objeto cuyo nombre-adjetivo están en las variables
	  34 y 35 respectivamente (es como WHATO seguido de REMOVE [8]).
	  Salida:       TRUE si se pudo quitar el objeto
			FALSE si no se pudo quitar
****************************************************************************/
BOOLEAN autor(void)
{
	whato();
	return(remove1(getflag(FRFOBJ)));
}

/****************************************************************************
	ISDOALL: comprueba si se está ejecutando un bucle DOALL (FIRSTO ha
	  sido ejecutado y NEXTO no ha alcanzado todavía último objeto).
	  Salida:       TRUE si se está ejecutando un bucle DOALL
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
	  Entrada:      'sysno1' mensaje del sistema que contiene la pregunta
			'sysno2'    "     "     "    con las posibles
			respuestas (cada una de una sola letra y seguidas,
			no importa si en mayúsculas o minúsculas), el máximo
			número de caracteres permitidos es 256 (0 a 255), si
			son más, el flag 'flagno' contendrá resultados
			imprevisibles
			'flagno' número de flag que contendrá la respuesta
	  Salida:       'flagno'=0 si se tecleó 1er carácter de 'sysno2', 1 si
			el 2?, 2 si el 3?, etc...
****************************************************************************/
BOOLEAN ask(BYTE sysno1, BYTE sysno2, BYTE flagno)
{
char *pm, tecla;
int antx, anty;
BYTE i;

/* imprime la pregunta */
sysmess(sysno1);
/* guarda posición actual del cursor */
antx=w[getflag(FACTWINDOW)].cvx;
anty=w[getflag(FACTWINDOW)].cvy;
while(1) {
	/* recupera coordenadas */
	w[getflag(FACTWINDOW)].cvx=antx;
	w[getflag(FACTWINDOW)].cvy=anty;

	/* imprime cursor */
	vv_impc(*(tab_msy+tab_desp_msy[7]),&w[getflag(FACTWINDOW)]);

	/* recupera coordenadas */
	w[getflag(FACTWINDOW)].cvx=antx;
	w[getflag(FACTWINDOW)].cvy=anty;

	do {
		tecla=mayuscula(vv_lee_tecla());
	} while((BYTE)tecla==COD_RAT);

	/* imprime tecla pulsada */
	vv_impc(tecla,&w[getflag(FACTWINDOW)]);

	/* comienza búsqueda */
	i=0;
	/* puntero a mensaje con las respuestas */
	pm=tab_msy+tab_desp_msy[sysno2];

	/* analiza hasta final de cadena */
	while(*pm) {
		if(mayuscula(*pm)==tecla) {
			/* almacena número de carácter de sysno2 y sale */
			setflag(flagno,i);
			return(TRUE);
		}
		/* pasa al siguiente carácter de sysno2 */
		pm++;
		i++;
	}
}

}

/****************************************************************************
	QUIT: presenta el mensaje del sistema 24 (?Estás seguro?) y pregunta
	  para abandonar.
	  Salida:       TRUE si se responde con el 1er carácter del mensaje
			del sistema 25
			FALSE si se responde con el 2o carácter del mensaje
			del sistema 25
	  NOTA: se usa el flag 11
****************************************************************************/
BOOLEAN quit(void)
{

/* hace pregunta '?Estás seguro?' */
ask(24,25,11);
newline();

/* si respondió con 1er carácter */
if(!getflag(FASK)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	SAVE: guarda la posición actual en disco (flags, banderas y
	  posición actual de objetos).
****************************************************************************/
BOOLEAN save(void)
{
BOOLEAN raton;
int h_save;

/* coge estado actual del raton */
raton=getflagbit(FFLAGS, FFMOUSEON);

sysmess(60);                    /* 'Nombre del fichero: ' */
clearflagbit(FFLAGS, FFMOUSEON);
vv_inputt(f_sl,MAXLONG,*(tab_msy+tab_desp_msy[7]),VVINP_CONV,&w[getflag(FACTWINDOW)],0,0);
if(raton) setflagbit(FFLAGS, FFMOUSEON); 
newline();

/* si el fichero ya existe */
if(!access(f_sl,0)) {
	ask(27,25,11);          /* 'Fichero ya existe. ?Quieres continuar? ' */
	newline();
	/* si respondió con 2? carácter del mensaje del sistema 25 */
	if(getflag(FASK)) return(TRUE);
}

h_save=open(f_sl,O_CREAT|O_TRUNC|O_BINARY|O_RDWR,S_IREAD|S_IWRITE);

/* si ocurrió un error en apertura */
if(h_save==-1) {
	sysmess(56);            /* 'Error de apertura de fichero.' */
	return(TRUE);
}

/* guarda flags */
if(write(h_save,flag,FLAGS)==-1) {
	sysmess(57);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}

/* guarda localidades actuales de los objetos */
if(write(h_save,loc_obj,MAX_OBJ)==-1) {
	sysmess(57);            /* 'Error de entrada/salida en fichero.' */
	close(h_save);
	return(TRUE);
}


close(h_save);

return(TRUE);
}

/****************************************************************************
	LOAD: recupera de disco una posición grabada con SAVE.
	  Entrada:      'ftop' máximo número de flag a recuperar (se
			recuperará desde la 0 hasta 'ftop' inclusive)
****************************************************************************/
BOOLEAN load(BYTE ftop)
{
int h_load, i, nbyte, nbit;
BOOLEAN raton;
BYTE mascara;
BYTE var_l[FLAGS];


/* coge estado actual del ratón */
raton=getflagbit(FFLAGS, FFMOUSEON);

sysmess(60);                    			/* 'Nombre del fichero: ' */
clearflagbit(FFLAGS, FFMOUSEON);                      /* desactiva pero no oculta ratón */
vv_inputt(f_sl,MAXLONG,*(tab_msy+tab_desp_msy[7]),VVINP_CONV,&w[getflag(FACTWINDOW)],0,0);
if(raton) setflagbit(FFLAGS, FFMOUSEON);
newline();

h_load=open(f_sl,O_BINARY|O_RDONLY);

/* si ocurrió un error en apertura */
if(h_load==-1)
{
	sysmess(56);            /* 'Error de apertura de fichero.' */
	return(TRUE);
}


/* recoge flags */
if(read(h_load,var_l,FLAGS)==-1) 
{
	sysmess(57);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}


/* recoge localidad actual de los objetos */
if(read(h_load,loc_obj,MAX_OBJ)==-1) 
{
	sysmess(57);            /* 'Error de entrada/salida en fichero.' */
	close(h_load);
	return(TRUE);
}


/* recupera flags */
for(i=0; i<=(int)ftop; i++) setflag(i,var_l[i]);


close(h_load);

return(TRUE);
}

/****************************************************************************
	EXIT1: sale al sistema operativo o reinicia.
	  Entrada:      'ex' si es 1 reinicia, si es 0 sale al sistema
			operativo
****************************************************************************/
BOOLEAN exit1(BYTE ex)
{
unsigned i;

/* sale al sistema operativo si 'ex' es 0 */
if(ex==1) m_err(0,"",1);

/* reinicialización */
/* borra la pantalla */
cls();

/* reinicializa flags
 */
inic();

/* G3.25: recupera los objetos */
for(i=0; i<cab.bytes_obj; i++) tab_obj[i]=tab_obj2[i];

/* se pone al inicio de proceso 0 */
restart();

/* -1 a ptr_proc (y -1 del restart), luego se sumarán 2 */
ptr_proc--;

return(FALSE);
}


/****************************************************************************
	END: pregunta para otra partida o salir. Si se responde con el primer
	  carácter del mensaje del sistema 25, ejecuta un EXIT 0 (reiniciali-
	  zación). Si no sale al sistema operativo.
****************************************************************************/
BOOLEAN end1(void)
{

ask(31,25,11);                          /* '?Lo intentas de nuevo? ' */
newline();

/* si respondió con 1er carácter del mensaje del sistema 25 */
if(!getflag(FASK)) {
	/* reinicialización */
	exit1(0);
	/* +1 a ptr_proc (-2 del exit1), luego se sumará 1 */
	ptr_proc++;
	return(FALSE);
}

/* sale al sistema operativo */
exit1(1);

}


/****************************************************************************
	PRINTAT: coloca el cursor en una posición dada de la ventana actual.
	  Entrada:      'y', 'x' coordenada del cursor (fila, columna); si
			salen de los límites de la ventana el cursor se
			colocará en la esquina superior izquierda (0,0)
****************************************************************************/
BOOLEAN printat(BYTE y, BYTE x)
{
	/* si se sale de ventana coloca cursor en esquina superior izquierda */
	if((y>(BYTE)(w[getflag(FACTWINDOW)].lyi-1)) || (x>(BYTE)(w[getflag(FACTWINDOW)].lxi-1))) y=x=0;

	w[getflag(FACTWINDOW)].cvy=y*w[getflag(FACTWINDOW)].chralt;
	w[getflag(FACTWINDOW)].cvx=x*8;

	return(TRUE);
}

/****************************************************************************
	TAB: cambia la columna en la que se escribe en la ventana actual
****************************************************************************/
BOOLEAN TAB(BYTE col)
{
	w[getflag(FACTWINDOW)].cvx=col*8;
}

/****************************************************************************
	SAVEAT: almacena la posición de impresión de la ventana actual. Cada
	  ventana tiene sus propias posiciones de impresión almacenadas.
****************************************************************************/
BOOLEAN saveat(void)
{

w[getflag(FACTWINDOW)].cvxs=w[getflag(FACTWINDOW)].cvx;
w[getflag(FACTWINDOW)].cvys=w[getflag(FACTWINDOW)].cvy;

return(TRUE);
}

/****************************************************************************
	BACKAT: recupera la posición de impresión guardada por el último
	  SAVEAT en la ventana actual.
	  NOTA: si no se ejecutó ningún SAVEAT, la posición de impresión
	  recuperada será la (0,0).
****************************************************************************/
BOOLEAN backat(void)
{

w[getflag(FACTWINDOW)].cvx=w[getflag(FACTWINDOW)].cvxs;
w[getflag(FACTWINDOW)].cvy=w[getflag(FACTWINDOW)].cvys;

return(TRUE);
}

/****************************************************************************
	NEWTEXT: deshecha el resto de la línea de input que aún queda por
	  analizar y coloca el puntero al final de la misma.
****************************************************************************/
BOOLEAN newtext(void)
{
	/* coloca puntero al final de la frase */
	for(; *lin; lin++);
	/* indica que no queda más por analizar */
	mas_texto=TRUE;

	return(TRUE);
}

/** FALTA: ESTO NO ES DAAD */
/****************************************************************************
	PRINTC: imprime un carácter en la posición actual del cursor y dentro
	  de la ventana activa.
	  Entrada:      'car' código ASCII del carácter a imprimir
****************************************************************************/
BOOLEAN printc(BYTE car)
{

/* imprime el carácter en la ventana activa */
vv_impc(car,&w[getflag(0)]);

return(TRUE);
}

/****************************************************************************
	INK: selecciona el color temporal del primer plano en la ventana
	  activa.
	  Todos los textos de esa ventana se imprimirán con ese color de
	  tinta.
	  Entrada:      'color' índice de color
****************************************************************************/
BOOLEAN ink(BYTE color)
{

w[getflag(FACTWINDOW)].colort=color;

return(TRUE);
}

/****************************************************************************
	PAPER: selecciona el color temporal del fondo en la ventana activa.
	  Todos los textos de esa ventana se imprimirán con ese color de
	  fondo.
	  Entrada:      'color' índice de color
****************************************************************************/
BOOLEAN paper(BYTE color)
{

w[getflag(FACTWINDOW)].colortf=color;

return(TRUE);
}

/****************************************************************************
	DEBUG: activa o desctiva el paso a paso.
	  Entrada:      'modo' indica si el paso a paso está activado (1)
			o desactivado (0)
	  NOTA: este condacto sólo es activo en el intérprete-debugger
****************************************************************************/
BOOLEAN debug(BYTE modo)
{

#if DEBUGGER==1
if(modo) {
	debugg=TRUE;
	/* indicamos que no es primera línea de */
	/* entrada para que imp_condacto() no */
	/* imprima el verbo-nombre ya que el puntero */
	/* al condacto puede no estar ajustado */
	pra_lin=FALSE;
}
else debugg=FALSE;
#endif

return(TRUE);
}


/****************************************************************************
	CHARSET: carga y selecciona un nuevo juego de caracteres.
	  Entrada:      'set' número del juego de caracteres (0-255), si se
			introduce 0, se seleccionará el juego de caracteres
			por defecto.
	  Salida:       TRUE si se cargó la fuente con éxito
			FALSE si hubo algún error o la tarjeta gráfica no
			soporta cambio del juego de caracteres
	  NOTA: los juegos de caracteres deben estar en ficheros cuyo nombre
	  sea F#??????.ext siendo 'ext' el número identificador de 1 a 255,
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

/* construye nombre de fichero a?adiendo extensión */
itoa((int)set,num,10);
strcat(nombre,num);

/* busca el fichero y sale si no lo encontró */
if(findfirst(nombre,&info,0)) return(FALSE);

/* el nombre verdadero del fichero estará en 'info.name' */
/* lo carga y actualiza generador de caracteres */
if(!carga_def(info.ff_name)) return(FALSE);

return(TRUE);
}

/****************************************************************************
	EXTERN1: No soportado
****************************************************************************/
BOOLEAN extern1(BYTE prg, BYTE par)
{

}

/****************************************************************************
	GTIME: devuelve la hora real del sistema.
	  Entrada:      'nv' número de flag inicial dónde se colocará
			la hora; 'flag' contendrá los segundos, 'flag+1' los
			minutos y 'flag+2' la hora
	  NOTA: si 'nv' es mayor de 253 el condacto no hará nada
****************************************************************************/
BOOLEAN gtime(BYTE flagno)
{
struct time hora;

/* comprueba que el número de flag no sea mayor de FLASG - 3  */
if(flagno>FLAGS-3) return(TRUE);

/* recoge hora del sistema */
gettime(&hora);

/* guarda en flags los segundos, minutos y la hora en este orden */
/* segundos de 0 a 59 */
setflag(flagno,hora.ti_sec);
/* minutos de 0 a 59 */
setflag(flagno+1,hora.ti_min);
/* hora de 0 a 23 */
setflag(flagno+2,hora.ti_hour);

return(TRUE);
}

/****************************************************************************
	TIME: selecciona tiempo muerto y modo de temporización del condacto
	  INPUT.
	  Entrada:      'tiempo' tiempo muerto para INPUT en segundos
			'modo' modo de temporización; 0 si el tiempo se
			da para teclear la frase completa, 1 si se da hasta
			teclear primer carácter
****************************************************************************/
BOOLEAN time1(BYTE tiempo, BYTE modo)
{

/* coloca el tiempo en el flag 16 */
setflag(FTIMEOUT,tiempo);

/* coloca el modo en el flag  6 */
if(modo==0) clearflagbit(FFLAGS, FTSTART);
else setflagbit(FFLAGS, FTSTART);

return(TRUE);
}

/****************************************************************************
	TIMEOUT: comprueba si se produjo un tiempo muerto en el último
	  condacto INPUT.
	  Salida:       TRUE si se produjo un tiempo muerto
			FALSE si no
****************************************************************************/
BOOLEAN timeout(void)
{

/* si el flag 12 es 1 se produjo un tiempo muerto */
if(getflag(FINPUT)==1) return(TRUE);
else return(FALSE);

}

#pragma warn -use
/****************************************************************************
	MODE: selecciona el modo de vídeo.
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
	GRAPHIC: presenta un gráfico en la ventana actual.
	  Entrada:      'graf' número de fichero GRF (librería)
			'img' número de imagen
			'modo' modo de dibujo
			  0 - dibuja la imagen según la lee del disco
			  1 - dibuja la imagen al final
			  2 - como 0 pero sin alterar paleta de colores
			  3 - como 1 pero sin alterar paleta de colores
			  otros valores se consideran igual a 0
	  Salida:       TRUE si se cargó el gráfico
			FALSE si hubo algún error
	  NOTA: los gráficos deben estar agrupados en ficheros GRF cuyo
	  nombre sea G#??????.ext siendo 'ext' el número identificador de 1
	  a 255, por ejemplo: G#OBJET.1, G#PERSO.203, ...
****************************************************************************/
BOOLEAN graphic(BYTE graf, BYTE img, BYTE modo)
{
STC_IMGRF imagen;
struct ffblk info;
char grafico[8]="G#*.", num[4];
int x, y, ancho, alto, err, paleta=0;

/* comprueba si modo es válido, si no le asigna 0 */
if(modo>3) modo=0;

/* si es modo 2 o 3 indica que hay que conservar paleta */
if((modo==2) || (modo==3)) paleta=1;

/* construye nombre de fichero a?adiendo extensión */
itoa((int)graf,num,10);
strcat(grafico,num);

/* busca el fichero y sale si no lo encuentra */
/* el nombre verdadero del fichero estará en 'info.ff_name' */
if(findfirst(grafico,&info,0)) return(FALSE);

/* calcula posición y dimensiones de la ventana actual, en pixels */
x=w[getflag(FACTWINDOW)].vxi*8;
y=w[getflag(FACTWINDOW)].vyi*w[getflag(FACTWINDOW)].chralt;
ancho=w[getflag(FACTWINDOW)].lxi*8;
alto=w[getflag(FACTWINDOW)].lyi*w[getflag(FACTWINDOW)].chralt;

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
	  Entrada:      'color' índice del color a cambiar
			'r', 'g', 'b' valor de las componentes RGB del color
****************************************************************************/
BOOLEAN remapc(BYTE color, BYTE r, BYTE g, BYTE b)
{

/* si en modo de 16 colores ajusta el índice de color para que no */
/* sobrepase el máximo permitido */
if((getflag(FVIDEOMODE)==0) && (color>15)) color=15;

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
	SETAT: activa una bandera de usuario del objeto actual (cuyo número
	  está en el flag del sistema 8).
	  Entrada:      'val' número de bandera de usuario a activar
			(0 a 17), 16 activa PRENDA y 17 activa FUENTE DE LUZ
****************************************************************************/
BOOLEAN setat(BYTE val)
{
char *po;
unsigned flags2, masc=0x8000;

/* puntero a banderas2 de objeto */
po=tab_obj+tab_desp_obj[getflag(FRFOBJ)]+4;

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

/* desplaza máscara */
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
	  número está en el flag del sistema 8).
	  Entrada:      'val' número de bandera de usuario a desactivar
			(0 a 17), 16 activa PRENDA y 17 activa FUENTE DE LUZ
****************************************************************************/
BOOLEAN clearat(BYTE val)
{
char *po;
unsigned flags2, masc=0x8000;

/* puntero a banderas2 de objeto */
po=tab_obj+tab_desp_obj[getflag(FRFOBJ)]+4;

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

/* desplaza máscara */
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
	  Entrada:      'color' índice del color
			'flag1', 'flag2', 'flag3' flags donde se
			almacenarán las 3 componentes RGB en este orden
****************************************************************************/
BOOLEAN getrgb(BYTE color, BYTE flag1, BYTE flag2, BYTE flag3)
{
BYTE r, g, b;

/* si en modo de 16 colores ajusta el índice de color para que no */
/* sobrepase el máximo permitido */
if((getflag(FVIDEOMODE)==0) && (color>15)) color=15;

asm {
	mov ax,1015h
	mov bh,0
	mov bl,color
	int 10h
	mov r,dh
	mov g,ch
	mov b,cl
}

setflag(flag1,r);
setflag(flag2,g);
setflag(flag3,b);

return(TRUE);
}

/****************************************************************************
	PLAYFLI: visualiza una animación FLI.
	  Entrada:      'fli' número de la animación
			'modo' modo de animación
			 0 - hasta que se pulse una tecla
			 1 - sólo una vez
	  Salida:       TRUE si pudo ejecutar la animación
			FALSE si hubo algún error
	  NOTA: las animaciones deben estar en ficheros cuyo nombre sea
	  A#??????.ext siendo 'ext' el número identificador de 1 a 255,
	  por ejemplo: A#INTRO.1, A#AVION.203, ...
****************************************************************************/
BOOLEAN playfli(BYTE fli, BYTE modo)
{
Errval err;
struct ffblk info;
char flianim[8]="A#*.", num[4];

/* comprueba que modo de pantalla sea válido */
if(getflag(FVIDEOMODE)==0) return(FALSE);

/* comprueba si modo es válido, si no le asigna 0 */
if(modo>1) modo=0;

/* construye nombre de fichero a?adiendo extensión */
itoa((int)fli,num,10);
strcat(flianim,num);

/* busca el fichero y sale si no lo encuentra */
/* el nombre verdadero del fichero estará en 'info.ff_name' */
if(findfirst(flianim,&info,0)) return(FALSE);

rg_puntero(RG_OCULTA);
if(modo==0) err=fli_play(info.ff_name);
else err=fli_once(info.ff_name);
rg_puntero(RG_MUESTRA);

if(err!=AA_SUCCESS) return(FALSE);

return(TRUE);
}

/****************************************************************************
	MOUSE: activa/desactiva el ratón.
	  Entrada:      'modo' 0 desactiva, 1 activa
****************************************************************************/
BOOLEAN mouse(BYTE modo)
{

if(modo==0) {
	rg_desconecta();
	clearflagbit(FFLAGS, FFMOUSEON);
}
else if(rg_inicializa()) setflag(FFLAGS, FFMOUSEON);

return(TRUE);
}

/****************************************************************************
	MOUSEXY: comprueba coordenadas del ratón.
	  Entrada:      'fil0', 'col0' esquina superior izquierda
			'fil1', 'col1' esquina inferior derecha
	  Salida:       TRUE si ratón se encuentra dentro del área definida
			por las coordenadas, FALSE si no o ratón desactivado
****************************************************************************/
BOOLEAN mousexy(BYTE fil0, BYTE col0, BYTE fil1, BYTE col1)
{
STC_RATONG r;

/* sale si ratón desactivado */
if(!getflagbit(FFLAGS, FFMOUSEON)) return(FALSE);

rg_estado(&r);

if(((BYTE)r.fil>=fil0) && ((BYTE)r.fil<=fil1) && ((BYTE)r.col>=col0) &&
  ((BYTE)r.col<=col1)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	MOUSEBT: comprueba estado de botones del ratón.
	  Entrada:      'btn' botón a comprobar
			 0 - izquierdo
			 1 - derecho
			 2 - ambos
			 3 - cualquiera de los dos
	  Salida:       TRUE si botón/es pulsado/s, FALSE si no o ratón
			desactivado
****************************************************************************/
BOOLEAN mousebt(BYTE btn)
{
STC_RATONG r;

/* sale si ratón desactivado */
if(!getflagbit(FFLAGS, FFMOUSEON)) return(FALSE);

rg_estado(&r);

if((btn==0) && r.boton1) return(TRUE);
else if((btn==1) && r.boton2) return(TRUE);
else if((btn==2) && r.boton1 && r.boton2) return(TRUE);
else if((btn==3) && (r.boton1 || r.boton2)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	PLAY: toca una nota.
	  Entrada:      'nota' número de nota a tocar
			'dur' duración en décimas de segundo
****************************************************************************/
BOOLEAN play(BYTE nota, BYTE dur)
{

if(nota<NUM_NOTAS) bpr_nota(nota,dur);

return(TRUE);
}

/****************************************************************************
	MUSIC: reproduce una melodía de un fichero SND.
	  Entrada:      'mus' número de la melodía
			'modo' modo de reprodución
			 0 - sólo una vez
			 1 - cíclicamente
	  Salida:       TRUE si pudo reproducir la melodía
			FALSE si hubo algún error
	  NOTA: las melodías deben estar en ficheros cuyo nombre sea
	  M#??????.ext siendo 'ext' el número identificador de 1 a 255,
	  por ejemplo: M#INIC.1, M#MUSI.203, ...
****************************************************************************/
BOOLEAN music(BYTE mus, BYTE modo)
{
struct ffblk info;
char musica[8]="M#*.", num[4];
int err;

/* comprueba si modo es válido, si no le asigna 0 */
if(modo>1) modo=0;

/* construye nombre de fichero a?adiendo extensión */
itoa((int)mus,num,10);
strcat(musica,num);

/* busca el fichero y sale si no lo encuentra */
/* el nombre verdadero del fichero estará en 'info.ff_name' */
if(findfirst(musica,&info,0)) return(FALSE);

if(modo==0) err=bpr_musica(info.ff_name,SND_SOLO);
else err=bpr_musica(info.ff_name,SND_MULT);

if(err) return(FALSE);

return(TRUE);
}
