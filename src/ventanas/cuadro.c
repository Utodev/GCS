/****************************************************************************
				 CUADRO.C

	Biblioteca de funciones para gestionar cuadros de di logo.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- c_crea_cuadro: crea e inicializa un cuadro de di logo
		- c_crea_elemento: crea o modifica un elemento de un cuadro
		- c_dibuja_elemento: dibuja un elemento de un cuadro
		- c_abre: abre un cuadro de di logo
		- c_cierra: cierra un cuadro de di logo
		- c_elimina: elimina de memoria un cuadro y sus elementos
		- c_cambia_color: modifica los colores de un cuadro
		- c_lee_input: permite la introducci¢n de una cadena de
		    caracteres
		- c_accion: env¡a una acci¢n a un cuadro de di logo
		- c_gestiona: gestiona un cuadro de di logo
		- c_elem_input: gestiona un elemento de entrada
		- c_elem_lista: gestiona un elemento de lista
		- c_mete_en_lista: inserta una cadena al final de una lista
		- c_borra_lista: elimina una lista y libera memoria
		- c_elem_check: gestiona un elemento caja de comprobaci¢n
		- c_selecc_ficheros: gestiona cuadro de selecci¢n de ficheros

	Las siguientes estructuras est n definidas en CUADRO.H:
		STC_ELEM: elemento de un cuadro de di logo
		STC_CUADRO: cuadro de di logo
		STC_ACCION_C: acci¢n para un cuadro de di logo
		STC_ELEM_BOTON: elemento bot¢n
		STC_ELEM_INPUT: elemento de entrada
		STC_ELEM_LISTA: elemento de lista
		STC_ELEM_CHECK: elemento de caja de comprobaci¢n
		STC_ELEM_TEXTO: elemento de texto
****************************************************************************/

#include <stdlib.h>
#include <stddef.h>
#include <bios.h>
#include <time.h>
#include <alloc.h>
#include <string.h>
#include <stdarg.h>
#include <dos.h>
#include <dir.h>
#include "ventana.h"
#include "raton.h"
#include "cuadro.h"

/*** Variables globales internas ***/
static char alt_tecla=0;

/*** Prototipos de funciones internas ***/
static int int24_hnd(int errval, int ax, int bp, int si);
static char mayuscula(char c);
static void may_str(char *s);
static void coge_pos_cursor(int *fil, int *col);
static void pon_cursor(int fil, int col);
static void imprime_txt_elem(int fil, int col, char *txt, int lng,
  BYTE clr, BYTE clrt, int modo);
static void resalta_elemento(STC_CUADRO *cuad);
static void siguiente_elemento(STC_CUADRO *c);
static void anterior_elemento(STC_CUADRO *c);
static void imprime_lista(STC_VENTANA *v, STC_LISTA *l, int elemento,
  int elemento_sel, BYTE clr_sel);
static void busca_elem_selecc(STC_ELEM_LISTA *e);
static void dibuja_flechas_vert(STC_VENTANA *v, int nelem, int elem);
static int lista_ficheros(STC_ELEM_LISTA *lista, char *ruta_masc);
static void pon_barra_dir(char *dir);
static char scan_ascii(int tecla);
static int busca_elemento_tecla(STC_CUADRO *c, char tecla);
static int busca_elemento_raton(STC_CUADRO *c, int fil, int col);
static void inicializa_elem_boton(STC_ELEM_BOTON *e, int ancho, char *texto);
static void inicializa_elem_input(STC_ELEM_INPUT *e, int ancho, char *texto,
  char *buff, int longitud);
static void inicializa_elem_lista(STC_ELEM_LISTA *e, int fil, int col,
  int ancho, int alto, char *texto, BYTE clr_princ, BYTE clr_s1,
  BYTE clr_s2, int modo, int orden);
static void inicializa_elem_check(STC_ELEM_CHECK *e, char *texto);
static void inicializa_elem_texto(STC_ELEM_TEXTO *e, char *texto, int fil,
  int col, int ancho, int alto, BYTE clr_princ, BYTE clr_s1, BYTE clr_s2,
  int modo, int borde);

/*** Variables globales ***/
char *c_Borde_Sel="ÉÍ»ººÈÍ¼";
char *c_Esp="\x18\x19±þ[]\x07X ";

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
	MAY_STR: convierte una cadena en may£scula.
	  Entrada:      's' puntero a cadena
****************************************************************************/
void may_str(char *s)
{

while(*s) {
	*s=mayuscula(*s);
	s++;
}

}

/****************************************************************************
	COGE_POS_CURSOR: devuelve la posici¢n del cursor.
	  Entrada:      'fil', 'col' punteros a variables donde se
			devolver  la posici¢n del cursor
	  Salida:       posici¢n del cursor en las variables a las que
			apuntan 'fil' y 'col'
****************************************************************************/
void coge_pos_cursor(int *fil, int *col)
{
int fila=0, columna=0;

asm {
	mov ah,03h              // funci¢n buscar posici¢n del cursor
	mov bh,0                // supone p gina 0
	int 10h
	mov byte ptr fila,dh
	mov byte ptr columna,dl
}

*fil=fila;
*col=columna;

}

/****************************************************************************
	PON_CURSOR: coloca el cursor en una posici¢n de pantalla.
	  Entrada:      'fil', 'col' fila y columna del cursor
****************************************************************************/
void pon_cursor(int fil, int col)
{

asm {
	mov ah,02h              // funci¢n definir posici¢n del cursor
	mov bh,0                // supone p gina 0
	mov dh,byte ptr fil     // DH = fila del cursor
	mov dl,byte ptr col     // DL = columna del cursor
	int 10h
}

}

/****************************************************************************
	IMPRIME_TXT_ELEM: imprime el texto de un elemento, resaltando la
	  tecla asociada.
	  Entrada:      'fil', 'col' posici¢n del texto
			'txt' texto a imprimir
			'lng' longitud a imprimir
			'clr' color del texto
			'clrt' color para tecla de activaci¢n
			'modo' modo de impresi¢n; 0 no se tiene en cuenta
			C_CARTECLA, 1 si
****************************************************************************/
void imprime_txt_elem(int fil, int col, char *txt, int lng, BYTE clr,
  BYTE clrt, int modo)
{

r_puntero(R_OCULTA);

while(*txt && (lng>0)) {
	/* si es tecla asociada, la imprime en otro color */
	if((*txt==C_CARTECLA) && modo) {
		txt++;
		lng--;
		v_impcar(fil,col,*txt,clrt);
	}
	else v_impcar(fil,col,*txt,clr);

	txt++;
	col++;
	lng--;
}

/* rellena con espacios hasta el final */
for(; lng>0; lng--, col++) v_impcar(fil,col,' ',clr);

r_puntero(R_MUESTRA);

}

/****************************************************************************
	RESALTA_ELEMENTO: resalta el elemento actual de un cuadro de
	  di logo.
	  Entrada:      'cuad' puntero a estructura de cuadro
****************************************************************************/
void resalta_elemento(STC_CUADRO *cuad)
{
STC_ELEM *elem;

for(elem=cuad->elem1; elem!=NULL; elem=elem->sgte) c_dibuja_elemento(cuad,
  elem);

}

/****************************************************************************
	SIGUIENTE_ELEMENTO: selecciona el siguiente elemento v lido de un
	  cuadro de di logo.
	  Entrada:      'c' puntero a estructura de cuadro
****************************************************************************/
void siguiente_elemento(STC_CUADRO *c)
{
STC_ELEM *elem;

elem=c->elemento;

if(elem->sgte==NULL) {
	c->elemento=c->elem1;
	return;
}

do {
	elem=elem->sgte;
	if(elem==NULL) elem=c->elem1;
} while((elem->tipo==C_ELEM_NULO) || (elem->tipo==C_ELEM_TEXTO));

c->elemento=elem;

}

/****************************************************************************
	ANTERIOR_ELEMENTO: selecciona el elemento anterior v lido de un
	  cuadro de di logo.
	  Entrada:      'cuad' puntero a estructura de cuadro
****************************************************************************/
void anterior_elemento(STC_CUADRO *c)
{
STC_ELEM *elem;

elem=c->elemento;

if(elem->ant==NULL) {
	while(elem->sgte!=NULL) elem=elem->sgte;
	c->elemento=elem;
	return;
}

do {
	elem=elem->ant;
	if(elem==NULL) while(elem->sgte!=NULL) elem=elem->sgte;
} while((elem->tipo==C_ELEM_NULO) || (elem->tipo==C_ELEM_TEXTO));

c->elemento=elem;

}

