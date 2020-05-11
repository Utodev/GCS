/****************************************************************************
	Funciones para compilar cada secci¢n de la base de datos
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dir.h>
#include "ventana.h"
#include "cuadro.h"
#include "sonido.h"
#include "version.h"
#include "sintac.h"
#include "cserr.h"
#include "compil.h"
#include "tabcond.h"

/*** Variables externas ***/
extern BOOLEAN cs_eds;
extern STC_CUADRO ccomp;
extern STC_ELEM_LISTA lsterr;
extern FILE *ferr;
extern STC_CONSTANTE constante[NCONST];
extern int pt_const;
extern struct palabra vocabulario[NUM_PAL];

/*** Variables globales ***/
/* cadenas usadas para comprobar los car cteres de una palabra */
/* de vocabulario, una etiqueta y una constante */
char *Abecedario="ABCDEFGHIJKLMN¥OPQRSTUVWXYZ";
char *Numeros="0123456789";
char *Signos="_+-*";

/* variable global para almacenar el n£mero de l¡nea actual dentro */
/* del fichero de entrada (se usa en algunas funciones de compilar) */
unsigned long numlin_fich_entr=0;

/* variable global para almacenar el n£mero de proceso que est  */
/* siendo compilado */
BYTE num_pro_act;

/* mensajes de aviso */
char *Tab_Esp="tabulaci¢n transformada en espacio";

/* tabla para guardar etiquetas */
STC_ETIQUETA etiqueta[LABELS];
int pt_etiq=0;

/* para guardar saltos (SKIP) 'forward' */
STC_SKPFORWARD fskip[FSKIP];
int pt_skip;

/* tabla y variables para guardar llamadas a procesos 'forward' */
STC_PRCFORWARD prc[FPROCESS];
int pprc=0;