/****************************************************************************
	IMPRIME_LISTA: imprime una lista de cadenas de un elemento de
	  lista.
	  Entrada:      'v' ventana donde se imprimir  la lista
			'l' puntero a primer elemento de la lista
			'elemento' n£mero de orden del primer elemento a
			imprimir
			'elemento_sel' n£mero de orden del elemento
			seleccionado
			'clr_sel' color del elemento seleccionado
****************************************************************************/
void imprime_lista(STC_VENTANA *v, STC_LISTA *l, int elemento,
  int elemento_sel, BYTE clr_sel)
{
STC_LISTA *elem;
int i, j;

v_pon_cursor(v,0,0);
r_puntero(R_OCULTA);

/* si lista vac¡a, imprime ventana vac¡a */
if(l==NULL) {
	for(j=0; j<(v->alto-2); j++) {
		v_impcad(v,"",V_RELLENA);
		v->colc=0;
		v->filc++;
	}
	r_puntero(R_MUESTRA);
	return;
}

/* posiciona el puntero en el primer elemento a imprimir */
elem=l;
for(i=0; i<elemento; i++) {
	if(elem->sgte==NULL) break;
	else elem=elem->sgte;
}

/* imprime hasta el £ltimo elemento o hasta que se llene la ventana */
j=0;
while((elem!=NULL) && (j<(v->alto-2))) {
	/* si es elemento seleccionado lo imprime resaltado */
	if((i+j)==elemento_sel) {
		v_color(v,clr_sel);
		v_impcad(v,elem->cadena,V_RELLENA);
		v_color(v,v->clr_princ);
	}
	else v_impcad(v,elem->cadena,V_RELLENA);
	v->colc=0;
	v->filc++;
	elem=elem->sgte;
	j++;
}

/* rellena resto de la ventana con l¡neas en blanco */
for(; j<(v->alto-2); j++) {
	v_impcad(v,"",V_RELLENA);
	v->colc=0;
	v->filc++;
}

r_puntero(R_MUESTRA);

}

/****************************************************************************
	BUSCA_ELEM_SELECC: busca elemento seleccionado de una lista.
	  Entrada:      'e' puntero a lista
****************************************************************************/
void busca_elem_selecc(STC_ELEM_LISTA *e)
{
STC_LISTA *elem;
int i;

elem=e->elemento;
for(i=0; i<e->elemento_sel; i++) elem=elem->sgte;
e->selecc=elem->cadena;

}

/****************************************************************************
	DIBUJA_FLECHAS_VERT: dibuja flechas de direcci¢n verticales en
	  en borde derecho de una ventana.
	  Entrada:      'v' puntero a ventana
			'nelem' n£mero de elementos
			'elem' elemento seleccionado
****************************************************************************/
void dibuja_flechas_vert(STC_VENTANA *v, int nelem, int elem)
{
int i, col, lng, pos_v;

r_puntero(R_OCULTA);

col=v->col+v->ancho-1;
v_impcar(v->fil+1,col,c_Esp[0],v->clr_s2);
v_impcar(v->fil+v->alto-2,col,c_Esp[1],v->clr_s2);

for(i=v->fil+2; i<(v->fil+v->alto-2); i++) {
	v_impcar(i,col,c_Esp[2],v->clr_s2);
}

lng=v->alto-4;
if(nelem==1) pos_v=0;
else pos_v=(int)(((unsigned long)(lng-1)*elem)/(nelem-1));
pos_v+=v->fil+2;
v_impcar(pos_v,col,c_Esp[3],v->clr_s2);

r_puntero(R_MUESTRA);

}

/****************************************************************************
	LISTA_FICHEROS: crea una lista con los ficheros y subdirectorios de
	  un directorio; si la lista estaba llena, la vac¡a.
	  Entrada:      'lista' puntero a elemento de lista
			'ruta_masc' ruta al directorio + m scara de
			b£squeda de ficheros
	  Salida:       1 si pudo crear la lista, 0 si no pudo crear lista
			completa
****************************************************************************/
int lista_ficheros(STC_ELEM_LISTA *lista, char *ruta_masc)
{
void interrupt (*int24_old)();
struct ffblk find;
char disq[MAXDRIVE], direct[MAXDIR], fich[MAXFILE], ext[MAXEXT],
  nfich[MAXPATH], dir[15];
int i, encontrado;

/* instala 'handler' de errores cr¡ticos */
int24_old=getvect(0x24);
harderr(int24_hnd);

/* separa ruta de m scara */
fnsplit(ruta_masc,disq,direct,fich,ext);

/* vac¡a la lista */
c_borra_lista(lista);

/* crea ruta + "*.*" */
strcpy(nfich,disq);
strcat(nfich,direct);
strcat(nfich,"*.*");

/* busca primero subdirectorios */
encontrado=findfirst(nfich,&find,FA_DIREC);

while(!encontrado) {
	if(find.ff_attrib & FA_DIREC) {
		/* a¤ade caracteres indicadores de directorio */
		dir[0]=C_CHRDIR1;
		strcpy(&dir[1],find.ff_name);
		i=strlen(dir);
		dir[i]=C_CHRDIR2;
		dir[i+1]='\0';

		/* mete nombre de subdirectorio en lista */
		if(c_mete_en_lista(lista,dir)==0) {
			/* restaura 'handler' de errores cr¡ticos */
			setvect(0x24,int24_old);
			return(0);
		}
	}

	/* busca siguiente */
	encontrado=findnext(&find);
}

/* ahora busca ficheros */
encontrado=findfirst(ruta_masc,&find,0);

while(!encontrado) {
	/* mete nombre de fichero en lista */
	if(c_mete_en_lista(lista,find.ff_name)==0) {
		/* restaura 'handler' de errores cr¡ticos */
		setvect(0x24,int24_old);
		return(0);
	}

	/* busca siguiente */
	encontrado=findnext(&find);
}

/* restaura 'handler' de errores cr¡ticos */
setvect(0x24,int24_old);

return(1);
}

/****************************************************************************
	PON_BARRA_DIR: a¤ade barra directorio ('\') al final de una cadena
	  si no la tiene. La cadena debe tener espacio para a¤adir el
	  car cter.
	  Entrada:      'dir' puntero a cadena
****************************************************************************/
void pon_barra_dir(char *dir)
{
int i;

i=strlen(dir);
if(dir[i-1]!='\\') strcat(dir,"\\");

}

/****************************************************************************
	SCAN_ASCII: convierte c¢digo 'scan' de una tecla en c¢digo ASCII;
	  s¢lo funciona con teclas alfab‚ticas.
	  Entrada:	'scan' c¢digo de la tecla pulsada
	  Salida:	c¢digo ASCII correspondiente
****************************************************************************/
char scan_ascii(int tecla)
{
char scasc[35]="QWERTYUIOP\0\0\0\0ASDFGHJKL\0\0\0\0\0ZXCVBNM";
int scan;

scan=((tecla >> 8) & 0x00ff)-16;
if((scan>0) && (scan<35)) return(scasc[scan]);
else return(0);

}

/****************************************************************************
	BUSCA_ELEMENTO_TECLA: busca y selecciona un elemento de un
	  cuadro seg£n su tecla asociada.
	  Entrada:	'c' puntero a estructura del cuadro
			'tecla' c¢digo ASCII de tecla
	  Salida:	n£mero elemento seleccionado, -1 si ninguno
****************************************************************************/
int busca_elemento_tecla(STC_CUADRO *c, char tecla)
{
STC_ELEM *elem;
int i;
char tecla_elem;

if(!tecla) return(-1);

for(elem=c->elem1, i=0; elem!=NULL; elem=elem->sgte, i++) {
	if((elem->tipo!=C_ELEM_NULO) && (elem->tipo!=C_ELEM_TEXTO)) {
		switch(elem->tipo) {
		    case C_ELEM_BOTON :
			tecla_elem=((STC_ELEM_BOTON *)elem->info)->tecla;
			break;
		    case C_ELEM_INPUT :
			tecla_elem=((STC_ELEM_INPUT *)elem->info)->tecla;
			break;
		    case C_ELEM_LISTA :
			tecla_elem=((STC_ELEM_LISTA *)elem->info)->tecla;
			break;
		    case C_ELEM_CHECK :
			tecla_elem=((STC_ELEM_CHECK *)elem->info)->tecla;
			break;
		}

		if(tecla==tecla_elem) {
			c->elemento=elem;
			resalta_elemento(c);
			return(i);
		}
	}
}

return(-1);
}

/****************************************************************************
	BUSCA_ELEMENTO_RATON: busca y selecciona un elemento de un
	  cuadro seg£n su posici¢n.
	  Entrada:	'c' puntero a estructura del cuadro
			'fil', 'col' posici¢n del puntero del rat¢n
	  Salida:	n£mero de elemento seleccionado, -1 si ninguno
****************************************************************************/
int busca_elemento_raton(STC_CUADRO *c, int fil, int col)
{
STC_ELEM *elem;
int i, fil0, col0, fil1, col1;

for(elem=c->elem1, i=0; elem!=NULL; elem=elem->sgte, i++) {
	if((elem->tipo!=C_ELEM_NULO) && (elem->tipo!=C_ELEM_TEXTO)) {
		switch(elem->tipo) {
		    case C_ELEM_BOTON :
			fil0=elem->fil+c->v.fil+1;
			col0=elem->col+c->v.col+1;
			fil1=fil0;
			col1=col0+((STC_ELEM_BOTON *)elem->info)->ancho-1;
			break;
		    case C_ELEM_INPUT :
			fil0=elem->fil+c->v.fil+1;
			col0=elem->col+c->v.col+1;
			fil1=fil0;
			col1=col0+((STC_ELEM_INPUT *)elem->info)->ancho-1;
			break;
		    case C_ELEM_LISTA :
			fil0=((STC_ELEM_LISTA *)elem->info)->v.fil;
			col0=((STC_ELEM_LISTA *)elem->info)->v.col;
			fil1=fil0+((STC_ELEM_LISTA *)elem->info)->v.alto-1;
			col1=col0+((STC_ELEM_LISTA *)elem->info)->v.ancho-1;
			break;
		    case C_ELEM_CHECK :
			fil0=elem->fil+c->v.fil+1;
			col0=elem->col+c->v.col+1;
			fil1=fil0;
			col1=col0+2;
			break;
		}

		if((fil>=fil0) && (fil<=fil1) && (col>=col0) && (col<=col1)) {
			if(c->elemento!=elem) {
				c->elemento=elem;
				resalta_elemento(c);
			}
			return(i);
		}
	}
}

return(-1);
}

/****************************************************************************
	INICIALIZA_ELEM_BOTON: inicializa un elemento de tipo bot¢n.
	  Entrada:      'e' puntero a informaci¢n de elemento
			'ancho' anchura del bot¢n
			'texto' texto del bot¢n
****************************************************************************/
void inicializa_elem_boton(STC_ELEM_BOTON *e, int ancho, char *texto)
{
char *txt;

e->ancho=ancho;
e->texto=texto;

e->tecla=0;
for(txt=texto; *txt; txt++) {
	if(*txt==C_CARTECLA) {
		e->tecla=mayuscula(*(txt+1));
		break;
	}
}

}

/****************************************************************************
	INICIALIZA_ELEM_INPUT: inicializa un elemento de entrada.
	  Entrada:      'e' puntero a informaci¢n de elemento
			'ancho' anchura de zona de 'input'
			'texto' texto
			'buff' puntero a buffer de 'input'
			'longitud' m x. longitud de cadena sin contar '\0'
****************************************************************************/
void inicializa_elem_input(STC_ELEM_INPUT *e, int ancho, char *texto,
  char *buff, int longitud)
{
char *txt;

e->ancho=ancho;
e->texto=texto;
e->cadena=buff;
e->longitud=longitud;

e->tecla=0;
for(txt=texto; *txt; txt++) {
	if(*txt==C_CARTECLA) {
		e->tecla=mayuscula(*(txt+1));
		break;
	}
}

}

/****************************************************************************
	INICIALIZA_ELEM_LISTA: inicializa un elemento de tipo lista.
	  Entrada:      'e' puntero a informaci¢n de elemento
			'fil', 'col' posici¢n de la lista
			'ancho', 'alto' dimensiones de la lista
 			'texto' texto de encabezamiento
			'clr_princ' color principal
			'clr_s1', 'clr_s2' colores de sombra
			'modo' C_LSTNORMAL si normal, C_LSTPRIMERO si
			elemento seleccionado siempre aparece primero
			'orden' C_LSTSINORDEN si lista desordenada,
			C_LSTORDENADA si lista ordenada
****************************************************************************/
void inicializa_elem_lista(STC_ELEM_LISTA *e, int fil, int col,
  int ancho, int alto, char *texto, BYTE clr_princ, BYTE clr_s1,
  BYTE clr_s2, int modo, int orden)
{
char *txt;

v_crea(&e->v,fil,col,ancho,alto,clr_princ,clr_s1,clr_s2,NULL,0);

e->texto=texto;
e->modo=modo;
e->orden=orden;
e->elemento=NULL;
e->num_elementos=0;
e->elemento_pr=0;
e->selecc=NULL;
e->elemento_sel=0;

e->tecla=0;
for(txt=texto; *txt; txt++) {
	if(*txt==C_CARTECLA) {
		e->tecla=mayuscula(*(txt+1));
		break;
	}
}

}

/****************************************************************************
	INICIALIZA_ELEM_CHECK: inicializa un elemento de caja de
	  comprobaci¢n.
	  Entrada:      'e' puntero a informaci¢n de elemento
			'texto' texto
****************************************************************************/
void inicializa_elem_check(STC_ELEM_CHECK *e, char *texto)
{
char *txt;

e->texto=texto;
e->estado=0;

e->tecla=0;
for(txt=texto; *txt; txt++) {
	if(*txt==C_CARTECLA) {
		e->tecla=mayuscula(*(txt+1));
		break;
	}
}

}

/****************************************************************************
	INICIALIZA_ELEM_TEXTO: inicializa un elemento de texto.
	  Entrada:      'e' puntero a informaci¢n de elemento
			'texto' texto
			'fil', 'col' posici¢n de la ventana
			'ancho', 'alto' dimensiones de la ventana
			'clr_princ' color principal
			'clr_s1', 'clr_s2' colores de sombra
			'modo' C_TXTLINEA si imprime cada l¡nea por separado
			C_TXTPASA si pasa texto a siguiente l¡nea si no cabe
			'borde' C_TXTNOBORDE si no imprime borde,
			C_TXTBORDE si lo imprime
****************************************************************************/
void inicializa_elem_texto(STC_ELEM_TEXTO *e, char *texto, int fil,
  int col, int ancho, int alto, BYTE clr_princ, BYTE clr_s1, BYTE clr_s2,
  int modo, int borde)
{

v_crea(&e->v,fil,col,ancho,alto,clr_princ,clr_s1,clr_s2,NULL,0);

e->texto=texto;
e->modo=modo;
e->borde=borde;

if(modo!=C_TXTLINEA) v_modo_texto(&e->v,V_PASA_LINEA);

}

/****************************************************************************
	C_CREA_CUADRO: crea un cuadro de di logo.
	  Entrada:      'cuad' puntero a estructura del cuadro a crear
			'titulo' texto de encabezamiento (NULL si ninguno)
			'fil', 'col' posici¢n del cuadro; si 'fil' es igual
			a C_CENT se centra el cuadro verticalmente, si 'col'
			es igual a C_CENT se centra horizontalmente
			'ancho', 'alto' tama¤o del cuadro
			'clr_princ' color principal del cuadro
			'clr_s1', 'clr_s2' colores para sombra
			'clr_boton' color para elemento bot¢n
			'clr_input' color para elemento input
			'clr_sel' color para elemento seleccionado en listas
			'clr_tecla' color para teclas de activaci¢n
			'clr_tecla_boton' color para tecla activaci¢n bot¢n
****************************************************************************/
void c_crea_cuadro(STC_CUADRO *cuad, char *titulo, int fil, int col,
  int ancho, int alto, BYTE clr_princ, BYTE clr_s1, BYTE clr_s2,
  BYTE clr_boton, BYTE clr_input, BYTE clr_sel, BYTE clr_tecla,
  BYTE clr_tecla_boton)
{

/* comprueba si hay que centrar el cuadro */
if(fil==C_CENT) fil=(25-alto)/2;
if(col==C_CENT) col=(80-ancho)/2;

v_crea(&cuad->v,fil,col,ancho,alto,clr_princ,clr_s1,clr_s2,titulo,1);

cuad->clr_boton=clr_boton;
cuad->clr_input=clr_input;
cuad->clr_sel=clr_sel;
cuad->clr_tecla=clr_tecla;
cuad->clr_tecla_boton=clr_tecla_boton;

/* inicializa elementos del cuadro */
cuad->elem1=NULL;
cuad->elemento=NULL;

}