/* tabla para guardar las constantes definidas durante la compilaci¢n */
STC_CONSTANTE constante[NCONST];
int pt_const=0;

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
	FIN_LINEA: comprueba si un car cter es indicador de fin de l¡nea.
	  Entrada:      'c' car cter a comprobar
	  Salida:       TRUE si el car cter es fin de l¡nea (retorno de
			carro o \0
			FALSE en otro caso
****************************************************************************/
BOOLEAN fin_linea(char c)
{

if((c=='\n') || (c=='\0')) return(TRUE);

return(FALSE);
}

/****************************************************************************
	ES_ESPACIO: comprueba si un car cter es espacio o tabulaci¢n.
	  Entrada:      'c' car cter a comprobar
	  Salida:       TRUE si el car cter es espacio o tabulaci¢n
			FALSE en otro caso
****************************************************************************/
BOOLEAN es_espacio(char c)
{

if((c==' ') || (c=='\t')) return(TRUE);

return(FALSE);
}

/****************************************************************************
	SALTA_ESPACIOS: salta los espacios y tabulaciones iniciales
	  de una l¡nea.
	  Entrada:      's' puntero a la l¡nea
	  Salida:       puntero al primer car cter encontrado que no sea
			espacio ni tabulaci¢n
****************************************************************************/
char *salta_espacios(char *s)
{

while(es_espacio(*s)) s++;

return(s);
}

/****************************************************************************
	HASTA_ESPACIO: salta los caracteres de una l¡nea, hasta encontar un
	  espacio o el fin de la linea.
	  Entrada:      's' puntero a la l¡nea
	  Salida:       puntero al primer espacio, tabulaci¢n, retorno de
			carro o fin de l¡nea encontrado
****************************************************************************/
char *hasta_espacio(char *s)
{

while(!es_espacio(*s) && !fin_linea(*s)) s++;

return(s);
}

/****************************************************************************
	ESTA_EN: comprueba si un car cter est  en una cadena. Devuelve 0 si
	  el car cter no est  en la cadena.
	  Entrada:      's' cadena con la que se comprobar  el car cter
			'c' car cter a comprobar
	  Salida:       TRUE si el car cter est  en la cadena
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
	NUM_LINEA: recoge el n£mero de l¡nea que hay en todas las l¡neas
	  de los ficheros temporales y lo convierte.
	  Entrada:      'l' direcci¢n del puntero a l¡nea de entrada
	  Salida:       n£mero de l¡nea (tambi‚n lo mete en la variable
			global 'numlin_fich_entr')
			'l' actualizado para que apunte al car cter siguiente
			a los dos puntos (:)
****************************************************************************/
unsigned long num_linea(char *(*l))
{
int i;
char lnum[40];

for(i=0; *(*l)!=':'; lnum[i++]=*(*l)++);
lnum[i]='\0';
(*l)++;         /* salta ':' */

numlin_fich_entr=atol(lnum);

return(numlin_fich_entr);
}

/****************************************************************************
	COMPILA_VOC: compila la secci¢n de Vocabulario.
	  Entrada:      'f_voc' puntero a fichero conteniendo el vocabulario
			'vocab' puntero a tabla de vocabulario
			'num_palc' puntero a variable donde colocar el
			n£mero de palabras compiladas
	  Salida:       c¢digo de error si lo hubo
****************************************************************************/
STC_ERR compila_voc(FILE *f_voc, struct palabra *vocab, int *num_palc)
{
STC_ERR err={_E_NERR, 0};
int i, ivoc=0;
char lin[LONG_LIN], *l;
struct palabra pal;

while(1) {
	if(fgets(lin,LONG_LIN,f_voc)==NULL) {
		/* comprueba si es error, si no lo es quiere decir */
		/* que se lleg¢ al final del fichero, en cuyo caso */
		/* sale del bucle */
		if(ferror(f_voc)) {
			err.codigo=_E_ETMP;
			return(err);
		}
		else break;
	}
	l=lin;  /* apunta al inicio de la l¡nea */

	err.linea=num_linea(&l);

	/* compila el resto de la l¡nea */
	if((err.codigo=comp_lin_voc(l,&pal))!=_E_NERR) return(err);

	/* intenta meter la palabra en la tabla de vocabulario */
	/* comprueba si el vocabulario est  lleno */
	if(ivoc>NUM_PAL) {
		err.codigo=_E_MVOC;
		return(err);
	}

	/* si el vocabulario est  vac¡o mete la palabra sin m s */
	if(!ivoc) {
		for(i=0; i<LONGPAL+1; i++) vocab[ivoc].p[i]=pal.p[i];
		vocab[ivoc].num=pal.num;
		vocab[ivoc].tipo=pal.tipo;
	}
	else {
		/* mira si la palabra est  repetida */
		i=0;
		do {
			if(!compara_pal(&pal,&vocab[i])) {
				err.codigo=_E_PREP;
				return(err);
			}
		} while(++i<ivoc);
		/* si no encontr¢ una igual, la mete */
		for(i=0; i<LONGPAL+1; i++) vocab[ivoc].p[i]=pal.p[i];
		vocab[ivoc].num=pal.num;
		vocab[ivoc].tipo=pal.tipo;
	}
	ivoc++;

}
*num_palc=ivoc;

return(err);
}

/****************************************************************************
	COMP_LIN_VOC: compila una l¡nea de la secci¢n de Vocabulario.
	  Entrada:      'l' l¡nea a compilar
			'p' puntero a buffer donde dejar la palabra
	  Salida:       c¢digo de error si lo hubo
****************************************************************************/
COD_ERR comp_lin_voc(char *l, struct palabra *p)
{
COD_ERR palerr;

/* CAMPO PALABRA */
l=salta_espacios(l);
palerr=coge_pal(l,p->p);
if(palerr) return(palerr);

/* CAMPO VALOR DE PALABRA */
/* desprecia caracteres hasta un espacio o fin de l¡nea */
/* error se encuentra fin de l¡nea (faltan campos) */
l=hasta_espacio(l);
if(fin_linea(*l)) return(_E_FCAM);
l=salta_espacios(l);
/* recoge el n£mero y si hay error sale */
palerr=coge_num(l,&p->num);
/* no admite indirecci¢n al dar el valor de una palabra de vocabulario */
if(palerr==_E_CIND) return(_E_CNUM);
if(palerr) return(palerr);

/* comprueba que sea un valor v lido de palabra */
if((p->num<1) || (p->num>254)) return(_E_NPAL);

/* CAMPO TIPO DE PALABRA */
/* desprecia caracteres hasta un espacio o fin de l¡nea */
/* error si encuentra fin de l¡nea (faltan campos) */
l=hasta_espacio(l);
l=salta_espacios(l);
if(fin_linea(*l)) return(_E_FCAM);
switch(mayuscula(*l)) {
case 'V' :
	p->tipo=_VERB;
	break;
case 'N' :
	p->tipo=_NOMB;
	break;
case 'A' :
	p->tipo=_ADJT;
	break;
case 'C' :
	p->tipo=_CONJ;
	break;
default :
	return(_E_TPAL);
}

return(_E_NERR);
}

/****************************************************************************
	COGE_PAL: recoge una palabra de la l¡nea de entrada y la mete en
	  'pal' (buffer de al menos LONGPAL+1 caracteres).
	  Entrada:      'l' puntero a linea de entrada
			'pal' puntero a buffer de palabra
	  Salida:       _E_FCAM si encontr¢ un car cter fin de linea
			(no hay m s campos detr s de la palabra)
			_E_CVOC si encontr¢ car cter no v lido
			_E_NERR si no hubo error.
****************************************************************************/
COD_ERR coge_pal(char *l, char *pal)
{
int i;

for(i=0; i<LONGPAL; i++) {
	/* si encuentra fin de l¡nea rellena con espacios y sale con error */
	if(fin_linea(*l)) {
		for(; i<LONGPAL; i++) *(pal+i)=' ';
		*(pal+i)='\0';          /* marca fin de cadena */
		return(_E_FCAM);
	}
	/* si encuentra espacio o tabulaci¢n, rellena con espacios y sale */
	if(es_espacio(*l)) {
		for(; i<LONGPAL; i++) *(pal+i)=' ';
		break;
	}
	else {
		*(pal+i)=mayuscula(*l);
		l++;
		/* comprueba si es car cter v lido de palabra */
		if(!esta_en(Abecedario,*(pal+i)) &&
		  !esta_en(Numeros,*(pal+i))) return(_E_CVOC);
	}
}
*(pal+i)='\0';

return(_E_NERR);
}

/****************************************************************************
	COGE_NUM: recoge un campo num‚rico de la linea de entrada, si
	  el n£mero est  encerrado entre '[]' devuelve un c¢digo indic ndolo.
	  NOTA: si encuentra una constante tratar  de devolver el valor
	  num‚rico correspondiente si es que esta ha sido definida.
	  Entrada:      'l' puntero a l¡nea de entrada
			'n' puntero a dato de tipo BYTE
	  Salida:       'n' conteniendo n£mero convertido
			_E_CNUM si hubo error al coger el campo num‚rico
			_E_FCAM si no encontr¢ n£mero (hay un comentario)
			_E_NNCT nombre de constante no v lido
			_E_CTND constante no definida
			_E_CIND si n£mero est  entre corchetes
			_E_NERR si no hubo error
****************************************************************************/
COD_ERR coge_num(char *l, BYTE *n)
{
char num[4], cnst[LNGCONST+1];
int i=0, val;
BOOLEAN indir=FALSE;
BYTE valor;

/* si encuentra car cter '[' indica indirecci¢n */
if(*l=='[') {
	indir=TRUE;
	l++;
}

/* intenta recoger el n£mero */
while(esta_en(Numeros,*l) && (i<3)) num[i++]=*l++;
num[i]='\0';

/* si recogi¢ alg£n n£mero */
if(i) {
	val=atoi(num);
	/* comprueba si n£mero fuera de rango 8 bits */
	if((val<0) || (val>255)) return(_E_CNUM);
	valor=(BYTE)val;
}
/* si el primer car cter no es n£mero considera que */
/* ha encontrado una constante */
else {
	/* si es comentario, sale con error */
	if(*l==CHR_COMENT) return(_E_FCAM);

	if(coge_nombre_etq(&l,cnst,LNGCONST+1)) return(_E_NNCT);

	/* si indirecci¢n el £ltimo car cter debe ser ']' */
	if(indir && (*l!=']')) return(_E_NNCT);
	else if(!es_espacio(*l) && !fin_linea(*l)) return(_E_CNUM);

	/* recoge el valor de la constante (si existe) */
	if(coge_const(cnst,&valor)) return(_E_CTND);
}

if(indir) {
	l=salta_espacios(l);
	/* si indirecci¢n y £ltimo car cter no es ']', error */
	if(*l!=']') return(_E_CNUM);
	l++;            /* salta el ']' */
}

/* si el £ltimo car cter no es blanco ni fin de l¡nea */
/* ni inicio de comentario (CHR_COMENT) da error */
if(!fin_linea(*l) && (*l!=' ') && (*l!='\t') && (*l!=CHR_COMENT))
  return(_E_CNUM);

*n=valor;

/* si n£mero est  entre corchetes lo indica */
if(indir) return(_E_CIND);

return(_E_NERR);
}

/****************************************************************************
	COMPARA_PAL: compara dos palabras de vocabulario.
	  Entrada:      'p1' y 'p2'     punteros a palabras de vocabulario
	  Salida:       TRUE si son distintas
			FALSE si son iguales
****************************************************************************/
BOOLEAN compara_pal(struct palabra *p1, struct palabra *p2)
{

if(strcmp(p1->p,p2->p)) return(TRUE);

return(FALSE);
}

/****************************************************************************
	AVISO: da un mensaje de aviso del compilador.
	  Entrada:      'nlin' n£mero de l¡nea del aviso (0 si ninguna)
			'fmt' cadena de formato (como printf())
			otros argumentos que ser n impresos seg£n la cadena
			de formato
	  NOTA: procurar que la cadena no exceda de 80 caracteres
****************************************************************************/
void aviso(unsigned long nlin, char *fmt, ...)
{
static int n_avisos=0;
va_list marker;
char buff[81], vmsg[129];

va_start(marker,fmt);
vsprintf(buff,fmt,marker);
va_end(marker);

/* si hay abierto fichero de errores, manda l¡nea y mensaje a fichero */
if(ferr!=NULL) fprintf(ferr,"%lu: %s\n",nlin,buff);

if(cs_eds==FALSE) {
	strcpy(vmsg,"AVISO: ");
	strcat(vmsg,buff);
	if(nlin) {
		sprintf(buff,", l¡nea %lu",nlin);
		strcat(vmsg,buff);
	}
	c_mete_en_lista(&lsterr,vmsg);
}

/* incrementa e imprime n£mero de avisos */
n_avisos++;
sprintf(buff,"AVISOS: %3u",n_avisos);

v_pon_cursor(&ccomp.v,FIL_AVISO,COL_AVISO);
v_impcad(&ccomp.v,buff,V_NORELLENA);

}

/****************************************************************************
	COMP_TXT: compila texto de secciones de Mensajes, Mensajes del
	  Sistema, Localidades... por cada llamada compila el texto
	  delimitado entre dos caracteres CHR_DELIM
	  Entrada:      'f_txt' puntero al fichero que contiene el texto
			'txt' direcci¢n del puntero a la posici¢n del buffer
			donde se guardar  el texto (su contenido quedar 
			actualizado tras finalizar)
			'ult_txt' puntero a la £ltima posici¢n del buffer
			donde se guarda el texto
			'num_txt' n£mero del texto que se est  compilando
			(se usa para comprobar continuidad de numeraci¢n)
			'max_txt' n£mero m ximo permitido para el texto
			(se usa para comprobar numeraci¢n fuera de rango)
			'nt' variable donde se guardar  el n£mero de textos
			compilados (normalmente 1, ser  mayor si hay saltos
			en la numeraci¢n)
	  Salida:       c¢digo de error si lo hubo
****************************************************************************/
STC_ERR comp_txt(FILE *f_txt, char *(*txt), char *ult_txt, BYTE num_txt,
  BYTE max_txt, int *nt)
{
STC_ERR err={_E_NERR, 0};
char lin[LONG_LIN], *l;
BYTE n, i;
COD_ERR txterr;

/* inicializa n£mero de textos compilados */
*nt=1;

/* coge la primera l¡nea del texto que deber  tener el */
/* formato: xxxxx:@nnn ...texto... */
/* sale si error o fin de fichero */
if(fgets(lin,LONG_LIN,f_txt)==NULL) {
	if(ferror(f_txt)) err.codigo=_E_LTMP;
	return(err);
}
l=lin;  /* apunta al inicio de la l¡nea */

err.linea=num_linea(&l);

/* si el primer car cter no es CHR_DELIM sale con error */
if(*l!=CHR_DELIM) {
	err.codigo=_E_FALT;
	return(err);
}
/* salta el delimitador */
l++;

/* coge el n£mero del mensaje */
txterr=coge_num(l,&n);
if(txterr) {
	/* si indirecci¢n indica error en campo num‚rico */
	if(txterr==_E_CIND) err.codigo=_E_CNUM;
	else err.codigo=txterr;
	return(err);
}

/* comprueba si se sale del rango */
if(n>max_txt) {
	err.codigo=_E_NVAL;
	return(err);
}

/* si el n£mero no es consecutivo pero es mayor, crea mensajes nulos */
/* hasta alcanzarlo; si es menor da error de fuera de secuencia */
if(n>num_txt) {
	for(i=num_txt; i<n; i++) {
		/* mete mensaje nulo */
		*(*txt)++='\0';

		/* incrementa n£mero de textos compilados */
		(*nt)++;

		/* comprueba si rebasa la memoria */
		if(*txt>=ult_txt) {
			err.codigo=_E_FMEM;
			return(err);
		}
	}
}
else if(n<num_txt) {
	err.codigo=_E_NFSC;
	return(err);
}

/* salta el espacio entre el n£mero y el primer car cter del texto */
l=hasta_espacio(l);
l++;

while(1) {
	/* repite hasta encontrar marca de fin de texto o fin de l¡nea */
	while((*l!=CHR_DELIM) && !fin_linea(*l)) {
		/* coge el car cter de la l¡nea de entrada y lo guarda */
		/* si es un '|' lo transforma en '\n' */
		/* si es una tabulaci¢n la transforma en espacio y da aviso */
		if(*l=='|') *(*txt)++='\n';
		else if(*l=='\t') {
			*(*txt)++=' ';
			aviso(err.linea,Tab_Esp);
		}
		else *(*txt)++=*l;

		l++;

		/* comprueba si rebasa la memoria */
		if(*txt>=ult_txt) {
			err.codigo=_E_FMEM;
			return(err);
		}
	}

	/* si encontr¢ final de mensaje coloca car cter '\0' */
	/* y retorna sin error */
	if(*l==CHR_DELIM) {
		*(*txt)++='\0';
		return(err);
	}

	/* si encontr¢ final de l¡nea lee la siguiente */
	if(fgets(lin,LONG_LIN,f_txt)==NULL) {
		/* si se produjo error sale indic ndolo */
		if(ferror(f_txt)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no es que ha llegado al final del fichero */
		/* sale sin indicar error por lo que donde se llame */
		/* a esta funci¢n se deber  comprobar el final de fichero */
		return(err);
	}
	l=lin;  /* apunta al inicio de la l¡nea */

	err.linea=num_linea(&l);
}

}

/****************************************************************************
	COMPILA_MSY: compila la secci¢n de Mensajes del sistema.
	  Entrada:      'f_msy' puntero al fichero con los mensajes
			'msy' puntero a inicio de buffer reservado para
			los mensajes
			'mem_msy' cantidad de memoria (bytes) reservada
			para los mensajes
			'max_msy' m ximo n£mero permitido para un mensaje
			'desp_msy' puntero a inicio de tabla donde se
			guardar n los desplazamientos de cada mensaje
			'n_msy' puntero a variable donde se guardar  el
			n£mero de mensajes compilados
			'bytes_msy' puntero a variable donde se guardar  la
			memoria ocupada por los mensajes
	  Salida:       c¢digo de error si lo hubo
****************************************************************************/
STC_ERR compila_msy(FILE *f_msy, char *msy, unsigned mem_msy, BYTE max_msy,
  unsigned *desp_msy, BYTE *n_msy, unsigned *bytes_msy)
{
STC_ERR err={_E_NERR, 0};
char *ult_msy, *act_msy, *ptr_act_msy;
BYTE i=0;
int nt, j;

/* inicializa tabla de desplazamientos de los mensajes */
for(j=0; j<MAX_MSY; j++) desp_msy[j]=0;

/* calcula £ltima posici¢n en buffer reservado para mensajes */
ult_msy=msy+mem_msy-1;
/* inicializa puntero a posici¢n donde ir  el mensaje actual */
act_msy=msy;

while(!feof(f_msy)) {
	/* guarda puntero a mensaje actual */
	ptr_act_msy=act_msy;

	err=comp_txt(f_msy,&act_msy,ult_msy,i,max_msy,&nt);
	/* sale si se produjo un error */
	if(err.codigo) return(err);

	/* calcula desplazamientos de mensajes, al final 'i' */
	/* contendr  el n£mero de siguiente mensaje a compilar */
	for(j=0; j<nt; j++, ptr_act_msy++, i++)
	  desp_msy[i]=(unsigned)(ptr_act_msy-msy);
}

/* guarda n£mero de mensajes compilados y la memoria ocupada */
*n_msy=--i;
*bytes_msy=(unsigned)(act_msy-msy);

return(err);
}

/****************************************************************************
	COMPILA_MSG: compila la secci¢n de Mensajes (compila una tabla de
	  mensajes en cada llamada).
	  Entrada:      'f_msg' puntero al fichero con los mensajes
			'msg' puntero a inicio de buffer reservado para
			los mensajes
			'mem_msg' cantidad de memoria (bytes) reservada
			para los mensajes
			'max_msg' m ximo n£mero permitido para un mensaje
			'max_tmsg' n£mero m ximo de tablas de mensajes
			'desp_msg' puntero a inicio de tabla donde se
			guardar n los desplazamientos de cada mensaje de
			la tabla actual
			'n_tmsg' puntero a variable donde se guardar 
			el n£mero de tabla que ha sido compilada
			'n_msg' puntero a variable donde se guardar  el
			n£mero de mensajes compilados en la tabla actual
			'bytes_msg' puntero a variable donde se guardar  la
			memoria ocupada por los mensajes de la tabla actual
	  Salida:       c¢digo de error si lo hubo, _E_MXXX si todav¡a
			quedan tablas de mensajes por compilar
****************************************************************************/
STC_ERR compila_msg(FILE *f_msg, char *msg, unsigned mem_msg, BYTE max_msg,
  BYTE max_tmsg, unsigned *desp_msg, BYTE *n_tmsg, BYTE *n_msg,
  unsigned *bytes_msg)
{
STC_ERR err={_E_NERR, 0};
char lin[LONG_LIN], *l, *ult_msg, *act_msg, *ptr_act_msg;
BYTE p, i=0;
static BYTE nmsg=0;
int nt, j;
fpos_t pos;
COD_ERR msgerr;

/* inicializa tabla de desplazamientos de los mensajes */
for(j=0; j<MAX_MSG; j++) desp_msg[j]=0;

/* lee l¡nea de entrada; se supone que es la primera */
/* de tabla de mensajes con el formato 'num_lin:\nnn...' */
if(fgets(lin,LONG_LIN,f_msg)==NULL) {
	if(ferror(f_msg)) {
		err.codigo=_E_LTMP;
		return(err);
	}
	/* si no fue error quiere decir que fue fin de fichero */
	return(err);
}

l=lin;
err.linea=num_linea(&l);
l++;    /* salta '\' */

/* recoge n£mero de tabla de mensajes y sale si n£mero */
/* no v lido, se sale del rango o est  fuera de secuencia */
msgerr=coge_num(l,&p);
if(msgerr) {
	/* no admite indirecci¢n, la trata como error en campo num‚rico */
	if(msgerr==_E_CIND) err.codigo=_E_CNUM;
	else err.codigo=msgerr;
	return(err);
}

if(p>max_tmsg) {
	err.codigo=_E_NVAL;
	return(err);
}

if(p<nmsg) {
	err.codigo=_E_NFSC;
	return(err);
}

/* guarda n£mero de tabla actual */
nmsg=p;

/* calcula £ltima posici¢n en buffer reservado para mensajes */
ult_msg=msg+mem_msg-1;
/* inicializa puntero a posici¢n donde ir  el mensaje actual */
act_msg=msg;

while(!feof(f_msg)) {
	/* coge siguiente l¡nea de fichero de entrada conservando */
	/* posici¢n actual de fichero */
	fgetpos(f_msg,&pos);
	if(fgets(lin,LONG_LIN,f_msg)==NULL) {
		if(ferror(f_msg)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error quiere decir que fue fin de fichero */
		break;
	}
	fsetpos(f_msg,&pos);

	/* si es inicio de tabla de mensajes finaliza */
	l=lin;
	num_linea(&l);
	if(*l=='\\') break;

	/* guarda puntero a mensaje actual */
	ptr_act_msg=act_msg;

	err=comp_txt(f_msg,&act_msg,ult_msg,i,max_msg,&nt);
	/* sale si se produjo un error */
	if(err.codigo) return(err);

	/* calcula desplazamientos de mensajes, al final 'i' */
	/* contendr  el n£mero de siguiente mensaje a compilar */
	for(j=0; j<nt; j++, ptr_act_msg++, i++)
	  desp_msg[i]=(unsigned)(ptr_act_msg-msg);
}

/* guarda n£mero de tabla actual, n£mero de mensajes compilados */
/* y la memoria ocupada */
*n_tmsg=nmsg;
*n_msg=i;
*bytes_msg=(unsigned)(act_msg-msg);

/* indica que quedan m s tablas por compilar */
err.codigo=_E_MXXX;

return(err);
}

/****************************************************************************
	ESTA_EN_VOC: comprueba si una palabra est  en el vocabulario.
	  Entrada:      'vocab' puntero a tabla de vocabulario
			'pvoc' n£mero de palabras en vocabulario
			'pal' puntero a palabra a buscar
	  Salida:       posici¢n dentro del vocabulario si se encontr¢, si no
			devuelve (NUM_PAL+1) que puede considerarse como
			palabra no encontrada
****************************************************************************/
int esta_en_voc(struct palabra *vocab, int pvoc, char *pal)
{
int i;

for(i=0; i<pvoc; i++) {
	if(!strcmp(pal,vocab[i].p)) return(i);
}

return(NUM_PAL+1);
}

/****************************************************************************
	COMPILA_LOC: compila la secci¢n de Localidades.
	  Entrada:      'f_loc' puntero al fichero con las localidades
			'loc' puntero a inicio de buffer reservado para
			los textos de localidades
			'mem_loc' cantidad de memoria (bytes) reservada
			para los textos de localidades
			'max_loc' m ximo n£mero permitido para una localidad
			'desp_loc' puntero a inicio de tabla donde se
			guardar n los desplazamientos de cada texto de
			localidad
			'conx' puntero a inicio de buffer reservado para
			las conexiones
			'mem_conx' cantidad de memoria (bytes) reservada
			para las conexiones
			'desp_conx' puntero a inicio de tabla donde se
			guardar n los desplazamientos de las conexiones
			'vocabulario' puntero a tabla de vocabulario
			'pal_voc' n£mero de palabras del vocabulario
			'v_mov' m ximo n£mero de verbo de movimiento
			'n_loc' puntero a variable donde se guardar  el
			n£mero de localidades compiladas
			'bytes_loc' puntero a variable donde se guardar  la
			memoria ocupada por los textos de localidades
			'bytes_conx' puntero a variable donde se guardar  la
			memoria ocupada por las conexiones
	  Salida:       c¢digo de error si lo hubo
****************************************************************************/
STC_ERR compila_loc(FILE *f_loc, char *loc, unsigned mem_loc, BYTE max_loc,
  unsigned *desp_loc, BYTE *conx, unsigned mem_conx, unsigned *desp_conx,
  struct palabra *vocabulario, int pal_voc, BYTE v_mov, BYTE *n_loc,
  unsigned *bytes_loc, unsigned *bytes_conx)
{
STC_ERR err={_E_NERR, 0};
char *ult_loc, *act_loc, *ptr_act_loc, lin[LONG_LIN], *l, pal_conx[LONGPAL+1];
BYTE *act_conx, *ult_conx, nloc=0, num, tipo, num_loc;
long pos_fich;
int npal, nt, i;
COD_ERR locerr;

/* inicializa tablas de desplazamientos de textos de localidades */
/* y de conexiones */
for(i=0; i<MAX_LOC; i++) {
	desp_loc[i]=0;
	desp_conx[i]=0;
}

/* puntero a £ltima posici¢n de buffer reservado a textos de localidades */
ult_loc=loc+mem_loc-1;
/* inicializa puntero al texto de localidad actual */
act_loc=loc;

/* puntero a £ltima posici¢n de buffer reservado a conexiones */
ult_conx=conx+mem_conx-1;
/* inicializa puntero a conexiones de localidad actual */
act_conx=conx;

while(1) {
	/* guarda puntero actual */
	ptr_act_loc=act_loc;

	/* compila el texto de la localidad */
	err=comp_txt(f_loc,&act_loc,ult_loc,nloc,max_loc,&nt);
	if(err.codigo) return(err);

	/* sale si fin de fichero */
	if(feof(f_loc)) break;

	/* guarda desplazamiento de conexiones de localidad actual */
	/* desp_conx[nloc]=act_conx-conx; */

	for(i=0; i<nt; i++, nloc++, ptr_act_loc++) {
		/* almacena desplazamientos de textos y conexiones */
		desp_loc[nloc]=(unsigned)(ptr_act_loc-loc);
		desp_conx[nloc]=(unsigned)(act_conx-conx);

		/* si son las localidades que salt¢ */
		if(i<(nt-1)) {
			/* marca que no tiene conexiones */
			*act_conx++=0;
			/* comprueba fuera de memoria */
			if(act_conx>=ult_conx) {
				err.codigo=_E_FMEM;
				return(err);
			}
		}
	}

	while(1) {
		/* almacena posici¢n en fichero */
		pos_fich=ftell(f_loc);

		/* recoge una l¡nea del fichero de entrada */
		/* sale si hubo error */
		if(fgets(lin,LONG_LIN,f_loc)==NULL) {
			if(ferror(f_loc)) {
				err.codigo=_E_LTMP;
				return(err);
			}
			/* si fin de fichero sale de bucle */
			break;
		}
		l=lin;  /* puntero al inicio de l¡nea */

		err.linea=num_linea(&l);

		/* si no encuentra como primer car cter de la l¡nea */
		/* MARCA_CNX restaura la posici¢n en el fichero y */
		/* sale del bucle */
		if(*l!=MARCA_CNX) {
			fseek(f_loc,pos_fich,SEEK_SET);
			break;
		}
		else {
			l=hasta_espacio(l);
			l=salta_espacios(l);

			/* coge el campo palabra */
			if((err.codigo=coge_pal(l,pal_conx))!=_E_NERR)
			  return(err);

			/* comprueba si la palabra est  en el vocabulario */
			npal=esta_en_voc(vocabulario,pal_voc,pal_conx);
			if(npal==(NUM_PAL+1)){
				err.codigo=_E_NPVC;
				return(err);
			}

			/* comprueba que sea verbo o nombre de movimiento */
			num=vocabulario[npal].num;
			tipo=vocabulario[npal].tipo;
			if(((tipo!=_VERB) && (tipo!=_NOMB)) || (num>=v_mov)) {
				err.codigo=_E_NMOV;
				return(err);
			}

			l=hasta_espacio(l);
			l=salta_espacios(l);

			/* si encuentra el final de la l¡nea sale con */
			/* error porque falta el n£mero de localidad con */
			/* la que conecta */
			if(fin_linea(*l)) {
				err.codigo=_E_FCAM;
				return(err);
			}

			/* coge y convierte el n£mero de localidad */
			locerr=coge_num(l,&num_loc);
			if(locerr) {
				/* no admite indirecci¢n, la trata como */
				/* error en campo num‚rico */
				if(locerr==_E_CIND) err.codigo=_E_CNUM;
				else err.codigo=locerr;
				return(err);
			}

			/* comprueba si n£mero de localidad es v lido */
			if(num_loc>(MAX_LOC-1)) {
				err.codigo=_E_NVAL;
				return(err);
			}

			/* guarda verbo de movimiento */
			*act_conx++=num;
			/* comprueba memoria */
			if(act_conx>=ult_conx) {
				err.codigo=_E_FMEM;
				return(err);
			}

			/* guarda n£mero de localidad a la que lleva */
			*act_conx++=num_loc;
			/* comprueba memoria */
			if(act_conx>=ult_conx) {
				err.codigo=_E_FMEM;
				return(err);
			}
		}
	}
	/* indica fin de conexiones de localidad actual */
	*act_conx++=0;
	/* comprueba fuera de memoria */
	if(act_conx>=ult_conx) {
		err.codigo=_E_FMEM;
		return(err);
	}
}

/* guarda n£mero de localidades compiladas y memoria ocupada por textos */
*n_loc=nloc;
*bytes_loc=(unsigned)(act_loc-loc);
*bytes_conx=(unsigned)(act_conx-conx);

return(err);
}

/****************************************************************************
	ES_BAND1: comprueba si el car cter dado es correspondiente a una
	  bandera de objeto (da lo mismo si es may£scula o min£scula).
	  Entrada:      'c' car cter a comprobar
	  Salida:       1 si 'P' (prenda)
			2 si 'L' (fuente de luz)
			0 otros  (no v lido)
****************************************************************************/
BYTE es_band1(char c)
{

switch(mayuscula(c)) {
case 'P' :
	return(1);
case 'L' :
	return(2);
}

return(0);
}

/****************************************************************************
	COMPILA_OBJ: compila la secci¢n de Objetos.
	  Entrada:      'f_obj' puntero al fichero con las localidades
			'obj' puntero a inicio de buffer reservado para
			los objetos
			'mem_obj' cantidad de memoria (bytes) reservada
			para los objetos
			'max_obj' m ximo n£mero permitido para un objeto
			'desp_obj' puntero a inicio de tabla donde se
			guardar n los desplazamientos de cada objeto
			'vocabulario' puntero a tabla de vocabulario
			'pal_voc' n£mero de palabras del vocabulario
			'n_loc' n£mero de localidades compiladas
			'n_obj' puntero a variable donde se guardar  el
			n£mero de objetos compilados
			'bytes_obj' puntero a variable donde se guardar  la
			memoria ocupada por los objetos
	  Salida:       c¢digo de error
****************************************************************************/
STC_ERR compila_obj(FILE *f_obj, char *obj, unsigned mem_obj, BYTE max_obj,
  unsigned *desp_obj, struct palabra *vocabulario, int pal_voc, BYTE n_loc,
  BYTE *n_obj, unsigned *bytes_obj)
{
STC_ERR err={_E_NERR, 0};
char *act_obj, *ult_obj, lin[LONG_LIN], *l, c, pal_obj[LONGPAL+1];
BYTE nobj=0, n, j, tipo, nombre, adjetivo, loc_ini, bands1, band, bands21,
  bands22;
int npal, i;
unsigned bands2, band2;
COD_ERR objerr;

/* inicializa tabla de desplazamientos de objetos */
for(i=0; i<MAX_OBJ; i++) desp_obj[i]=0;

/* inicializa puntero a objeto actual */
act_obj=obj;
/* inicializa puntero a £ltimo byte de buffer de objetos */
ult_obj=obj+mem_obj;

while(1) {
	/* lee l¡nea de entrada */
	if(fgets(lin,LONG_LIN,f_obj)==NULL) {
		if(ferror(f_obj)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error quiere decir que fue fin de fichero */
		/* en cuyo caso sale del bucle infinito */
		break;
	}
	/* inicializa puntero a l¡nea de entrada */
	l=lin;

	err.linea=num_linea(&l);

	/* comprueba si el primer car cter es CHR_DELIM */
	if(*l!=CHR_DELIM) {
		err.codigo=_E_FALT;
		return(err);
	}
	/* salta el delimitador */
	l++;

	/* coge el n£mero del objeto */
	objerr=coge_num(l,&n);
	if(objerr) {
		/* no admite indirecci¢n, la trata como error campo num‚rico */
		if(objerr==_E_CIND) err.codigo=_E_CNUM;
		else err.codigo=objerr;
		return(err);
	}
	/* comprueba es n£mero de objeto v lido */
	if(n>=MAX_OBJ) {
		err.codigo=_E_NVAL;
		return(err);
	}

	/* si el n£mero no es consecutivo pero es mayor, crea objetos */
	/* nulos hasta alcanzarlo */
	if(n>nobj) {
		for(j=nobj; j<n; j++) {
			/* almacena desplazamiento de objeto */
			desp_obj[nobj]=(unsigned)(act_obj-obj);

			/* comprobamos primero si hay memoria disponible */
			if(act_obj+6>=ult_obj) {
				err.codigo=_E_FMEM;
				return(err);
			}

			/* crea objeto nulo */
			*act_obj++=NO_PAL;      /* nombre */
			*act_obj++=NO_PAL;      /* adjetivo */
			*act_obj++=0;           /* localidad inicial */
			*act_obj++=0;           /* banderas */
			*act_obj++=0;           /* banderas de usuario */
			*act_obj++=0;
			*act_obj++='\0';        /* texto objeto */

			/* incrementa n£mero de objetos compilados */
			nobj++;
			if(nobj>max_obj) {
				err.codigo=_E_OPNV;
				return(err);
			}
		}
	}
	else if(n<nobj) {
		err.codigo=_E_NFSC;
		return(err);
	}

	/* guarda desplazamiento del objeto actual */
	desp_obj[nobj]=(unsigned)(act_obj-obj);

	l=hasta_espacio(l);
	l=salta_espacios(l);

	/* coge el campo palabra (nombre) */
	if((err.codigo=coge_pal(l,pal_obj))!=_E_NERR) return(err);

	if((npal=esta_en_voc(vocabulario,pal_voc,pal_obj))==(NUM_PAL+1)) {
		err.codigo=_E_NNVC;
		return(err);
	}

	/* comprueba que la palabra sea un nombre */
	/* si lo es guarda su n£mero */
	tipo=vocabulario[npal].tipo;
	if(tipo!=_NOMB) {
		err.codigo=_E_NNOM;
		return(err);
	}
	nombre=vocabulario[npal].num;

	l=hasta_espacio(l);
	l=salta_espacios(l);

	/* si no es indicador de cualquier palabra (CHR_NOPAL) */
	/* recoge la palabra comprobando que sea un adjetivo */
	/* si no guarda valor de NO_PAL */
	if(*l!=CHR_NOPAL) {
		/* coge el campo palabra (nombre) */
		if((err.codigo=coge_pal(l,pal_obj))!=_E_NERR) return(err);
		if((npal=esta_en_voc(vocabulario,pal_voc,
		  pal_obj))==(NUM_PAL+1)) {
			err.codigo=_E_NAVC;
			return(err);
		}
		tipo=vocabulario[npal].tipo;
		if(tipo!=_ADJT) {
			err.codigo=_E_NADJ;
			return(err);
		}
		adjetivo=vocabulario[npal].num;
	}
	else {
		adjetivo=NO_PAL;
		l++;
	}

	l=hasta_espacio(l);
	l=salta_espacios(l);

	/* coge el n£mero de localidad inicial */
	objerr=coge_num(l,&loc_ini);
	if(objerr) {
		/* no admite indirecci¢n, la trata como */
		/* error en campo num‚rico */
		if(objerr==_E_CIND) err.codigo=_E_CNUM;
		else err.codigo=objerr;
		return(err);
	}
	if(((loc_ini<NO_CREADO) && (loc_ini>=n_loc)) || loc_ini==LOC_ACTUAL) {
		err.codigo=_E_LINC;
		return(err);
	}

	/* recoge las banderas del objeto hasta que encuentre un */
	/* car cter que no se corresponda a uno v lido como bandera */
	bands1=0;
	do {
		l=hasta_espacio(l);
		l=salta_espacios(l);
		band=es_band1(*l);
		/* activa la bandera correspondiente y salta a la */
		/* siguiente posici¢n si fue v lida */
		bands1|=band;
		if(band) l++;
	} while(band);

	/* recoge las banderas de usuario del objeto */
	bands2=0;
	/* bit de mayor peso de variable auxiliar a 1 */
	band2=0x8000;
	for(i=0; i<16; i++) {
		c=mayuscula(*l);
		if((c!=BAND_0) && (c!=BAND_1)) {
			err.codigo=_E_BAND;
			return(err);
		}
		if(c==BAND_1) bands2|=band2;
		/* desplaza hacia la derecha */
		band2>>=1;
		l++;
	}
	/* separa la parte baja de la parte alta */
	bands22=(BYTE)(bands2 & 0x00ff);
	bands21=(BYTE)((bands2 >> 8) & 0x00ff);

	/* guardamos los datos recogidos */
	/* comprobando primero si hay memoria disponible */
	if(act_obj+5>=ult_obj) {
		err.codigo=_E_FMEM;
		return(err);
	}

	*act_obj++=nombre;     	/* nombre */
	*act_obj++=adjetivo;    /* adjetivo */
	*act_obj++=loc_ini;     /* localidad inicial */
	*act_obj++=bands1;      /* banderas */
	*act_obj++=bands21;     /* banderas de usuario */
	*act_obj++=bands22;

	/* lee l¡nea de entrada */
	if(fgets(lin,LONG_LIN,f_obj)==NULL) {
		if(ferror(f_obj)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error de lectura es que se lleg¢ al final */
		/* del fichero por lo que da error 'falta texto de objeto' */
		else {
			err.codigo=_E_OTEX;
			return(err);
		}
	}
	/* inicializa puntero a l¡nea de entrada */
	l=lin;

	err.linea=num_linea(&l);

	/* si encuentra inicio de otro objeto es que falta texto */
	if(*l==CHR_DELIM) {
		err.codigo=_E_OTEX;
		return(err);
	}

	/* compila texto de objeto */
	while(*l) {
		/* comprueba fuera de memoria */
		if(act_obj>=ult_obj) {
			err.codigo=_E_FMEM;
			return(err);
		}
		/* si encuentra un '\n' coloca final de texto */
		/* y sale del bucle */
		if(*l=='\n') {
			*act_obj++='\0';
			break;
		}
		/* si encuentra una tabulaci¢n la transforma en espacio */
		/* y avisa */
		else if(*l=='\t') {
			aviso(err.linea,Tab_Esp);
			*act_obj++=' ';
			l++;
		}
		/* en otro caso guarda el car cter encontrado */
		else *act_obj++=*l++;
	}

	/* siguiente objeto */
	nobj++;
	if(nobj>max_obj) {
		err.codigo=_E_OPNV;
		return(err);
	}

}

/* devuelve el n£mero de objetos compilados y la memoria ocupada */
*n_obj=nobj;
*bytes_obj=(unsigned)(act_obj-obj);

return(err);
}

/****************************************************************************
	COMPILA_PRO: compila la secci¢n de Procesos.
	  Entrada:      'f_pro' puntero al fichero con los procesos
			'pro' puntero a inicio de buffer reservado para
			los procesos
			'mem_pro' cantidad de memoria (bytes) reservada
			para los procesos
			'max_pro' m ximo n£mero permitido para un proceso
			'desp_pro' puntero a inicio de tabla donde se
			guardar n los desplazamientos de cada proceso
			'vocabulario' puntero a tabla de vocabulario
			'pal_voc' n£mero de palabras del vocabulario
			'n_loc' n£mero de localidades compiladas
			'n_msy' n£mero de mensajes del sistema compilados
			'n_obj' n£mero de objetos compilados
			'n_conv' m ximo n£mero de nombre convertible
			'n_pro' puntero a variable donde se guardar  el
			n£mero de procesos compilados
			'bytes_pro' puntero a variable donde se guardar  la
			memoria ocupada por los procesos
	  Salida:       c¢digo de error si lo hubo
****************************************************************************/
STC_ERR compila_pro(FILE *f_pro, BYTE *pro, unsigned mem_pro, BYTE max_pro,
  unsigned *desp_pro, struct palabra *vocabulario, int pal_voc, BYTE n_loc,
  BYTE n_msy, BYTE n_obj, BYTE n_conv, BYTE *n_pro, unsigned *bytes_pro)
{
STC_ERR err={_E_NERR, 0};
char lin[LONG_LIN], *l, etq[LONGETQ+1], pal[LONGPAL+1], cond[LNGCOND+1];
BYTE p, j, *act_pro, *ult_pro, npro=0, *sgte_entr;
long pos_fich;
int i, npal;
BOOLEAN pra_lin_entr;
unsigned se;
COD_ERR proerr;

/* inicializa tabla de desplazamientos de procesos */
for(i=0; i<MAX_PRO; i++) desp_pro[i]=0;

/* inicializa puntero a buffer de proceso actual */
act_pro=pro;
/* puntero a £ltimo byte de buffer reservado para procesos */
ult_pro=pro+mem_pro;

while(1) {
	/* lee l¡nea de entrada; se supone que es la primera */
	/* del proceso con el formato 'num_lin:\nnn...' */
	if(fgets(lin,LONG_LIN,f_pro)==NULL) {
		if(ferror(f_pro)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error quiere decir que fue fin de fichero */
		/* en cuyo caso sale del bucle infinito */
		break;
	}
	/* inicializa puntero a l¡nea de entrada */
	l=lin;

	err.linea=num_linea(&l);
	l++;    /* salta '\' */

	/* recoge n£mero de proceso y sale si n£mero no v lido */
	/* o se sale del rango */
	proerr=coge_num(l,&p);
	if(proerr) {
		/* no admite indirecci¢n, la trata como */
		/* error en campo num‚rico */
		if(proerr==_E_CIND) err.codigo=_E_CNUM;
		else err.codigo=proerr;
		return(err);
	}
	if(p>max_pro) {
		err.codigo=_E_NVAL;
		return(err);
	}

	/* si el n£mero no es consecutivo pero es mayor, crea procesos */
	/* nulos hasta alcanzarlo */
	if(p>npro) {
		for(j=npro; j<p; j++) {
			/* guarda desplazamiento de proceso */
			desp_pro[npro]=(unsigned)(act_pro-pro);

			/* comprueba fuera de memoria */
			if(act_pro+1>=ult_pro) {
				err.codigo=_E_FMEM;
				return(err);
			}

			/* crea proceso nulo */
			*act_pro++=0;
			*act_pro++=0;

			/* incrementa n£mero de proceso actual */
			npro++;
		}
	}
	else if(p<npro) {
		err.codigo=_E_NFSC;
		return(err);
	}

	/* actualiza variable global 'num_pro_act' */
	num_pro_act=npro;

	/* las etiquetas son locales, es decir, s¢lo valen dentro del */
	/* proceso en el que fueron definidas */
	/* inicializa puntero a tabla de etiquetas */
	pt_etiq=0;
	/* inicializa puntero a tabla de referencias forward */
	pt_skip=0;

	/* guarda desplazamiento de proceso */
	desp_pro[npro]=(unsigned)(act_pro-pro);

	/* guarda posici¢n en fichero */
	pos_fich=ftell(f_pro);
	/* lee l¡nea de entrada */
	if(fgets(lin,LONG_LIN,f_pro)==NULL) {
		if(ferror(f_pro)) {
			err.codigo=_E_LTMP;
			return(err);
		}
		/* si no fue error quiere decir que fue fin de fichero */
		/* en cuyo caso sale del bucle infinito */
		break;
	}
	/* inicializa puntero a l¡nea de entrada */
	l=lin;
	err.linea=num_linea(&l);

	while(1) {
		/* si encuentra marca de inicio de proceso '\' */
		/* quiere decir que empieza un nuevo proceso y por */
		/* tanto acaba el actual */
		if(*l=='\\') {
			/* siguiente n£mero de proceso */
			npro++;

			/* comprueba fuera de memoria */
			if(act_pro+1>=ult_pro) {
				err.codigo=_E_FMEM;
				return(err);
			}
			/* pone marca de fin de proceso */
			*act_pro++=0;
			*act_pro++=0;

			/* recupera posici¢n en fichero */
			fseek(f_pro,pos_fich,SEEK_SET);

			/* sale del bucle para compilar siguiente proceso */
			break;
		}

		/* el primer car cter de la l¡nea no debe ser un blanco */
		/* ya que si no la entrada no es v lida */
		if(es_espacio(*l)) {
			err.codigo=_E_NENT;
			return(err);
		}

		/*** ETIQUETAS ***/
		if(*l==MARCA_ETQ) {
			/* comprueba si la tabla de etiquetas est  llena */
			if(pt_etiq>=LABELS) {
				err.codigo=_E_RBTL;
				return(err);
			}
			l++;

			/* recoge etiqueta */
			if(coge_nombre_etq(&l,etq,LONGETQ+1)) {
				err.codigo=_E_ENVL;
				return(err);
			}

			/* si el £ltimo car cter no es fin de l¡nea */
			/* ni espacio, error en nombre de etiqueta */
			if(!fin_linea(*l) && !es_espacio(*l)) {
				err.codigo=_E_ENVL;
				return(err);
			}

			/* si la tabla de etiquetas est  vac¡a, mete la */
			/* etiqueta sin m s, si no mira si est  ya definida */
			if(!pt_etiq) {
				strcpy(etiqueta[pt_etiq].etq,etq);
				etiqueta[pt_etiq].petq=act_pro;
			}
			else {
				for(i=0; i<pt_etiq; i++) {
					/* error si etiqueta ya definida */
					if(!strcmp(etq,etiqueta[i].etq)) {
					      err.codigo=_E_EREP;
					      return(err);
					}
				}
				/* guarda etiqueta y su direcci¢n */
				strcpy(etiqueta[pt_etiq].etq,etq);
				etiqueta[pt_etiq].petq=act_pro;
			}
			/* siguiente entrada en la tabla de etiquetas */
			pt_etiq++;

			/* lee l¡nea de entrada */
			if(fgets(lin,LONG_LIN,f_pro)==NULL) {
				if(ferror(f_pro)) {
					err.codigo=_E_LTMP;
					return(err);
				}
				/* si fin de fichero sale de bucle */
				break;
			}
		}

		/* inicializa puntero a l¡nea de entrada */
		l=lin;
		err.linea=num_linea(&l);

		/* espera encontrar un verbo o nombre convertible */
		/* y un nombre (o la palabra nula CHR_NOPAL) */

		/* si es palabra nula (CHR_NOPAL) guarda NO_PAL */
		if(*l==CHR_NOPAL) *act_pro++=NO_PAL;
		else {
			/* recoge el campo verbo */
			if((err.codigo=coge_pal(l,pal))!=_E_NERR) return(err);

			/* comprueba si est  en el vocabulario */
			npal=esta_en_voc(vocabulario,pal_voc,pal);
			if(npal==NUM_PAL+1) {
				err.codigo=_E_NPVC;
				return(err);
			}
			/* si no es verbo ni nombre convertible */
			if((vocabulario[npal].tipo!=_VERB) &&
			  ((vocabulario[npal].tipo==_NOMB) &&
			  (vocabulario[npal].num>=n_conv))) {
				err.codigo=_E_NVNC;
				return(err);
			}
			/* guarda n£mero de verbo o nombre convertible */
			*act_pro++=vocabulario[npal].num;
		}
		/* comprueba fuera de memoria */
		if(act_pro>=ult_pro) {
			err.codigo=_E_FMEM;
			return(err);
		}

		l=hasta_espacio(l);
		l=salta_espacios(l);

		/* si es palabra nula (CHR_NOPAL) guarda NO_PAL */
		if(*l==CHR_NOPAL) *act_pro++=NO_PAL;
		else {
			/* recoge el campo nombre */
			if((err.codigo=coge_pal(l,pal))!=_E_NERR) return(err);

			/* comprueba si est  en el vocabulario */
			npal=esta_en_voc(vocabulario,pal_voc,pal);
			if(npal==NUM_PAL+1) {
				err.codigo=_E_NNVC;
				return(err);
			}
			/* comprueba si es nombre */
			if(vocabulario[npal].tipo!=_NOMB) {
				err.codigo=_E_NNNN;
				return(err);
			}
			/* guarda n£mero de nombre */
			*act_pro++=vocabulario[npal].num;
		}
		/* comprueba fuera de memoria */
		if(act_pro>=ult_pro) {
			err.codigo=_E_FMEM;
			return(err);
		}

		/* guarda posici¢n donde meter el desplazamiento de */
		/* la siguiente entrada */
		sgte_entr=act_pro;
		/* salta 2 bytes para empezar a compilar la entrada */
		act_pro+=2;
		/* comprueba fuera de memoria */
		if(act_pro>=ult_pro) {
			err.codigo=_E_FMEM;
			return(err);
		}

		/*** COMPILA CONDACTOS ***/
		/* espera encontrar un condacto */
		/* indica que estamos en primera linea de entrada */
		pra_lin_entr=TRUE;
		while(1) {
			/* si es primera linea entrada debe saltar */
			/* el campo nombre y dejar el puntero al inicio */
			/* del condacto */
			if(pra_lin_entr) {
				l=hasta_espacio(l);
				pra_lin_entr=FALSE;
			}
			else {
				/* guarda la posici¢n en el fichero */
				pos_fich=ftell(f_pro);

				/* lee l¡nea de entrada */
				if(fgets(lin,LONG_LIN,f_pro)==NULL) {
					if(ferror(f_pro)) {
						err.codigo=_E_LTMP;
						return(err);
					}
					/* si fin de fichero sale de bucle */
					break;
				}
				l=lin;
				err.linea=num_linea(&l);

				/* si el primer car cter no es espacio */
				/* finaliza el proceso actual */
				if((*l!=' ') && (*l!='\t')) {
					/* marca fin de entrada */
					*act_pro++=0;
					/* comprueba fuera de memoria */
					if(act_pro>=ult_pro) {
						err.codigo=_E_FMEM;
						return(err);
					}

					/* guarda el desplazamiento de la */
					/* siguiente entrada respecto al */
					/* inicio del proceso */
					se=(unsigned)(act_pro-pro)-
					  desp_pro[npro];
					/* byte bajo */
					*sgte_entr=(BYTE)(se & 0x00ff);
					/* byte alto */
					*(sgte_entr+1)=(BYTE)((se >> 8)
					  & 0x00ff);

					break;  /* fin de entrada */
				}
			}

			l=salta_espacios(l);

			/* recoge el condacto */
			for(i=0; i<LNGCOND; i++, l++) {
				cond[i]=mayuscula(*l);
				if(!esta_en(Abecedario,cond[i]) &&
				  !esta_en(Numeros,cond[i])) break;
			}
			if((*l!=' ') && (*l!='\t') && (*l!='\n')) {
				err.codigo=_E_NCND;
				return(err);
			}

			/* rellena con espacios y a¤ade fin de cadena */
			for(; i<LNGCOND; cond[i++]=' ');
			cond[i]='\0';

			/* busca el condacto */
			for(i=1; i<N_CONDACTOS; i++) {
				if(!strcmp(cond,condacto[i].cnd)) break;
			}

			/* retorna con error si condacto no encontrado */
			/* o no v lido */
			if(i==N_CONDACTOS) {
				err.codigo=_E_NCND;
				return(err);
			}

			/* compila seg£n tipo */
			switch(condacto[i].tipo) {
				case 0 :
					*act_pro++=(BYTE)i;
					err.codigo=_E_NERR;
					break;
				case 1 :
					err.codigo=process(&l,&act_pro,
					  ult_pro,(BYTE)i);
					break;
				case 2 :
					err.codigo=compila_par7(&l,&act_pro,
					  ult_pro,(BYTE)i,N_VENT-1,255,
					  _E_WPNW,_E_NERR);
					break;
				case 3 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,N_VENT-1,255,
					  _E_WPNW);
					break;
				case 4 :
					err.codigo=compila_par2(&l,&act_pro,
					  ult_pro,(BYTE)i,255,255,0,_E_NERR,
					  _E_NERR);
					break;
				case 5 :
					err.codigo=compila_par2(&l,&act_pro,
					  ult_pro,(BYTE)i,MAX_TMSG-1,MAX_MSG-1,
					  255,_E_TMSG,_E_MPNM);
					break;
				case 6 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,(BYTE)(n_msy-1),255,
					  _E_MPNM);
					break;
				case 7 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,(BYTE)(n_loc-1),255,
					  _E_MPNL);
					break;
				case 8 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,255,0,_E_NERR);
					break;
				case 9 :
					err.codigo=compila_par2(&l,&act_pro,
					  ult_pro,(BYTE)i,(BYTE)(n_obj-1),
					  (BYTE)(n_loc-1),NO_CREADO,
					  _E_OPNV,_E_MPNL);
					break;
				case 10 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,(BYTE)(n_obj-1),255,
					  _E_OPNV);
					break;
				case 11 :
					err.codigo=skip(&l,&act_pro,ult_pro,
					  (BYTE)i);
					break;
				case 12 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,(BYTE)(n_loc-1),
					  NO_CREADO,_E_MPNL);
					break;
				case 13 :
					err.codigo=compila_par1_pal(&l,
					  &act_pro,ult_pro,(BYTE)i,pal_voc,
					  _ADJT,_E_NCAD);
					break;
				case 14 :
					err.codigo=compila_par1_pal(&l,
					  &act_pro,ult_pro,(BYTE)i,pal_voc,
					  _NOMB,_E_NNMB);
					break;
				case 15 :
					err.codigo=compila_par2(&l,&act_pro,
					  ult_pro,(BYTE)i,(BYTE)(n_obj-1),
					  (BYTE)(n_obj-1),255,_E_OPNV,_E_OPNV);
					break;
				case 16 :
					err.codigo=synonym(&l,&act_pro,
					  ult_pro,(BYTE)i,pal_voc,n_conv);
					break;
				case 17 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,17,255,_E_NVHT);
					break;
				case 18 :
					err.codigo=compila_par2(&l,&act_pro,
					  ult_pro,(BYTE)i,(BYTE)(n_obj-1),255,
					  0,_E_OPNV,_E_NERR);
					break;
				case 19 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,100,255,_E_CHNV);
					break;
				case 20 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,BANCOS_RAM-1,255,
					  _E_RBNV);
					break;
				case 21 :
					err.codigo=compila_par3(&l,&act_pro,
					  ult_pro,(BYTE)i,BANCOS_RAM-1,255,
					  _E_RBNV,_E_NERR);
					break;
				case 22 :
					err.codigo=compila_par3(&l,&act_pro,
					  ult_pro,(BYTE)i,(BYTE)(n_msy-1),
					  (BYTE)(n_msy-1),_E_MPNP,_E_MPNS);
					break;
				case 23 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,1,255,_E_NO01);
					break;
				case 24 :
					err.codigo=compila_par2(&l,&act_pro,
					  ult_pro,(BYTE)i,NUM_NOTAS-1,255,0,
					  _E_NOTA,_E_NERR);
					break;
				case 25 :
					err.codigo=_E_TABC;
					break;
				case 26 :
					err.codigo=compila_par1(&l,&act_pro,
					  ult_pro,(BYTE)i,1,255,_E_COLR);
					break;
				case 27 :
					err.codigo=compila_par2(&l,&act_pro,
					  ult_pro,(BYTE)i,N_VENT-1,N_BORD-1,
					  255,_E_WPNW,_E_NBWN);
					break;
				case 28 :
					err.codigo=compila_par2(&l,&act_pro,
					  ult_pro,(BYTE)i,255,1,255,_E_NERR,
					  _E_SP01);
					break;
				case 29 :
					err.codigo=compila_par4(&l,&act_pro,
					  ult_pro,(BYTE)i,255,63,63,63,
					  _E_NERR,_E_CRGB,_E_CRGB,_E_CRGB);
					break;
				case 30 :
					err.codigo=compila_par4(&l,&act_pro,
					  ult_pro,(BYTE)i,255,255,255,255,
					  _E_NERR,_E_NERR,_E_NERR,_E_NERR);
					break;
				case 31 :
					err.codigo=compila_par3(&l,&act_pro,
					  ult_pro,(BYTE)i,255,255,_E_NERR,
					  _E_NERR);
					break;
			}

			/* retorna si hubo alg£n error */
			if(err.codigo!=_E_NERR) return(err);

			/* si quedan caracteres al final de la l¡nea */
			/* dar  un mensaje de aviso */
			l=hasta_espacio(l);
			l=salta_espacios(l);
			if(!fin_linea(*l) && *l!=';') {
				aviso(err.linea,"demasiados campos, se"
				  " ignoran los sobrantes");
			}
		}

	if(feof(f_pro)) break;
	}