/****************************************************************************
	C_CREA_ELEMENTO: crea un elemento de un cuadro.
	  Entrada:      'cuad' puntero a estructura del cuadro
			'tipo' tipo de elemento
			'fil', 'col' posici¢n del elemento dentro del cuadro
			... otros par metros dependiendo del elemento
			a crear:
			  BOTON: 'texto' 	(char *)
 				 'ancho'	(int)
			  INPUT: 'texto' 	(char *)
 				 'ancho' 	(int)
 				 'buff'  	(char *)
 				 'longitud'	(int)
			  LISTA: 'texto'        (char *)
 				 'ancho'        (int)
 				 'alto'         (int)
				 'modo'         (int)
 				 'orden'        (int)
			  CHECK: 'texto'        (char *)
			  TEXTO: 'texto'        (char *)
				 'ancho'        (int)
				 'alto'		(int)
 				 'modo'         (int)
 				 'borde'        (int)

	  Salida:       puntero a elemento creado, NULL si no pudo
****************************************************************************/
STC_ELEM *c_crea_elemento(STC_CUADRO *cuad, int tipo, int fil, int col, ...)
{
STC_ELEM *e, *elem;
va_list par;
void *info;
char *texto, *buff;
int fil_elem, col_elem, ancho, alto, longitud, modo, orden, borde;

va_start(par,col);

/* calcula posici¢n del elemento en pantalla */
fil_elem=cuad->v.fil+fil+1;
col_elem=cuad->v.col+col+1;

e=(STC_ELEM *)malloc(sizeof(STC_ELEM));
if(e==NULL) return(NULL);

e->tipo=tipo;
e->fil=fil;
e->col=col;

/* crea elemento seg£n tipo */
switch(tipo) {
	case C_ELEM_BOTON :
		info=(STC_ELEM_BOTON *)malloc(sizeof(STC_ELEM_BOTON));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		ancho=va_arg(par,int);
		inicializa_elem_boton((STC_ELEM_BOTON *)info,ancho,texto);
		break;
	case C_ELEM_INPUT :
		info=(STC_ELEM_INPUT *)malloc(sizeof(STC_ELEM_INPUT));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		ancho=va_arg(par,int);
		buff=va_arg(par,char *);
		longitud=va_arg(par,int);
		inicializa_elem_input((STC_ELEM_INPUT *)info,ancho,texto,buff,
		  longitud);
		break;
	case C_ELEM_LISTA :
		info=(STC_ELEM_LISTA *)malloc(sizeof(STC_ELEM_LISTA));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		ancho=va_arg(par,int);
		alto=va_arg(par,int);
		modo=va_arg(par,int);
		orden=va_arg(par,int);
		inicializa_elem_lista((STC_ELEM_LISTA *)info,fil_elem,col_elem,
		  ancho,alto,texto,cuad->v.clr_princ,cuad->v.clr_s2,
		  cuad->v.clr_s1,modo,orden);
		break;
	case C_ELEM_CHECK :
		info=(STC_ELEM_CHECK *)malloc(sizeof(STC_ELEM_CHECK));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		inicializa_elem_check((STC_ELEM_CHECK *)info,texto);
		break;
	case C_ELEM_TEXTO :
		info=(STC_ELEM_TEXTO *)malloc(sizeof(STC_ELEM_TEXTO));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		ancho=va_arg(par,int);
		alto=va_arg(par,int);
		modo=va_arg(par,int);
		borde=va_arg(par,int);
		inicializa_elem_texto((STC_ELEM_TEXTO *)info,texto,fil_elem,
		  col_elem,ancho,alto,cuad->v.clr_princ,cuad->v.clr_s2,
		  cuad->v.clr_s1,modo,borde);
		break;
}

va_end(par);

if(info==NULL) {
	free(e);
	return(NULL);
}

/* si es primer elemento */
if(cuad->elem1==NULL) {
	e->ant=NULL;
	e->sgte=NULL;
	cuad->elem1=e;
}
else {
	for(elem=cuad->elem1; elem->sgte!=NULL; elem=elem->sgte);
	e->ant=elem;
	e->sgte=NULL;
	elem->sgte=e;
}

return(e);
}

/****************************************************************************
	C_DIBUJA_ELEMENTO: dibuja un elemento de un cuadro.
	  Entrada:      'cuad' puntero al cuadro al que pertenece el elemento
 			'e' puntero al elemento
	  Salida:       1 si pudo dibujarlo, 0 si no
****************************************************************************/
void c_dibuja_elemento(STC_CUADRO *cuad, STC_ELEM *e)
{
STC_ELEM_BOTON *boton;
STC_ELEM_INPUT *input;
STC_ELEM_LISTA *lista;
STC_ELEM_CHECK *check;
STC_ELEM_TEXTO *texto;
BYTE clr_sel;
char *txt, c1, c2;
int i, seleccionado, fil, col;

r_puntero(R_OCULTA);

/* posici¢n del elemento en pantalla */
fil=e->fil+cuad->v.fil+1;
col=e->col+cuad->v.col+1;

/* comprueba si es el elemento seleccionado */
if(e==cuad->elemento) seleccionado=1;
else seleccionado=0;

switch(e->tipo) {
	case C_ELEM_NULO :
		break;
	case C_ELEM_BOTON :
		boton=(STC_ELEM_BOTON *)e->info;
		txt=boton->texto;
		if(!seleccionado) {
			c1=CBOTON1;
			c2=CBOTON2;
		}
		else {
			c1=CBOTON1_ON;
			c2=CBOTON2_ON;
		}
		v_impcar(fil,col,c1,cuad->clr_boton);
		imprime_txt_elem(fil,col+1,"",boton->ancho-2,cuad->clr_boton,
		  cuad->clr_boton,0);
		imprime_txt_elem(fil,col+((boton->ancho-strlen(txt)+1)/2),txt,
		  strlen(txt),cuad->clr_boton,cuad->clr_tecla_boton,1);
		v_impcar(fil,col+boton->ancho-1,c2,cuad->clr_boton);
		/* sombra del bot¢n */
		for(i=0; i<boton->ancho; i++) v_impcar(fil+1,col+i+1,CBOTONS1,
		  cuad->v.clr_princ);
		v_impcar(fil,col+boton->ancho,CBOTONS2,cuad->v.clr_princ);
		break;
	case C_ELEM_INPUT :
		input=(STC_ELEM_INPUT *)e->info;
		txt=input->texto;
		imprime_txt_elem(fil,col-strlen(txt),txt,strlen(txt),
		  cuad->v.clr_princ,cuad->clr_tecla,1);
		if(!seleccionado) {
			c1=CINPUT1;
			c2=CINPUT2;
		}
		else {
			c1=CINPUT1_ON;
			c2=CINPUT2_ON;
		}
		txt=input->cadena;
		v_impcar(fil,col,c1,cuad->clr_input);
		imprime_txt_elem(fil,col+1,txt,input->ancho-1,
		  cuad->clr_input,cuad->clr_input,0);
		v_impcar(fil,col+input->ancho-1,c2,cuad->clr_input);
		break;
	case C_ELEM_LISTA :
		lista=(STC_ELEM_LISTA *)e->info;
                if(seleccionado) v_borde(&lista->v,c_Borde_Sel);
		else v_borde(&lista->v,NULL);
		v_dibuja(&lista->v,0);
		txt=lista->texto;
		if(lista->modo==C_LSTPRIMERO) clr_sel=lista->v.clr_princ;
		else clr_sel=cuad->clr_sel;
		imprime_txt_elem(lista->v.fil,lista->v.col+
		  ((lista->v.ancho-strlen(txt))/2),txt,
		  strlen(txt),cuad->v.clr_princ,cuad->clr_tecla,1);
		imprime_lista(&lista->v,lista->elemento,lista->elemento_pr,
		  lista->elemento_sel,clr_sel);
		dibuja_flechas_vert(&lista->v,lista->num_elementos,
		  lista->elemento_sel);
		break;
	case C_ELEM_CHECK :
		check=(STC_ELEM_CHECK *)e->info;
		v_impcar(fil,col,c_Esp[4],cuad->v.clr_princ);
		v_impcar(fil,col+2,c_Esp[5],cuad->v.clr_princ);
		if(!seleccionado) v_impcar(fil,col+3,' ',cuad->v.clr_princ);
		else v_impcar(fil,col+3,c_Esp[6],cuad->v.clr_princ);
		txt=check->texto;
		imprime_txt_elem(fil,col+4,txt,strlen(txt),cuad->v.clr_princ,
		  cuad->clr_tecla,1);
		if(check->estado) v_impcar(fil,col+1,c_Esp[7],cuad->v.clr_princ);
		else v_impcar(fil,col+1,c_Esp[8],cuad->v.clr_princ);
		break;
	case C_ELEM_TEXTO :
		texto=(STC_ELEM_TEXTO *)e->info;
		if(texto->borde==C_TXTBORDE) v_dibuja(&texto->v,1);
		txt=texto->texto;
		v_pon_cursor(&texto->v,0,0);
		v_impcad(&texto->v,txt,V_NORELLENA);
		break;
}

r_puntero(R_MUESTRA);

}

/****************************************************************************
	C_ABRE: abre un cuadro de di logo.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
****************************************************************************/
void c_abre(STC_CUADRO *cuad)
{
STC_ELEM *e;

v_abre(&cuad->v);
for(e=cuad->elem1; e!=NULL; e=e->sgte) c_dibuja_elemento(cuad,e);

}

/****************************************************************************
	C_CIERRA: cierra un cuadro de di logo.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
****************************************************************************/
void c_cierra(STC_CUADRO *cuad)
{

v_cierra(&cuad->v);

}

/****************************************************************************
	C_ELIMINA: elimina un cuadro de di logo y todos sus elementos
 	  asociados liberando la memoria ocupada.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
****************************************************************************/
void c_elimina(STC_CUADRO *cuad)
{
STC_ELEM *e, *e1;

e=cuad->elem1;
while(e!=NULL) {
	e1=e->sgte;
	free(e);
	e=e1;
}

cuad->elem1=NULL;
cuad->elemento=NULL;

}