/* ahora sustituye las etiquetas */
err=sust_etiquetas();
if(err.codigo) return(err);
}

/* pone desplazamiento a siguiente entrada en la £ltima entrada */
se=(unsigned)(act_pro-pro)-desp_pro[npro];
*sgte_entr=(BYTE)(se & 0x00ff);
*(sgte_entr+1)=(BYTE)((se >> 8) & 0x00ff);

/* indica fin de £ltimo proceso */
*act_pro++=0;
*act_pro++=0;

/* ahora procesa llamadas a procesos 'forward' */
err=chequea_pro(npro);

/* devuelve el n£mero de procesos compilados y la memoria ocupada */
*n_pro=++npro;
*bytes_pro=(unsigned)(act_pro-pro);

return(err);
}

/****************************************************************************
	COGE_NOMBRE_ETQ: recoge el nombre de una etiqueta o una constante
	  Entrada:      'l' puntero a puntero a la posici¢n de la l¡nea donde
			comienza el nombre de la etiqueta o constante
			'etq' puntero a buffer donde se colocar  el
			nombre de la etiqueta o constante
			'lngbuff' tama¤o del buffer
	  Salida:       TRUE si hubo error y adem s deja 'l' apuntando al
			primer car cter que no se corresponda con uno v lido
			para nombre de etiqueta o constante
			FALSE si no
****************************************************************************/
BOOLEAN coge_nombre_etq(char *(*l), char *etq, int lngbuff)
{
int i;

for(i=0; i<lngbuff-1; i++, (*l)++) {
	/* sale si encuentra fin de l¡nea o espacio */
	if(fin_linea(*(*l)) || es_espacio(*(*l))) break;
	etq[i]=mayuscula(*(*l));
	/* si encuentra un car cter no v lido como nombre de etiqueta */
	/* o constante sale con error */
	if(!esta_en(Abecedario,etq[i]) && !esta_en(Numeros,etq[i]) &&
	  !esta_en(Signos,etq[i])) return(TRUE);
}
/* marca fin de etiqueta o constante */
etq[i]='\0';

/* si no se pudo recoger nombre de etiqueta o constante */
/* es que no es v lido, si no deja 'l' apuntando al primer car cter */
/* que no sea v lido para nombre de etiqueta o constante */
if(!i) return(TRUE);
else while(esta_en(Abecedario,mayuscula(*(*l))) && esta_en(Numeros,*(*l))
  && esta_en(Signos,*(*l))) (*l)++;

return(FALSE);
}

/****************************************************************************
	SUST_ETIQUETAS: sutituye referencias 'forward' en saltos SKIP.
	  Salida:       c¢digo de error
****************************************************************************/
STC_ERR sust_etiquetas(void)
{
STC_ERR err={_E_NERR, 0};
int i, j;
unsigned desplu;

for(i=0; i<pt_skip; i++) {
	/* busca la etiqueta a sustituir (fskip[i].etq) en la */
	/* tabla de etiquetas definidas (etiqueta[j].etq) */
	for(j=0; j<pt_etiq; j++) {
		if(!strcmp(fskip[i].etq,etiqueta[j].etq)) break;
	}
	/* si no encontr¢ la etiqueta pone en la variable global */
	/* 'numlin_fich_entr' el n£mero de l¡nea del error */
	if(j==pt_etiq) {
		err.codigo=(_E_RFFW);
		err.linea=fskip[i].nl;
		return(err);
	}

	/* si la etiqueta est  definida calcula el desplazamiento */
	/* comprueba que no se salga de rango y lo introduce en el c¢digo */
	desplu=(unsigned)(etiqueta[j].petq-(fskip[i].fsk-1));
	if(desplu>(unsigned)32767) {
		err.codigo=_E_LFFR;
		err.linea=fskip[i].nl;
		return(err);
	}
	*(fskip[i].fsk)=(BYTE)(desplu & 0x00ff);
	*(fskip[i].fsk+1)=(BYTE)((desplu >> 8) & 0x00ff);
}

return(err);
}