/****************************************************************************
	C_CAMBIA_COLOR: modifica los colores de un cuadro de di logo.
	  Entrada:      'cuad' puntero a estructura del cuadro
			'clr_princ' color principal del cuadro
			'clr_s1', 'clr_s2' colores para sombra
			'clr_boton' color para elemento bot¢n
			'clr_input' color para elemento input
			'clr_sel' color para elemento seleccionado en listas
			'clr_tecla' color para teclas de activaci¢n
			'clr_tecla_boton' color para tecla activaci¢n bot¢n
			'modo' 1 redibuja el cuadro, 0 no lo redibuja
****************************************************************************/
void c_cambia_color(STC_CUADRO *cuad, BYTE clr_princ, BYTE clr_s1,
  BYTE clr_s2, BYTE clr_boton, BYTE clr_input, BYTE clr_sel, BYTE clr_tecla,
  BYTE clr_tecla_boton, int modo)
{
STC_ELEM *e;
STC_VENTANA *v;

cuad->v.clr_princ=clr_princ;
cuad->v.clr_s1=clr_s1;
cuad->v.clr_s2=clr_s2;
cuad->v.clr_texto=clr_princ;

cuad->clr_boton=clr_boton;
cuad->clr_input=clr_input;
cuad->clr_sel=clr_sel;
cuad->clr_tecla=clr_tecla;
cuad->clr_tecla_boton=clr_tecla_boton;

/* recorremos elementos del cuadro para modificar los colores en */
/* los que es necesario (C_ELEM_LISTA y C_ELEM_TEXTO) */
for(e=cuad->elem1; e!=NULL; e=e->sgte) {
	if(e->tipo==C_ELEM_LISTA) {
		v=&((STC_ELEM_LISTA *)(e->info))->v;
		v->clr_princ=clr_princ;
		v->clr_s1=clr_s1;
		v->clr_s2=clr_s2;
		v->clr_texto=clr_princ;
	}
	else if(e->tipo==C_ELEM_TEXTO) {
		v=&((STC_ELEM_TEXTO *)(e->info))->v;
		v->clr_princ=clr_princ;
		v->clr_s1=clr_s1;
		v->clr_s2=clr_s2;
		v->clr_texto=clr_princ;
	}
}

/* redibuja el cuadro */
if(modo) {
	v_dibuja(&cuad->v,1);
	for(e=cuad->elem1; e!=NULL; e=e->sgte) c_dibuja_elemento(cuad,e);
}

}

/****************************************************************************
	C_LEE_INPUT: lee una cadena de caracteres por teclado o edita una
	  cadena ya existente.
	  Entrada:      'fil', 'col' posici¢n de la zona de 'input'
			'ancho' anchura de la zona de 'input'
			'clr' color para zona de 'input'
			'cadena' puntero a buffer d¢nde se guardar 
			la cadena tecleada; si el primer car cter no es '\0'
			se ajustar  la rutina para poder editar la cadena
			'longitud' longitud de la cadena (sin contar el
			\0 final)
	  Salida:       n£mero de caracteres tecleados
			-1 si se puls¢ TAB
			-2 si puls¢ con rat¢n fuera de la ventana
			-3 si se puls¢ ESCAPE
			-4 si se puls¢ ALT+tecla (variable global 'alt_tecla'
			   contendr  c¢digo ASCII de tecla pulsada)
****************************************************************************/
int c_lee_input(int fil, int col, int ancho, BYTE clr, char *cadena,
  int longitud)
{
STC_RATON r;
char *cur, *fin, *ptr;
int num_car=0, ccur, tecla, shift, ascii, fila_ant, columna_ant, maxcol;

/* coge posici¢n actual del cursor */
coge_pos_cursor(&fila_ant,&columna_ant);

/* busca final de la cadena y n£mero de caracteres */
for(fin=cadena; *fin; fin++, num_car++);

/* inicializa posici¢n del cursor */
cur=fin;
if(num_car<ancho) ccur=num_car;
else ccur=ancho-1;

do {
	/* imprime l¡nea */
	imprime_txt_elem(fil,col,cur-ccur,ancho,clr,clr,0);

	/* imprime cursor */
	pon_cursor(fil,col+ccur);

	do {
		/* recoge posici¢n del rat¢n */
		r_estado(&r);
		if(r.boton1) {
			/* comprueba si se puls¢ fuera de la zona de 'input' */
			/* y si es as¡ sale */
			maxcol=col+ancho-1;
			if((r.fil!=fil) || (r.col<col) || (r.col>maxcol)) {
				/* imprime l¡nea */
				imprime_txt_elem(fil,col,cadena,ancho,clr,
				  clr,0);
				pon_cursor(fila_ant,columna_ant);
				return(-2);
			}
		}

		/* recoge tecla pulsada */
		tecla=bioskey(1);

	} while(!tecla);

	/* saca tecla del buffer */
	bioskey(0);

	/* sale si se ha pulsado ALT y tecla pulsada no genera c¢digo ASCII */
	shift=bioskey(2);
	if((shift & 0x0008) && !(tecla & 0x00ff)) {
		/* imprime l¡nea */
		imprime_txt_elem(fil,col,cadena,ancho,clr,clr,0);
		alt_tecla=scan_ascii(tecla);
		return(-4);
	}

	/* recoge c¢digo ASCII */
	ascii=tecla & 0x00ff;

	/* si es un car cter v lido y no se ha alcanzado el */
	/* n£mero m ximo de caracteres permitidos */
	if((ascii>31) && (num_car<longitud)) {
		/* si cursor est  en zona intermedia de l¡nea */
		/* inserta car cter desplazando el resto */
		if(cur!=fin) {
			/* desplaza caracteres */
			for(ptr=fin; ptr>=cur; ptr--) *(ptr+1)=*ptr;
		}
		/* inserta car cter tecleado */
		*cur++=(char)ascii;
		fin++;
		/* inserta fin de l¡nea */
		*fin='\0';
		/* incrementa n£mero de caracteres tecleados */
		num_car++;
		/* incrementa posici¢n del cursor */
		if(ccur<(ancho-1)) ccur++;
	}
	/* comprueba si es una tecla de movimiento de cursor */
	else switch((tecla >> 8) & 0x00ff) {
		case 0x4b :             /* cursor izquierda */
			if(cur>cadena) {
				cur--;
				if(ccur>0) ccur--;
			}
			break;
		case 0x4d :             /* cursor derecha */
			if(cur<fin) {
				cur++;
				if(ccur<(ancho-1)) ccur++;
			}
			break;
		case 0x47 :             /* cursor origen */
			cur=cadena;
			ccur=0;
			break;
		case 0x4f :             /* cursor fin */
			cur=fin;
			if(num_car<ancho) ccur=num_car;
			else ccur=ancho-1;
			break;
		case 0x0e :             /* borrado hacia atr s */
			if(cur>cadena) {
				cur--;
				fin--;
				for(ptr=cur; ptr<=fin; ptr++) *ptr=*(ptr+1);
				num_car--;
				if(ccur>0) ccur--;
			}
			break;
		case 0x53 :             /* borrado */
			if(cur<fin) {
				for(ptr=cur; ptr<fin; ptr++) *ptr=*(ptr+1);
				fin--;
				num_car--;
			}
			break;
		case 0x0f :             /* TAB */
			/* imprime l¡nea */
			imprime_txt_elem(fil,col,cadena,ancho,clr,clr,0);
			pon_cursor(fila_ant,columna_ant);
			return(-1);
		case 0x01 :             /* ESCAPE */
			/* imprime l¡nea */
			imprime_txt_elem(fil,col,cadena,ancho,clr,clr,0);
			pon_cursor(fila_ant,columna_ant);
			return(-3);
	}

} while(tecla!=0x1c0d);	/* hasta que pulse RETURN */

/* imprime l¡nea */
imprime_txt_elem(fil,col,cadena,ancho,clr,clr,0);
pon_cursor(fila_ant,columna_ant);

return(num_car);
}

/****************************************************************************
	C_ACCION: env¡a una acci¢n a un cuadro de di logo.
	  Entrada:      'c' puntero a cuadro de di logo
			'acc' puntero a estructura con la acci¢n sobre el
			cuadro
	  Salida:       elemento seleccionado o c¢digo de acci¢n (si <0)
****************************************************************************/
int c_accion(STC_CUADRO *c, STC_ACCION_C *acc)
{
STC_RATON r;
STC_ELEM *e;
int i;
char tecla;

switch(acc->accion) {
	case CUADRO_NULA :              /* acci¢n nula */
		break;
	case CUADRO_SALIDA :            /* salir del cuadro */
		c_cierra(c);
		c_elimina(c);
		break;
	case CUADRO_SGTE_ELEM :         /* siguiente elemento */
		siguiente_elemento(c);
		resalta_elemento(c);
		return(CUADRO_NULA);
	case CUADRO_ANT_ELEM :          /* elemento anterior */
		anterior_elemento(c);
		resalta_elemento(c);
		return(CUADRO_NULA);
	case CUADRO_SELECC :
		for(i=0, e=c->elem1; (e!=NULL) && (e!=c->elemento);
		  e=e->sgte, i++);
		return(i);
	case CUADRO_TECLA :
		/* si estaba pulsado ALT convierte c¢digo 'scan' a ASCII */
		if(acc->shift & 0x0008) tecla=scan_ascii(acc->tecla);
		else tecla=mayuscula(acc->tecla & 0x00ff);

		/* busca elemento correspondiente a tecla pulsada */
		i=busca_elemento_tecla(c,tecla);
		if(i==-1) return(CUADRO_NULA);
		else return(i);
	case CUADRO_RATON :
		do {
			i=busca_elemento_raton(c,acc->fil,acc->col);
			r_estado(&r);
			acc->fil=r.fil;
			acc->col=r.col;
		} while(r.boton1);
		if(i==-1) return(CUADRO_NULA);
		else return(i);
}

return(acc->accion);
}

/****************************************************************************
	C_GESTIONA: gestiona un cuadro de di logo.
	  Entrada:      'c' puntero a cuadro de di logo
	  Salida:       £ltimo elemento seleccionado, -1 si se sali¢ del
			cuadro
****************************************************************************/
int c_gestiona(STC_CUADRO *c)
{
STC_ACCION_C acc;
STC_RATON r;
int i, accion, elemento, shift, tecla, filcur, colcur;

/* si rat¢n no est  inicializado, lo inicializa */
if(!r_puntero(R_MUESTRA)) r_inicializa();

/* oculta cursor */
coge_pos_cursor(&filcur,&colcur);
pon_cursor(25,0);

c->elemento=c->elem1;
resalta_elemento(c);
acc.accion=CUADRO_NULA;

while(1) {
	/* si elemento actual es de entrada o lista, lo selecciona */
	if(((c->elemento)->tipo==C_ELEM_INPUT) ||
	  ((c->elemento)->tipo==C_ELEM_LISTA)) {
		acc.accion=CUADRO_SELECC;
		accion=3;
	}
	else do {
		accion=0;
		r_estado(&r);
		/* si pulsado el bot¢n 1 del rat¢n, indica acci¢n del rat¢n */
		if(r.boton1) accion=1;
		else {
			/* si hay tecla esperando, indica acci¢n de teclado */
			tecla=bioskey(1);
			if(tecla) {
				bioskey(0);
				shift=bioskey(2);
				accion=2;
			}
		}
	} while(!accion);

	if(accion==1) {
		acc.accion=CUADRO_RATON;
		acc.fil=r.fil;
		acc.col=r.col;
	}
	else if(accion==2) switch((tecla >> 8) & 0x00ff) {
		case 0x01 :     /* ESCAPE */
			acc.accion=CUADRO_SALIDA;
			break;
		case 0x1c :     /* RETURN */
			acc.accion=CUADRO_SELECC;
			break;
		case 0x0f :     /* TAB */
			if(shift & 0x0003) acc.accion=CUADRO_ANT_ELEM;
			else acc.accion=CUADRO_SGTE_ELEM;
			break;
		default :
			acc.accion=CUADRO_TECLA;
			acc.tecla=tecla;
			acc.shift=shift;
			break;
	}

	elemento=c_accion(c,&acc);

	/* ejecuta acciones seg£n el tipo de elemento */
	if(elemento>=0) {
		switch((c->elemento)->tipo) {
			case C_ELEM_NULO :
				break;
			case C_ELEM_BOTON :
				pon_cursor(filcur,colcur);
				return(elemento);
			case C_ELEM_INPUT :
				i=c_elem_input(c->v.fil+(c->elemento)->fil+1,
				  c->v.col+(c->elemento)->col+1,c->clr_input,
				  (STC_ELEM_INPUT *)(c->elemento)->info);
				/* si puls¢ TAB */
				if(i==-1) {
					shift=bioskey(2);
					if(shift & 0x0003) anterior_elemento(c);
					else siguiente_elemento(c);
					resalta_elemento(c);
				}
				/* si puls¢ con rat¢n fuera del elemento */
				else if(i==-2) {
					r_estado(&r);
					busca_elemento_raton(c,r.fil,r.col);
				}
				/* si puls¢ ESC */
				else if(i==-3) {
					pon_cursor(filcur,colcur);
					return(-1);
				}
				/* si puls¢ ALT+tecla */
				else if(i==-4) busca_elemento_tecla(c,
				  alt_tecla);
				break;
			case C_ELEM_LISTA :
				i=c_elem_lista(
				  (STC_ELEM_LISTA *)(c->elemento)->info,
				  c->clr_sel);
				/* si puls¢ TAB */
				if(i==-1) {
					shift=bioskey(2);
					if(shift & 0x0003) anterior_elemento(c);
					else siguiente_elemento(c);
					resalta_elemento(c);
				}
				/* si puls¢ con rat¢n fuera del elemento */
				else if(i==-2) {
					r_estado(&r);
					busca_elemento_raton(c,r.fil,r.col);
				}
				/* si puls¢ ESC */
				else if(i==-3) {
					pon_cursor(filcur,colcur);
					return(-1);
				}
				/* si lista est  vac¡a */
				else if(i==-4) {
					siguiente_elemento(c);
					resalta_elemento(c);
				}
				/* si se puls¢ ALT+tecla */
				else if(i==-5) busca_elemento_tecla(c,
				  alt_tecla);
				break;
			case C_ELEM_CHECK :
				c_elem_check(
				  (STC_ELEM_CHECK *)(c->elemento)->info,
				  c,c->elemento);
				break;
			case C_ELEM_TEXTO :
				break;
		}
	}
	else if(elemento==CUADRO_SALIDA) {
		pon_cursor(filcur,colcur);
		return(-1);
	}
}

}

/****************************************************************************
	C_ELEM_INPUT: gestiona un elemento de entrada.
	  Entrada:      'fil', 'col' posici¢n del elemento en pantalla
			'clr' color para la zona de entrada
			'e' puntero a informaci¢n de elemento de entrada
	  Salida:       n£mero de caracteres tecleados.
			-1 si se puls¢ TAB
			-2 si puls¢n con rat¢n fuera de la ventada
			-3 si se puls¢ ESCAPE
			-4 si se puls¢ ALT+tecla (variable global 'alt_tecla'
			   contendr  c¢digo de tecla pulsada)
****************************************************************************/
int c_elem_input(int fil, int col, BYTE clr, STC_ELEM_INPUT *e)
{
int i;

i=c_lee_input(fil,col+1,e->ancho-2,clr,e->cadena,e->longitud);

return(i);
}

/****************************************************************************
	C_ELEM_LISTA: gestiona un elemento de lista.
	  Entrada:      'e' puntero a informaci¢n de elemento de lista
			'clr_sel' color para elemento seleccionado
	  Salida:       n£mero de elemento seleccionado
			-1 si se puls¢ TAB
			-2 si puls¢n con rat¢n fuera de la ventana
			-3 si se puls¢ ESCAPE
			-4 si la lista est  vac¡a
			-5 si se puls¢ ALT+tecla (variable global 'alt_tecla'
			   contendr  c¢digo de tecla pulsada)
****************************************************************************/
int c_elem_lista(STC_ELEM_LISTA *e, BYTE clr_sel)
{
STC_RATON r;
STC_VENTANA *v;
int elemento_ant=-1, tecla, shift, maxfil, maxcol, fil, col, filcur, colcur;

/* si la lista est  vac¡a, sale */
if(!e->num_elementos) return(-4);

/* oculta cursor */
coge_pos_cursor(&filcur,&colcur);
pon_cursor(25,0);

/* si es modo C_LSTPRIMERO no resalta elemento */
if(e->modo==C_LSTPRIMERO) clr_sel=e->v.clr_princ;

/* puntero a ventana de lista */
v=&e->v;

/* busca elemento seleccionado */
busca_elem_selecc(e);

while(1) {
	if(e->elemento_sel!=elemento_ant) imprime_lista(v,e->elemento,
	  e->elemento_pr,e->elemento_sel,clr_sel);
	elemento_ant=e->elemento_sel;

	do {
		tecla=0;
		/* recoge posici¢n del rat¢n */
		r_estado(&r);
		if(r.boton1) {
			/* comprueba si se puls¢ fuera de la ventana */
			/* y si es as¡ sale */
			maxfil=v->fil+v->alto-1;
			maxcol=v->col+v->ancho-1;
			if((r.fil<v->fil) || (r.fil>maxfil) ||
			  (r.col<v->col) || (r.col>maxcol)) {
				imprime_lista(v,e->elemento,e->elemento_pr,
				  e->elemento_sel,clr_sel);
				dibuja_flechas_vert(v,e->num_elementos,
				  e->elemento_sel);
				busca_elem_selecc(e);
				pon_cursor(filcur,colcur);
				return(-2);
			}
			else {
				/* calcula coordenadas relativas al */
				/* origen de la ventana */
				fil=r.fil-v->fil;
				col=r.col-v->col;

				if((fil==0) && (col==(v->ancho-1)))
				  tecla=0x4700;
				else if((fil==(v->alto-1)) &&
				  (col==(v->ancho-1))) tecla=0x4f00;
				else if((fil==1) && (col==(v->ancho-1)))
				  tecla=0x4800;
				else if((fil==(v->alto-2)) &&
				  (col==(v->ancho-1))) tecla=0x5000;
				else if((fil==0) && (col!=0) &&
				  (col!=(v->ancho-1))) tecla=0x4800;
				else if((fil==(v->alto-1)) && (col!=0) &&
				  (col!=(v->ancho-1))) tecla=0x5000;
				else if((fil<(v->alto/2)) &&
				  (col==(v->ancho-1))) tecla=0x4900;
				else if((fil>=(v->alto/2)) &&
				  (col==(v->ancho-1))) tecla=0x5100;
				else if((fil>0) && (fil<(v->alto-1)) &&
				  (col>0) && (col<(v->ancho-1))) {
					elemento_ant=e->elemento_sel;
					e->elemento_sel=e->elemento_pr+fil-1;
					if(e->elemento_sel>
					  (e->num_elementos-1))
					  e->elemento_sel=e->num_elementos-1;

					/* si elemento seleccionado es */
					/* distinto a elemento anterior */
					/* lo resalta; si no, lo selecciona */
					/* y sale */
					if(e->elemento_sel!=elemento_ant) {
						imprime_lista(v,e->elemento,
						  e->elemento_pr,
						  e->elemento_sel,clr_sel);
						dibuja_flechas_vert(v,
						  e->num_elementos,
						  e->elemento_sel);
					}
					else {
						r_estado(&r);
						if(r.boton1) break;
						imprime_lista(v,e->elemento,
						  e->elemento_pr,
						  e->elemento_sel,clr_sel);
						dibuja_flechas_vert(v,
						  e->num_elementos,
						  e->elemento_sel);
						busca_elem_selecc(e);
						pon_cursor(filcur,colcur);
						return(e->elemento_sel);
					}
				}
			}

			delay(C_PAUSA);
		}
		else {
			tecla=bioskey(1);
			/* saca tecla del buffer */
			if(tecla) bioskey(0);
			/* recoge estado de teclas 'shift' */
			shift=bioskey(2);
		}

	} while(!tecla);

	/* si est  pulsado ALT, sale */
	if(shift & 0x0008) {
		imprime_lista(v,e->elemento,e->elemento_pr,e->elemento_sel,
		  clr_sel);
		dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
		busca_elem_selecc(e);
		pon_cursor(filcur,colcur);
		alt_tecla=scan_ascii(tecla);
		return(-5);
	}

	/* comprueba si es una tecla de movimiento de cursor */
	switch((tecla >> 8) & 0x00ff) {
		case 0x48 :             /* cursor arriba */
			if(e->elemento_sel>0) e->elemento_sel--;
			if(e->modo==0) {
				if(e->elemento_sel<e->elemento_pr)
				  e->elemento_pr--;
			}
			else e->elemento_pr=e->elemento_sel;
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x50 :             /* cursor abajo */
			if(e->elemento_sel<(e->num_elementos-1))
			  e->elemento_sel++;
			if(e->modo==0) {
				if(e->elemento_sel>(e->elemento_pr+v->alto-3))
				  e->elemento_pr++;
			}
			else e->elemento_pr=e->elemento_sel;
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x49 :             /* p gina arriba */
			if(e->elemento_pr==0) break;
			e->elemento_pr-=v->alto-2;
			if(e->elemento_pr<0) e->elemento_pr=0;
			e->elemento_sel=e->elemento_pr;
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,clr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x51 :             /* p gina abajo */
			if(e->elemento_pr==(e->num_elementos-1)) break;
			e->elemento_pr+=v->alto-2;
			if(e->elemento_pr>(e->num_elementos-1))
			  e->elemento_pr=e->num_elementos-1;
			e->elemento_sel=e->elemento_pr;
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,clr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x47 :             /* cursor origen */
			e->elemento_pr=0;
			e->elemento_sel=0;
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x4f :             /* cursor fin */
			e->elemento_pr=e->num_elementos-(v->alto-2);
			if(e->elemento_pr<0) e->elemento_pr=0;
			e->elemento_sel=e->num_elementos-1;
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			break;
		case 0x1c :             /* RETURN */
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,clr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			busca_elem_selecc(e);
			pon_cursor(filcur,colcur);
			return(e->elemento_sel);
		case 0x0f :             /* TAB */
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,clr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			busca_elem_selecc(e);
			pon_cursor(filcur,colcur);
			return(-1);
		case 0x01 :             /* ESCAPE */
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel,clr_sel);
			dibuja_flechas_vert(v,e->num_elementos,
			  e->elemento_sel);
			e->selecc=NULL;
			pon_cursor(filcur,colcur);
			return(-3);
	}
}

}