/****************************************************************************
	CHEQUEA_PRO: comprueba si hay alguna llamada a un proceso inexistente.
	  Entrada:      'num_pro' n£mero de procesos compilados
	  Salida:       c¢digo de error
****************************************************************************/
STC_ERR chequea_pro(BYTE num_pro)
{
STC_ERR err={_E_NERR, 0};
int i;

for(i=0; i<pprc; i++) {
	/* mira a ver si se llama a un proceso no definido */
	if(prc[i].numpro>num_pro) {
		err.codigo=_E_PRLL;
		err.linea=prc[i].nl;
		return(err);
	}
}

return(err);
}

/****************************************************************************
	METE_CONST: mete una constante en la tabla de constantes.
	  Entrada:      'l' puntero a l¡nea de entrada conteniendo
			la constante
			'l¡nea' n£mero de l¡nea del fichero donde se define
			la constante (0 ninguna)
	  Salida:       c¢digo de error
****************************************************************************/
COD_ERR mete_const(char *l, unsigned long linea)
{
char cnst[LNGCONST+1], num[4];
BYTE val;
int i, vali;

/* mira si la tabla est  llena */
if(pt_const>=NCONST) return(_E_TCCN);

/* el primer car cter del nombre de la constante no puede ser n£mero */
if(esta_en(Numeros,*l)) return(_E_PCCN);

if(coge_nombre_etq(&l,cnst,LNGCONST+1)) return(_E_NNCT);

/* si lo que sigue al nombre de la constante no es espacio, da error */
if(!es_espacio(*l)) return(_E_NNCT);

l=salta_espacios(l);

/* si encuentra fin de l¡nea o inicio de comentario (CHR_COMENT) */
/* da error ya que falta valor de constante */
if(fin_linea(*l) || (*l==CHR_COMENT)) return(_E_FVCN);

/* coge el valor de la constante */
i=0;
while(esta_en(Numeros,*l) && (i<3)) num[i++]=*l++;
num[i]='\0';    /* marca fin de cadena */

/* si recogi¢ alg£n n£mero */
if(i) {
	vali=atoi(num);
	/* comprueba si n£mero fuera de rango 8 bits */
	if((vali<0) || (vali>255)) return(_E_VNCT);
	val=(BYTE)vali;
}
/* si no pudo recoger ning£n n£mero sale con error */
else return(_E_VNCT);

/* comprueba si hay alguna constante con el mismo nombre */
/* si es as¡ da un mensaje de aviso */
for(i=0; i<pt_const; i++) {
	if(!strcmp(cnst,constante[i].cnst))
	  aviso(linea,"constante redefinida: %s",cnst);
}

strcpy(constante[pt_const].cnst,cnst);
constante[pt_const].valor=val;

/* siguiente entrada en la tabla */
pt_const++;

return(_E_NERR);
}

/****************************************************************************
	COGE_CONST: recoge una constante de la tabla de constantes.
	  Entrada:      'cnst' nombre de la constante a recoger
			'valor' puntero a variable donde se dejar 
			el valor de la constante (quedar  sin modificar
			si no se encontr¢ la constante)
	  Salida:       FALSE si no hubo error
			TRUE si la constante no fue definida
****************************************************************************/
BOOLEAN coge_const(char *cnst, BYTE *valor)
{
int i;

/* si no se defini¢ ninguna constante sale con error */
if(!pt_const) return(TRUE);

/* busca la constante */
for(i=0; i<pt_const; i++) {
	if(!strcmp(constante[i].cnst,cnst)) break;
}

/* si no la encontr¢ sale con error */
if(i==pt_const) return(TRUE);

/* coge el valor de la constante y lo devuelve */
*valor=constante[i].valor;

return(FALSE);
}

/****************************************************************************
	SGTE_CAMPO: desplaza puntero de l¡nea al siguiente campo.
	  Entrada:      'l' puntero a l¡nea de entrada (quedar  actualizado)
	  Salida:       TRUE si no hay m s campos
			FALSE si hay m s campos
****************************************************************************/
BOOLEAN sgte_campo(char *(*l))
{

*l=hasta_espacio(*l);
*l=salta_espacios(*l);

/* comprueba si hay m s campos */
if(fin_linea(*(*l))) return(TRUE);

return(FALSE);
}