/****************************************************************************
	C_METE_EN_LISTA: inserta una cadena al final de la lista.
	  Entrada:      'e' puntero a la estructura con la informaci¢n de
			la lista
			'cadena' puntero a cadena de caracteres a a¤adir
			al final de la lista
	  Salida:       1 si se pudo crear y a¤adir elemento a lista, si
			no devuelve 0
****************************************************************************/
int c_mete_en_lista(STC_ELEM_LISTA *e, char *cadena)
{
STC_LISTA *l_ultimo, *l_anterior, *l;

/* crea nuevo elemento */
l=(STC_LISTA *)malloc(sizeof(STC_LISTA));
if(l==NULL) return(0);
l->cadena=(char *)malloc(strlen(cadena)+1);
if(l->cadena==NULL) {
	free(l);
	return(0);
}
strcpy(l->cadena,cadena);
l->sgte=NULL;

/* puntero a primer elemento de la lista */
l_ultimo=e->elemento;

/* si el primer elemento est  vac¡o, mete ah¡ la cadena */
if(l_ultimo==NULL) {
	e->elemento=l;
	e->selecc=l->cadena;
	e->num_elementos++;
	return(1);
}

/* si lista no ordenada, a¤ade elemento al final */
if(e->orden==C_LSTSINORDEN) {
	/* busca £ltimo elemento de la lista */
	while(l_ultimo->sgte!=NULL) l_ultimo=l_ultimo->sgte;
	l_ultimo->sgte=l;
}
else {
	l_anterior=NULL;
	/* busca elemento anterior al que se quiere insertar */
	while((l_ultimo->sgte!=NULL) && (strcmp(l_ultimo->cadena,cadena)<0)) {
		l_anterior=l_ultimo;
		l_ultimo=l_ultimo->sgte;
	}

	/* si elemento debe ir en primera posici¢n */
	if(l_anterior==NULL) {
		/* si s¢lo hay un elemento, comprueba si el que */
		/* se quiere insertar va antes o despu‚s */
		if(l_ultimo->sgte==NULL) {
			if(strcmp(l_ultimo->cadena,cadena)<0) {
				l_ultimo->sgte=l;
				l->sgte=NULL;
			}
			else {
				l->sgte=l_ultimo;
				e->elemento=l;
			}
		}
		else {
			l->sgte=e->elemento;
			e->elemento=l;
		}
	}
	else {
		/* si se lleg¢ a £ltimo elemento comprueba si el que */
		/* se quiere insertar va antes o despu‚s */
		if(l_ultimo->sgte==NULL) {
			if(strcmp(l_ultimo->cadena,cadena)<0) {
				l_ultimo->sgte=l;
				l->sgte=NULL;
			}
			else {
				l->sgte=l_ultimo;
				l_anterior->sgte=l;
			}
		}
		else {
			l->sgte=l_anterior->sgte;
			l_anterior->sgte=l;
		}
	}
}

/* incrementa n£mero de elementos en lista */
e->num_elementos++;

/* busca elemento seleccionado */
busca_elem_selecc(e);

return(1);
}

/****************************************************************************
	C_BORRA_LISTA: elimina todos los elementos de una lista, liberando
	  la memoria ocupada.
	  Entrada:      'e' puntero a la estructura con la informaci¢n de
			la lista
****************************************************************************/
void c_borra_lista(STC_ELEM_LISTA *e)
{
STC_LISTA *l, *l_sgte;

/* puntero a primer elemento de la lista */
l=e->elemento;

/* sale si lista est  vac¡a */
if(l==NULL) return;

do {
	/* puntero a siguiente elemento de la lista */
	l_sgte=l->sgte;

	free(l->cadena);
	free(l);

	l=l_sgte;

} while(l_sgte!=NULL);

e->elemento=NULL;
e->selecc=NULL;
e->num_elementos=0;
e->elemento_pr=0;
e->elemento_sel=0;

}

/****************************************************************************
	C_ELEM_CHECK: gestiona un elemento de caja de comprobaci¢n.
	  Entrada:      'e' puntero a informaci¢n de elemento de entrada
			'cuad' puntero a cuadro al que pertenece
			'elem' puntero a elemento dentro del cuadro
	  Salida:       estado; 0 desactivada, 1 activada
****************************************************************************/
int c_elem_check(STC_ELEM_CHECK *e, STC_CUADRO *cuad, STC_ELEM *elem)
{

if(e->estado==0) e->estado=1;
else e->estado=0;

c_dibuja_elemento(cuad,elem);

return(e->estado);
}