/****************************************************************************
	METE_FSK: introduce etiqueta y su direcci¢n de sustituci¢n para una
	  llamada 'forward' con SKIP.
	  Entrad:       'etq' etiqueta a introducir
			'dir' direcci¢n donde meter el valor de la etiqueta
			cuando se calcule
	  Salida:       TRUE si la tabla de sustituci¢n de SKIP est  llena
			FALSE si no hubo error
****************************************************************************/
int mete_fsk(char *etq, BYTE *dir)
{
int i;

/* comprueba si se rebosa la tabla */
if(pt_skip>=FSKIP) return(TRUE);

/* guarda la etiqueta, su direcci¢n y la l¡nea dentro de fichero de entrada */
for(i=0; i<LONGETQ+1; i++) fskip[pt_skip].etq[i]=etq[i];
fskip[pt_skip].fsk=dir;
fskip[pt_skip].nl=numlin_fich_entr;
pt_skip++;

return(FALSE);
}

/****************************************************************************
	COMPILA_PAR1: compila condactos con un par metro.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'codigo' c¢digo num‚rico del condacto
			'lim1', 'lim2' l¡mites para el par metro (NOTA: meter
			255 y 0 respectivamente para saltar comprobaci¢n o
			'lim1' y 255 para comprobar entre 0..lim1)
			'coderr' c¢digo de error en caso de que el par metro
			sea mayor que 'lim1' y menor que 'lim2'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR compila_par1(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim1, BYTE lim2, COD_ERR coderr)
{
BYTE par;
COD_ERR indir;

/* comprueba si cabe en memoria */
if(*act_pro+4>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par);
/* si hubo error al coger par metro 1, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);

/* comprueba si es mayor que 'lim1' y menor que 'lim2' */
if((indir!=_E_CIND) && (par>lim1) && (par<lim2)) return(coderr);
if(indir==_E_CIND) {
	*(*act_pro)++=INDIR;
	*(*act_pro)++=0x01;
}

*(*act_pro)++=codigo;
*(*act_pro)++=par;

return(_E_NERR);
}

/****************************************************************************
	COMPILA_PAR1_PAL: compila condactos con un par metro, el cual es
	  una palabra del vocabulario.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'codigo' c¢digo num‚rico del condacto
			'pal_voc' n£mero de palabras en vocabulario
			'tipopal' tipo de palabra esperada (_VERB, _NOMB,
			_ADJT, _CONJ)
			'coderr' c¢digo de error en caso de que la palabra
			encontrada no sea del tipo especificado por 'tipopal'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR compila_par1_pal(char *(*l), BYTE *(*act_pro), BYTE *ult_pro,
  BYTE codigo, int pal_voc, BYTE tipopal, COD_ERR coderr)
{
int numpal;
char palabra[LONGPAL+1];

/* comprueba si cabe en memoria */
if(*act_pro+3>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);
if(coge_pal(*l,palabra)==_E_CVOC) return(_E_CVOC);

/* saca la palabra del vocabulario */
if((numpal=esta_en_voc(vocabulario,pal_voc,palabra))==(NUM_PAL+1))
  return(_E_NPVC);

/* comprueba que sea del tipo especificado */
if(vocabulario[numpal].tipo!=tipopal) return(coderr);

*(*act_pro)++=codigo;
*(*act_pro)++=vocabulario[numpal].num;

return(_E_NERR);
}

/****************************************************************************
	COMPILA_PAR2: compila condactos con dos par metros.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'codigo' c¢digo num‚rico del condacto
			'lim11' l¡mite para el par metro 1 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim21', 'lim22' l¡mites para el par metro 2 (NOTA:
			meter 255 y 0 respectivamente para saltar
			comprobaci¢n o 'lim21' y 255 para comprobar 0..lim21)
			'coderr1' c¢digo de error en caso de que el par metro
			1 sea mayor que 'lim11'
			'coderr2' c¢digo de error en caso de que el par metro
			2 sea mayor que 'lim21' y menor que 'lim22'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR compila_par2(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim11, BYTE lim21, BYTE lim22, COD_ERR coderr1, COD_ERR coderr2)
{
BYTE par1, par2, nindir=0;
COD_ERR indir;

/* comprueba si cabe en memoria */
if(*act_pro+5>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par1);
/* si hubo error al coger el par metro 1, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim11' */
if((indir!=_E_CIND) && (par1>lim11)) return(coderr1);
if(indir==_E_CIND) nindir |= 0x01;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par2);
/* si hubo error al coger el par metro 2, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim21' y menor que 'lim22' */
/* si 'lim22' es 255 comprueba entre 0 y 'lim21' */
if((lim22!=255) && (indir!=_E_CIND) && (par2>lim21) && (par2<lim22))
  return(coderr2);
else if((lim22==255) && (indir!=_E_CIND) && (par2>lim21)) return(coderr2);
if(indir==_E_CIND) nindir |= 0x02;

if(nindir) {
	*(*act_pro)++=INDIR;
	*(*act_pro)++=nindir;
}

*(*act_pro)++=codigo;
*(*act_pro)++=par1;
*(*act_pro)++=par2;

return(_E_NERR);
}