/****************************************************************************
	C_SELECC_FICHEROS: gestiona un cuadro de selecci¢n de ficheros.
	  Entrada:      'fil', 'col' posici¢n del cuadro; si 'fil' o 'col'
			son iguales a C_CENT se centrar  el cuadro
			'titulo' t¡tulo del cuadro
			'clr_princ' color principal
			'clr_s1', 'clr_s2' colores de sombreado
			'clr_boton' color de botones
 			'clr_input' color de input
			'clr_tecla' color de teclas de activaci¢n
			'clr_tecla_boton' color de tecla de activaci¢n botones
			'clr_sel' color elemento seleccionado
			'ruta' ruta completa al directorio; si se da una
			cadena vac¡a se coge la ruta al directorio actual
			'mascara' m scara de b£squeda de ficheros
			'fichero' puntero a buffer donde se dejar  la
			ruta completa y el nombre del fichero seleccionado o
			una cadena vac¡a si no seleccion¢ ning£n fichero; el
			buffer debe ser de una longitud de MAXPATH bytes
****************************************************************************/
void c_selecc_ficheros(int fil, int col, char *titulo, BYTE clr_princ,
  BYTE clr_s1, BYTE clr_s2, BYTE clr_boton, BYTE clr_input, BYTE clr_tecla,
  BYTE clr_tecla_boton, BYTE clr_sel, char *ruta, char *mascara, char *fichero)
{
STC_CUADRO cfich;
STC_ACCION_C acc;
STC_RATON r;
STC_ELEM *elista, *einput;
STC_ELEM_LISTA *lista_fich;
char *c, nfich[MAXPATH], nfich2[MAXPATH], disq[MAXDRIVE], direct[MAXDIR],
  fich[MAXFILE], ext[MAXEXT], dir_orig[MAXPATH], dir[15];
int i, elemento, accion, unid_orig, shift, tecla, filcur, colcur;

/* guarda directorio y unidad actuales */
getcwd(dir_orig,MAXPATH);
unid_orig=getdisk();

/* si rat¢n no est  inicializado, lo inicializa */
if(!r_puntero(R_MUESTRA)) r_inicializa();

/* oculta cursor */
coge_pos_cursor(&filcur,&colcur);
pon_cursor(25,0);

/* inicializa buffer para nombre de fichero */
if(*ruta) strcpy(nfich,ruta);
else strcpy(nfich,dir_orig);

/* cambia a directorio */
i=strlen(nfich);
if(nfich[i-1]=='\\') nfich[i-1]='\0';
chdir(nfich);

pon_barra_dir(nfich);
strcat(nfich,mascara);
may_str(nfich);

/* crea cuadro de selecci¢n de ficheros */
c_crea_cuadro(&cfich,titulo,fil,col,C_FICH_ANCHO,C_FICH_ALTO,clr_princ,
  clr_s1,clr_s2,clr_boton,clr_input,clr_sel,clr_tecla,clr_tecla_boton);
einput=c_crea_elemento(&cfich,C_ELEM_INPUT,0,9,"^Fichero",C_FICH_ANCHO-12,
  nfich,MAXPATH-1);
elista=c_crea_elemento(&cfich,C_ELEM_LISTA,2,0," ^Lista ",C_FICH_ANCHO-20,
  C_FICH_ALTO-4,C_LSTNORMAL,C_LSTORDENADA);
lista_fich=(STC_ELEM_LISTA *)elista->info;
c_crea_elemento(&cfich,C_ELEM_BOTON,C_FICH_ALTO-6,C_FICH_ANCHO-17,"^Vale",11);
c_crea_elemento(&cfich,C_ELEM_BOTON,C_FICH_ALTO-4,C_FICH_ANCHO-17,"^Salir",11);

cfich.elemento=cfich.elem1;
lista_ficheros(lista_fich,nfich);
c_abre(&cfich);
resalta_elemento(&cfich);
acc.accion=CUADRO_NULA;

while(1) {
	/* si elemento actual es de entrada o lista, lo selecciona */
	if(((cfich.elemento)->tipo==C_ELEM_INPUT) ||
	  ((cfich.elemento)->tipo==C_ELEM_LISTA)) {
		acc.accion=CUADRO_SELECC;
		accion=3;
	}
	else do {
		accion=0;
		r_estado(&r);
		/* si pulsado el bot¢n 1 del rat¢n, indica acci¢n del rat¢n */
		if(r.boton1) accion=1;
		else {
			/* si hay tecla esperando, indica acci¢n de teclado */
			tecla=bioskey(1);
			if(tecla) {
				bioskey(0);
				shift=bioskey(2);
				accion=2;
			}
		}
	} while(!accion);

	if(accion==1) {
		acc.accion=CUADRO_RATON;
		acc.fil=r.fil;
		acc.col=r.col;
	}
	else if(accion==2) switch((tecla >> 8) & 0x00ff) {
		case 0x01 :     /* ESCAPE */
			acc.accion=CUADRO_SALIDA;
			break;
		case 0x1c :     /* RETURN */
			acc.accion=CUADRO_SELECC;
			break;
		case 0x0f :     /* TAB */
			if(shift & 0x0003) acc.accion=CUADRO_ANT_ELEM;
			else acc.accion=CUADRO_SGTE_ELEM;
			break;
		default :
			acc.accion=CUADRO_TECLA;
			acc.tecla=tecla;
			acc.shift=shift;
			break;
	}

	elemento=c_accion(&cfich,&acc);

	/* ejecuta acciones seg£n el elemento seleccionado */
	switch(elemento) {
		case 0 :        /* ventana introducci¢n nombre fichero */
			i=c_elem_input(cfich.v.fil+(cfich.elemento)->fil+1,
			  cfich.v.col+(cfich.elemento)->col+1,
			  cfich.clr_input,(cfich.elemento)->info);
			lista_ficheros(lista_fich,nfich);
			c_dibuja_elemento(&cfich,elista);
			strcpy(nfich2,nfich);
			c=strrchr(nfich2,'\\');
			*c='\0';
			chdir(nfich2);
			/* si puls¢ TAB */
			if(i==-1) {
				shift=bioskey(2);
				if(shift & 0x0003) anterior_elemento(&cfich);
				else siguiente_elemento(&cfich);
				resalta_elemento(&cfich);
			}
			/* si se puls¢ con rat¢n fuera del elemento */
			else if(i==-2) {
				r_estado(&r);
				busca_elemento_raton(&cfich,r.fil,r.col);
			}
			/* si puls¢ ESC */
			else if(i==-3) {
				c_cierra(&cfich);
				c_elimina(&cfich);
				c_borra_lista(lista_fich);
				*fichero='\0';
				/* restaura directorio y unidad de origen */
				setdisk(unid_orig);
				chdir(dir_orig);
				pon_cursor(filcur,colcur);
				return;
			}
			/* si puls¢ ALT+tecla */
			else if(i==-4) busca_elemento_tecla(&cfich,alt_tecla);
			break;
		case 1 :        /* lista de ficheros */
			i=c_elem_lista((STC_ELEM_LISTA *)(cfich.elemento)->info,
			  cfich.clr_sel);
			/* si puls¢ TAB */
			if(i==-1) {
				shift=bioskey(2);
				if(shift & 0x0003) anterior_elemento(&cfich);
				else siguiente_elemento(&cfich);
				resalta_elemento(&cfich);
			}
			/* si puls¢ con rat¢n fuera del elemento */
			else if(i==-2) {
				r_estado(&r);
				busca_elemento_raton(&cfich,r.fil,r.col);
			}
			/* si puls¢ ESC */
			else if(i==-3) {
				c_cierra(&cfich);
				c_elimina(&cfich);
				c_borra_lista(lista_fich);
				*fichero='\0';
				/* restaura directorio y unidad de origen */
				setdisk(unid_orig);
				chdir(dir_orig);
				pon_cursor(filcur,colcur);
				return;
			}
			/* si lista est  vac¡a */
			else if(i==-4) {
				siguiente_elemento(&cfich);
				resalta_elemento(&cfich);
			}
			/* si se puls¢ ALT+tecla */
			else if(i==-5) busca_elemento_tecla(&cfich,alt_tecla);
			/* si seleccion¢ un elemento de la lista */
			/* comprueba si es directorio o fichero */
			else if((i>=0) && (lista_fich->selecc!=NULL)) {
				fnsplit(nfich,disq,direct,fich,ext);
				if(*lista_fich->selecc==C_CHRDIR1) {
					/* coge nombre de directorio */
					strcpy(dir,lista_fich->selecc+1);
					/* elimina car cter final */
					i=strlen(dir);
					dir[i-1]='\0';

					/* cambia a ese directorio */
					chdir(dir);

					/* coge nueva ruta */
					getcwd(nfich,MAXPATH);
					pon_barra_dir(nfich);
					strcat(nfich,mascara);
					c_dibuja_elemento(&cfich,einput);

					/* crea lista nueva */
					lista_ficheros(lista_fich,nfich);
					c_dibuja_elemento(&cfich,elista);
				}
				else {
					strcpy(nfich,disq);
					strcat(nfich,direct);
					strcat(nfich,lista_fich->selecc);
					c_cierra(&cfich);
					c_elimina(&cfich);
					c_borra_lista(lista_fich);
					strcpy(fichero,nfich);
					/* restaura directorio y unidad */
					/* de origen */
					setdisk(unid_orig);
					chdir(dir_orig);
					pon_cursor(filcur,colcur);
					return;
				}
			}
			break;
		case 2 :        /* bot¢n 'Vale' */
			fnsplit(nfich,disq,direct,fich,ext);
			/* si hay elemento seleccionado de lista */
			/* y no es directorio */
			if((lista_fich->selecc!=NULL) &&
			  (*lista_fich->selecc!=C_CHRDIR1)) {
				strcpy(nfich,disq);
				strcat(nfich,direct);
				strcat(nfich,lista_fich->selecc);
			}
			c_cierra(&cfich);
			c_elimina(&cfich);
			c_borra_lista(lista_fich);
			strcpy(fichero,nfich);
			/* restaura directorio y unidad de origen */
			setdisk(unid_orig);
			chdir(dir_orig);
			pon_cursor(filcur,colcur);
			return;
		case 3 :        /* bot¢n 'Salir' */
		case CUADRO_SALIDA :
			c_cierra(&cfich);
			c_elimina(&cfich);
			c_borra_lista(lista_fich);
			*fichero='\0';
			/* restaura directorio y unidad de origen */
			setdisk(unid_orig);
			chdir(dir_orig);
			pon_cursor(filcur,colcur);
			return;
	}
}

}