/****************************************************************************
	COMPILA_PAR3: compila condactos con tres par metros.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'codigo' c¢digo num‚rico del condacto
			'lim1' l¡mite para el par metro 1 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim2' l¡mite para el par metro 2 (NOTA: meter
			255 para saltar comprobaci¢n)
			'coderr1' c¢digo de error en caso de que el par metro
			1 sea mayor que 'lim1'
			'coderr2' c¢digo de error en caso de que el par metro
			2 sea mayor que 'lim2'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR compila_par3(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim1, BYTE lim2, COD_ERR coderr1, COD_ERR coderr2)
{
BYTE par1, par2, par3, nindir=0;
COD_ERR indir;

/* comprueba si cabe en memoria */
if(*act_pro+6>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par1);
/* si hubo error al coger el par metro 1, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim1' */
if((indir!=_E_CIND) && (par1>lim1)) return(coderr1);
if(indir==_E_CIND) nindir |= 0x01;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par2);
/* si hubo error al coger el par metro 2, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim2' */
if((indir!=_E_CIND) && (par2>lim2)) return(coderr2);
if(indir==_E_CIND) nindir |= 0x02;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par3);
/* si hubo error al coger el par metro 3, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
if(indir==_E_CIND) nindir |= 0x04;

if(nindir) {
	*(*act_pro)++=INDIR;
	*(*act_pro)++=nindir;
}

*(*act_pro)++=codigo;
*(*act_pro)++=par1;
*(*act_pro)++=par2;
*(*act_pro)++=par3;

return(_E_NERR);
}

/****************************************************************************
	COMPILA_PAR4: compila condactos con cuatro par metros.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'codigo' c¢digo num‚rico del condacto
			'lim1' l¡mite para el par metro 1 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim2' l¡mite para el par metro 2 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim3' l¡mite para el par metro 3 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim4' l¡mite para el par metro 4 (NOTA: meter
			255 para saltar comprobaci¢n)
			'coderr1' c¢digo de error en caso de que el par metro
			1 sea mayor que 'lim1'
			'coderr2' c¢digo de error en caso de que el par metro
			2 sea mayor que 'lim2'
			'coderr3' c¢digo de error en caso de que el par metro
			3 sea mayor que 'lim3'
			'coderr4' c¢digo de error en caso de que el par metro
			4 sea mayor que 'lim4'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR compila_par4(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim1, BYTE lim2, BYTE lim3, BYTE lim4, COD_ERR coderr1,
  COD_ERR coderr2, COD_ERR coderr3, COD_ERR coderr4)
{
BYTE par1, par2, par3, par4, nindir=0;
COD_ERR indir;

/* comprueba si cabe en memoria */
if(*act_pro+7>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par1);
/* si hubo error al coger el par metro 1, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim1' */
if((indir!=_E_CIND) && (par1>lim1)) return(coderr1);
if(indir==_E_CIND) nindir |= 0x01;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par2);
/* si hubo error al coger el par metro 2, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim2' */
if((indir!=_E_CIND) && (par2>lim2)) return(coderr2);
if(indir==_E_CIND) nindir |= 0x02;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par3);
/* si hubo error al coger el par metro 3, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim3' */
if((indir!=_E_CIND) && (par3>lim3)) return(coderr3);
if(indir==_E_CIND) nindir |= 0x04;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par4);
/* si hubo error al coger el par metro 4, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim4' */
if((indir!=_E_CIND) && (par4>lim4)) return(coderr4);
if(indir==_E_CIND) nindir |= 0x08;

if(nindir) {
	*(*act_pro)++=INDIR;
	*(*act_pro)++=nindir;
}

*(*act_pro)++=codigo;
*(*act_pro)++=par1;
*(*act_pro)++=par2;
*(*act_pro)++=par3;
*(*act_pro)++=par4;

return(_E_NERR);
}

/****************************************************************************
	COMPILA_PAR7: compila condactos con 7 par metros.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'codigo' c¢digo num‚rico del condacto
			'lim1' l¡mite para el par metro 1 (NOTA: meter
			255 para saltar comprobaci¢n)
			'lim2' l¡mite para el par metro 2 (NOTA: meter
			255 para saltar comprobaci¢n)
			'coderr1' c¢digo de error en caso de que el par metro
			1 sea mayor que 'lim1'
			'coderr2' c¢digo de error en caso de que el par metro
			2 sea mayor que 'lim2'
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR compila_par7(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE codigo,
  BYTE lim1, BYTE lim2, COD_ERR coderr1, COD_ERR coderr2)
{
BYTE par1, par2, par3, par4, par5, par6, par7, nindir=0;
COD_ERR indir;

/* comprueba si cabe en memoria */
if(*act_pro+9>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par1);
/* si hubo error al coger el par metro 1, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim1' */
if((indir!=_E_CIND) && (par1>lim1)) return(coderr1);
if(indir==_E_CIND) nindir |= 0x01;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par2);
/* si hubo error al coger el par metro 2, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
/* comprueba si es mayor que 'lim2' */
if((indir!=_E_CIND) && (par2>lim2)) return(coderr2);
if(indir==_E_CIND) nindir |= 0x02;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par3);
/* si hubo error al coger el par metro 3, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
if(indir==_E_CIND) nindir |= 0x04;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par4);
/* si hubo error al coger el par metro 4, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
if(indir==_E_CIND) nindir |= 0x08;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par5);
/* si hubo error al coger el par metro 5, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
if(indir==_E_CIND) nindir |= 0x10;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par6);
/* si hubo error al coger el par metro 6, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
if(indir==_E_CIND) nindir |= 0x20;

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par7);
/* si hubo error al coger el par metro 7, sale */
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
if(indir==_E_CIND) nindir |= 0x40;

if(nindir) {
	*(*act_pro)++=INDIR;
	*(*act_pro)++=nindir;
}

*(*act_pro)++=codigo;
*(*act_pro)++=par1;
*(*act_pro)++=par2;
*(*act_pro)++=par3;
*(*act_pro)++=par4;
*(*act_pro)++=par5;
*(*act_pro)++=par6;
*(*act_pro)++=par7;

return(_E_NERR);
}

/****************************************************************************
	PROCESS: compila condacto PROCESS.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'condacto' n£mero de condacto
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR process(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE condacto)
{
BYTE par1;
COD_ERR indir;

/* comprueba si cabe en memoria */
if(*act_pro+3>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);
indir=coge_num(*l,&par1);
if((indir!=_E_NERR) && (indir!=_E_CIND)) return(indir);
if((indir!=_E_CIND) && (par1==num_pro_act)) return(_E_PRPR);
if(indir==_E_CIND) {
	*(*act_pro)++=INDIR;
	*(*act_pro)++=0x01;
}
*(*act_pro)++=condacto;
*(*act_pro)++=par1;

/* si se llama a un proceso no definido guardar  informaci¢n sobre esa */
/* llamada en una tabla para resolverla al final de la compilaci¢n */
if((indir!=_E_CIND) && (par1>num_pro_act)) {
	/* comprueba si rebosa la tabla, si no guarda el n£mero del */
	/* proceso al que se llama y el n£mero de l¡nea en el archivo */
	/* de entrada */
	if(pprc>=FPROCESS) return(_E_PRRB);
	prc[pprc].numpro=par1;
	prc[pprc].nl=numlin_fich_entr;
	pprc++;
}

return(_E_NERR);
}

/****************************************************************************
	SKIP: compila condacto SKIP.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'condacto' n£mero del condacto
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR skip(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE condacto)
{
char etq[LONGETQ+1];
int i, despli;
unsigned desplu;

/* comprueba si cabe en memoria */
if(*act_pro+4>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);

/* comprueba si lo que sigue es una etiqueta */
/* para lo cual debe empezar con el car cter MARCA_ETQ */
if(*(*l)!=MARCA_ETQ) return(_E_NLAB);
(*l)++;

/* recoge la etiqueta */
if(coge_nombre_etq(l,etq,LONGETQ+1)) return(_E_ENVL);

/* si no hay ninguna etiqueta definida, la mete en tabla sin m s */
if(!pt_etiq) {
	mete_fsk(etq,*act_pro+1);
	*(*act_pro)++=32;
	/* inicializa a 0, pero luego ser  sustituido */
	/* por su desplazamiento correspondiente */
	*(*act_pro)++=0;
	*(*act_pro)++=0;
}
else {
	/* mira a ver si la etiqueta ya existe */
	for(i=0; i<pt_etiq; i++) {
		if(!strcmp(etq,etiqueta[i].etq)) break;
	}
	/* si la etiqueta no existe la guarda */
	if(i==pt_etiq) {
		if(mete_fsk(etq,*act_pro+1)) return(_E_RBTS);
		*(*act_pro)++=condacto;
		/* inicializa a 0, pero luego ser  sustituido */
		/* por su desplazamiento correspondiente */
		*(*act_pro)++=0;
		*(*act_pro)++=0;
	}
	else {
		*(*act_pro)++=condacto;
		/* si la etiqueta fue definida, recupera su valor */
		desplu=(unsigned)(*act_pro-1-etiqueta[i].petq);
		if(desplu>0x8000) return(_E_LBFR);
		despli=-desplu;
		/* guarda byte bajo y byte alto */
		*(*act_pro)++=(BYTE)(despli & 0x00ff);
		*(*act_pro)++=(BYTE)((despli >> 8) & 0x00ff);
	}
}

return(_E_NERR);
}

/****************************************************************************
	SYNONYM: compila condacto SYNONYM.
	  Entrada:      'l' puntero al puntero a la l¡nea con el par metro
			del condacto
			'act_pro' puntero al puntero a donde dejar condacto
			compilado
			'ult_pro' puntero a £ltimo byte de buffer de procesos
			'condacto' n£mero del condacto
			'pal_voc' n£mero de palabras en vocabulario
			'n_conv' m ximo n£mero de nombre convertible
	  Salida:       'l' y 'act_pro' actualizados.
			c¢digo de error
****************************************************************************/
COD_ERR synonym(char *(*l), BYTE *(*act_pro), BYTE *ult_pro, BYTE condacto,
  int pal_voc, BYTE n_conv)
{
BYTE verb, nomb;
char pal[LONGPAL+1];
int i, errct;

/* comprueba si cabe en memoria */
if(*act_pro+4>=ult_pro) return(_E_FMEM);

if(sgte_campo(l)) return(_E_FCAM);

/* recoge campo verbo */
errct=coge_pal(*l,pal);

/* comprueba si es palabra nula (CHR_NOPAL) */
/* si lo es mete NO_PAL */
if(*pal==CHR_NOPAL) verb=NO_PAL;
else {
	/* si hubo error al coger palabra, sale */
	if(errct) return(errct);

	/* comprueba si est  en vocabulario */
	if((i=esta_en_voc(vocabulario,pal_voc,pal))==NUM_PAL+1)
	  return(_E_NPVC);

	/* si no es verbo ni nombre convertible, error */
	if((vocabulario[i].tipo!=_VERB) &&
	  ((vocabulario[i].tipo==_NOMB) && (vocabulario[i].num>=n_conv)))
	  return(_E_NVNC);

	verb=vocabulario[i].num;
}
if(sgte_campo(l)) return(_E_FCAM);

/* recoge campo nombre */
errct=coge_pal(*l,pal);

/* comprueba si es palabra nula (CHR_NOPAL) */
/* si lo es mete NO_PAL */
if(*pal==CHR_NOPAL) nomb=NO_PAL;
else {
	/* si hubo error al coger palabra, sale */
	if(errct==_E_CVOC) return(errct);

	/* comprueba si est  en vocabulario */
	if((i=esta_en_voc(vocabulario,pal_voc,pal))==NUM_PAL+1)
	  return(_E_NPVC);

	/* si no es nombre, error */
	if(vocabulario[i].tipo!=_NOMB) return(_E_NNNN);

	nomb=vocabulario[i].num;
}

*(*act_pro)++=condacto;
*(*act_pro)++=verb;
*(*act_pro)++=nomb;

return(_E_NERR);
}

