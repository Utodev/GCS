/****************************************************************************
				  CUADROG.C

	Biblioteca de funciones para gestionar cuadros de di logo en los
	modos gr ficos de 16 y 256 colores.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- cg_crea_cuadro: crea e inicializa un cuadro de di logo
		- cg_crea_elemento: crea o modifica un elemento de un cuadro
		- cg_dibuja_elemento: dibuja un elemento de un cuadro
		- cg_abre: abre un cuadro de di logo
		- cg_cierra: cierra un cuadro de di logo
		- cg_lee_input: permite la introducci¢n de una cadena de
		    caracteres en una ventana
		- cg_accion: env¡a una acci¢n a un cuadro de di logo
		- cg_gestiona: gestiona un cuadro de di logo
		- cg_elem_input: gestiona un elemento de entrada
		- cg_elem_lista: gestiona un elemento de lista
		- cg_mete_en_lista: inserta una cadena al final de una lista
		- cg_borra_lista: elimina una lista y libera memoria
		- cg_elem_check: gestiona un elemento caja de comprobaci¢n
		- cg_selecc_ficheros: gestiona cuadro de selecci¢n de ficheros

	Las siguientes estructuras est n definidas en CUADROG.H:
		STC_ELEMG: elemento de un cuadro de di logo
		STC_CUADROG: cuadro de di logo
		STC_ACCION_CG: acci¢n para un cuadro de di logo
		STC_ELEM_BOTONG: elemento bot¢n
		STC_ELEM_INPUTG: elemento de entrada
		STC_ELEM_LISTAG: elemento de lista
		STC_ELEM_CHECKG: elemento de caja de comprobaci¢n
		STC_ELEM_TETXOG: elemento de texto
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
#include "graf.h"
#include "ventanag.h"
#include "cuadrog.h"

/*** Variables globales internas ***/
static char alt_tecla=0;

/*** Prototipos de funciones internas ***/
static int int24_hnd(int errval, int ax, int bp, int si);
static char mayuscula(char c);
static void may_str(char *s);
static void dibuja_rec_elem(int x0, int y0, int x1, int y1, int clr_s1,
  int clr_s2, int clr_fondo, int rellena, int seleccionado);
static void imprime_txt_elem(int fil, int col, char *txt, int lng, BYTE colorf,
  BYTE colorp, BYTE colortf, BYTE colortp, int modo);
static void resalta_elemento(STC_CUADROG *cuad);
static void siguiente_elemento(STC_CUADROG *c);
static void anterior_elemento(STC_CUADROG *c);
static void imprime_lista(STC_VENTANAG *v, STC_LISTAG *l, int elemento,
  int elemento_sel);
static void busca_elem_selecc(STC_ELEM_LISTAG *e);
static void dibuja_flechas_vert(STC_VENTANAG *v, int nelem, int elem);
static int lista_ficheros(STC_ELEM_LISTAG *lista, char *ruta_masc);
static void pon_barra_dir(char *dir);
static char scan_ascii(int tecla);
static int busca_elemento_tecla(STC_CUADROG *c, char tecla);
static int busca_elemento_raton(STC_CUADROG *c, int fil, int col);
static void inicializa_elem_boton(STC_ELEM_BOTONG *e, int ancho, char *texto);
static void inicializa_elem_input(STC_ELEM_INPUTG *e, int ancho, char *texto,
  char *buff, int longitud);
static void inicializa_elem_lista(STC_ELEM_LISTAG *e, int fil, int col,
  int ancho, int alto, char *texto, BYTE clr_fondo, BYTE clr_pplano,
  BYTE clr_s1, BYTE clr_s2, int modo, int orden);
static void inicializa_elem_check(STC_ELEM_CHECKG *e, char *texto);
static void inicializa_elem_texto(STC_ELEM_TEXTOG *e, char *texto, int fil,
  int col, int ancho, int alto, BYTE clr_fondo, BYTE clr_pplano, BYTE clr_s1,
  BYTE clr_s2, int modo, int borde);

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
	DIBUJA_REC_ELEM: dibuja recuadro de elemento.
	  Entrada:      'x0', 'y0' coordenadas esquina superior izquierda
			'x1', 'y1' coordenadas esquina inferior derecha
			'clr_s1', 'clr_s2' colores de sombra
			'clr_fondo' color de fondo
			'rellena' 0 s¢lo dibuja marco, 1 rellena interior
			(+2 para que no se dibuje la parte derecha del
			marco; esto se usa en elementos de lista)
			'seleccionado' 1 si elemento est  seleccionado,
			0 si no
****************************************************************************/
void dibuja_rec_elem(int x0, int y0, int x1, int y1, int clr_s1, int clr_s2,
  int clr_fondo, int rellena, int seleccionado)
{
int y, xr0, xr1;

rg_puntero(RG_OCULTA);

/* borde exterior */
g_linea(x0,y0,x0,y1,clr_s1,G_NORM);
g_linea(x0,y0,x1,y0,clr_s1,G_NORM);
g_linea(x0+1,y1,x1,y1,clr_s2,G_NORM);
if(!(rellena & 0x02)) g_linea(x1,y0+1,x1,y1,clr_s2,G_NORM);

/* rellena interior */
if(rellena & 0xfd) {
	xr0=x0+1;
	xr1=x1-1;
	for(y=y0+1; y<=(y1-1); y++) g_linea(xr0,y,xr1,y,clr_fondo,G_NORM);
}
else {
	g_linea(x0+1,y0+1,x1-1,y0+1,clr_fondo,G_NORM);
	g_linea(x0+1,y0+1,x0+1,y1-1,clr_fondo,G_NORM);
	g_linea(x0+1,y1-1,x1-1,y1-1,clr_fondo,G_NORM);
	if(!(rellena & 0x02)) g_linea(x1-1,y0+1,x1-1,y1-1,clr_fondo,G_NORM);
}

if(seleccionado) {
	g_linea(x0+1,y0+1,x1-1,y0+1,clr_s1,G_NORM);
	g_linea(x0+1,y0+1,x0+1,y1-1,clr_s1,G_NORM);
	g_linea(x0+2,y1-1,x1-1,y1-1,clr_s2,G_NORM);
	if(!(rellena & 0x02)) g_linea(x1-1,y0+2,x1-1,y1-1,clr_s2,G_NORM);
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	IMPRIME_TXT_ELEM: imprime el texto de un elemento, resaltando la
	  tecla asociada.
	  Entrada:      'fil', 'col' posici¢n del texto
			'txt' texto a imprimir
			'lng' longitud a imprimir
			'colorf', 'colorp' colores de fondo y primer planoi
			para texto
			'colortf', 'colortp' colores de fondo y primer plano
			para tecla de activaci¢n
			'modo' modo de impresi¢n; 0 no se tiene en cuenta
			CG_CARTECLA, 1 si
****************************************************************************/
void imprime_txt_elem(int fil, int col, char *txt, int lng, BYTE colorf,
  BYTE colorp, BYTE colortf, BYTE colortp, int modo)
{

rg_puntero(RG_OCULTA);

while(*txt && (lng>0)) {
	/* si es tecla asociada, la imprime en otro color */
	if((*txt==CG_CARTECLA) && modo) {
		txt++;
		lng--;
		vg_impcar(fil,col,*txt,colortf,colortp);
	}
	else vg_impcar(fil,col,*txt,colorf,colorp);

	txt++;
	col++;
	lng--;
}

/* rellena con espacios hasta el final */
for(; lng>0; lng--, col++) vg_impcar(fil,col,' ',colorf,colorp);

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	RESALTA_ELEMENTO: resalta el elemento actual de un cuadro de
	  di logo.
	  Entrada:      'cuad' puntero a estructura de cuadro
****************************************************************************/
void resalta_elemento(STC_CUADROG *cuad)
{
STC_ELEMG *elem;

for(elem=cuad->elem1; elem!=NULL; elem=elem->sgte) cg_dibuja_elemento(cuad,
  elem);

}

/****************************************************************************
	SIGUIENTE_ELEMENTO: selecciona el siguiente elemento v lido de un
	  cuadro de di logo.
	  Entrada:      'c' puntero a estructura de cuadro
****************************************************************************/
void siguiente_elemento(STC_CUADROG *c)
{
STC_ELEMG *elem;

elem=c->elemento;

if(elem->sgte==NULL) {
	c->elemento=c->elem1;
	return;
}

do {
	elem=elem->sgte;
	if(elem==NULL) elem=c->elem1;
} while((elem->tipo==CG_ELEM_NULO) || (elem->tipo==CG_ELEM_TEXTO));

c->elemento=elem;

}

/****************************************************************************
	ANTERIOR_ELEMENTO: selecciona el elemento anterior v lido de un
	  cuadro de di logo.
	  Entrada:      'cuad' puntero a estructura de cuadro
****************************************************************************/
void anterior_elemento(STC_CUADROG *c)
{
STC_ELEMG *elem;

elem=c->elemento;

if(elem->ant==NULL) {
	while(elem->sgte!=NULL) elem=elem->sgte;
	c->elemento=elem;
	return;
}

do {
	elem=elem->ant;
	if(elem==NULL) while(elem->sgte!=NULL) elem=elem->sgte;
} while((elem->tipo==CG_ELEM_NULO) || (elem->tipo==CG_ELEM_TEXTO));

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
****************************************************************************/
void imprime_lista(STC_VENTANAG *v, STC_LISTAG *l, int elemento,
  int elemento_sel)
{
STC_LISTAG *elem;
int i, j, chralt, x0, y0, x1, y1;

/* si lista vac¡a, sale */
if(l==NULL) return;

rg_puntero(RG_OCULTA);

/* posiciona el puntero en el primer elemento a imprimir */
elem=l;
for(i=0; i<elemento; i++) {
	if(elem->sgte==NULL) break;
	else elem=elem->sgte;
}

vg_pon_cursor(v,0,0);

/* imprime hasta el £ltimo elemento o hasta que se llene la ventana */
j=0;
while((elem!=NULL) && (j<(v->alto-2))) {
	/* calcula recuadro para elemento */
	chralt=chr_altura();
	y0=(v->fil+v->filc+1)*chralt;
	y1=y0+chralt-1;
	x0=(v->col+v->colc+1)*8;
	x1=x0+((v->ancho-2)*8)-1;

	/* si es elemento seleccionado lo resalta */
	if((i+j)==elemento_sel) {
		vg_impcad(v,elem->cadena,VG_RELLENA);
		rg_puntero(RG_OCULTA);
		g_linea(x0,y0,x1,y0,v->clr_s1,G_NORM);
		g_linea(x0,y0,x0,y1,v->clr_s1,G_NORM);
		g_linea(x1,y0+1,x1,y1,v->clr_s2,G_NORM);
		g_linea(x0+1,y1,x1,y1,v->clr_s2,G_NORM);
		rg_puntero(RG_MUESTRA);
	}
	else {
		rg_puntero(RG_OCULTA);
		g_rectangulo(x0,y0,x1,y1,v->clr_fondo,G_NORM,0);
		rg_puntero(RG_MUESTRA);
		vg_impcad(v,elem->cadena,VG_RELLENA);
	}
	v->colc=0;
	v->filc++;
	elem=elem->sgte;
	j++;
}

/* rellena resto de la ventana con l¡neas en blanco */
for(; j<(v->alto-2); j++) {
	vg_impcad(v,"",VG_RELLENA);
	v->colc=0;
	v->filc++;
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	BUSCA_ELEM_SELECC: busca elemento seleccionado de una lista.
	  Entrada:      'e' puntero a lista
****************************************************************************/
void busca_elem_selecc(STC_ELEM_LISTAG *e)
{
STC_LISTAG *elem;
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
void dibuja_flechas_vert(STC_VENTANAG *v, int nelem, int elem)
{
int i, x, y, chralt, lng, pos_v;

rg_puntero(RG_OCULTA);

chralt=chr_altura();

x=(v->col+v->ancho-1)*8;
y=(v->fil+1)*chralt;
lng=(v->alto-2)*chralt;

g_linea(x,y,x,y+lng-2,v->clr_pplano,G_NORM);
for(i=x+1; i<(x+chralt-1); i++) g_linea(i,y,i,y+lng-2,v->clr_fondo,G_NORM);
g_linea(i,y,i,y+lng-2,v->clr_pplano,G_NORM);
imp_chr_pos(x+((chralt-8)/2),y);
imp_chr(CG_FLECHARR,v->clr_fondo,v->clr_pplano,CHR_NORM);
g_linea(x,y,x+chralt-1,y,v->clr_s2,G_NORM);
g_linea(x,y,x,y+chralt-1,v->clr_s2,G_NORM);
g_linea(x+chralt-1,y+1,x+chralt-1,y+chralt-1,v->clr_s1,G_NORM);
g_linea(x+1,y+chralt-1,x+chralt-1,y+chralt-1,v->clr_s1,G_NORM);
y=y+lng-chralt-1;
imp_chr_pos(x+((chralt-8)/2),y);
imp_chr(CG_FLECHABJ,v->clr_fondo,v->clr_pplano,CHR_NORM);
g_linea(x,y,x+chralt-1,y,v->clr_s2,G_NORM);
g_linea(x,y,x,y+chralt-1,v->clr_s2,G_NORM);
g_linea(x+chralt-1,y+1,x+chralt-1,y+chralt-1,v->clr_s1,G_NORM);
g_linea(x+1,y+chralt-1,x+chralt-1,y+chralt-1,v->clr_s1,G_NORM);

lng=(v->alto-5)*chralt;
if(nelem==1) pos_v=0;
else pos_v=(int)(((unsigned long)(lng-1)*elem)/(nelem-1));
pos_v=pos_v+((v->fil+2)*chralt);

g_linea(x,pos_v,x+chralt-1,pos_v,v->clr_s2,G_NORM);
g_linea(x,pos_v,x,pos_v+chralt-1,v->clr_s2,G_NORM);
g_linea(x+chralt-1,pos_v+1,x+chralt-1,pos_v+chralt-1,v->clr_s1,G_NORM);
g_linea(x+1,pos_v+chralt-1,x+chralt-1,pos_v+chralt-1,v->clr_s1,G_NORM);

rg_puntero(RG_MUESTRA);

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
int lista_ficheros(STC_ELEM_LISTAG *lista, char *ruta_masc)
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
cg_borra_lista(lista);

/* crea ruta + "*.*" */
strcpy(nfich,disq);
strcat(nfich,direct);
strcat(nfich,"*.*");

/* busca primero subdirectorios */
encontrado=findfirst(nfich,&find,FA_DIREC);

while(!encontrado) {
	if(find.ff_attrib & FA_DIREC) {
		/* a¤ade caracteres indicadores de directorio */
		dir[0]=CG_CHRDIR1;
		strcpy(&dir[1],find.ff_name);
		i=strlen(dir);
		dir[i]=CG_CHRDIR2;
		dir[i+1]='\0';

		/* mete nombre de subdirectorio en lista */
		if(cg_mete_en_lista(lista,dir)==0) {
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
	if(cg_mete_en_lista(lista,find.ff_name)==0) {
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
	  Salida:	n£mero de elemento seleccionado, -1 si ninguno
****************************************************************************/
int busca_elemento_tecla(STC_CUADROG *c, char tecla)
{
STC_ELEMG *elem;
int i;
char tecla_elem;

if(!tecla) return(-1);

for(elem=c->elem1, i=0; elem!=NULL; elem=elem->sgte, i++) {
	if((elem->tipo!=CG_ELEM_NULO) && (elem->tipo!=CG_ELEM_TEXTO)) {
		switch(elem->tipo) {
		    case CG_ELEM_BOTON :
			tecla_elem=((STC_ELEM_BOTONG *)elem->info)->tecla;
			break;
		    case CG_ELEM_INPUT :
			tecla_elem=((STC_ELEM_INPUTG *)elem->info)->tecla;
			break;
		    case CG_ELEM_LISTA :
			tecla_elem=((STC_ELEM_LISTAG *)elem->info)->tecla;
			break;
		    case CG_ELEM_CHECK :
			tecla_elem=((STC_ELEM_CHECKG *)elem->info)->tecla;
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
int busca_elemento_raton(STC_CUADROG *c, int fil, int col)
{
STC_ELEMG *elem;
int i, fil0, col0, fil1, col1;

for(elem=c->elem1, i=0; elem!=NULL; elem=elem->sgte, i++) {
	if((elem->tipo!=CG_ELEM_NULO) && (elem->tipo!=CG_ELEM_TEXTO)) {
		switch(elem->tipo) {
		    case CG_ELEM_BOTON :
			fil0=elem->fil+c->v.fil+1;
			col0=elem->col+c->v.col+1;
			fil1=fil0;
			col1=col0+((STC_ELEM_BOTONG *)elem->info)->ancho-1;
			break;
		    case CG_ELEM_INPUT :
			fil0=elem->fil+c->v.fil+1;
			col0=elem->col+c->v.col+1;
			fil1=fil0;
			col1=col0+((STC_ELEM_INPUTG *)elem->info)->ancho-1;
			break;
		    case CG_ELEM_LISTA :
			fil0=((STC_ELEM_LISTAG *)elem->info)->v.fil;
			col0=((STC_ELEM_LISTAG *)elem->info)->v.col;
			fil1=fil0+((STC_ELEM_LISTAG *)elem->info)->v.alto-1;
			col1=col0+((STC_ELEM_LISTAG *)elem->info)->v.ancho-1;
			/* tenemos en cuenta la anchura de las flechas de */
			/* desplazamiento */
			if(chr_altura()!=8) col1++;
			break;
		    case CG_ELEM_CHECK :
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
void inicializa_elem_boton(STC_ELEM_BOTONG *e, int ancho, char *texto)
{
char *txt;

e->ancho=ancho;
e->texto=texto;

e->tecla=0;
for(txt=texto; *txt; txt++) {
	if(*txt==CG_CARTECLA) {
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
void inicializa_elem_input(STC_ELEM_INPUTG *e, int ancho, char *texto,
  char *buff, int longitud)
{
char *txt;

e->ancho=ancho;
e->texto=texto;
e->cadena=buff;
e->longitud=longitud;

e->tecla=0;
for(txt=texto; *txt; txt++) {
	if(*txt==CG_CARTECLA) {
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
			'clr_fondo', 'clr_pplano' colores principales
			'clr_s1', 'clr_s2' colores de sombra
			'modo' CG_LSTNORMAL si normal, CG_LSTPRIMERO si
			elemento seleccionado siempre aparece primero
			'orden' CG_LSTSINORDEN si lista desordenada,
			CG_LSTORDENADA si lista ordenada
****************************************************************************/
void inicializa_elem_lista(STC_ELEM_LISTAG *e, int fil, int col,
  int ancho, int alto, char *texto, BYTE clr_fondo, BYTE clr_pplano,
  BYTE clr_s1, BYTE clr_s2, int modo, int orden)
{
char *txt;

vg_crea(&e->v,fil,col,ancho,alto,clr_fondo,clr_pplano,clr_s1,clr_s2,NULL);

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
	if(*txt==CG_CARTECLA) {
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
void inicializa_elem_check(STC_ELEM_CHECKG *e, char *texto)
{
char *txt;

e->texto=texto;
e->estado=0;

e->tecla=0;
for(txt=texto; *txt; txt++) {
	if(*txt==CG_CARTECLA) {
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
			'clr_fondo', 'clr_pplano' colores principal
			'clr_s1', 'clr_s2' colores de sombra
			'modo' CG_TXTLINEA si imprime cada l¡nea por separado
			CG_TXTPASA si pasa texto a siguiente l¡nea si no cabe
			'borde' CG_TXTNOBORDE si no imprime borde,
			CG_TXTBORDE si lo imprime
****************************************************************************/
void inicializa_elem_texto(STC_ELEM_TEXTOG *e, char *texto, int fil,
  int col, int ancho, int alto, BYTE clr_fondo, BYTE clr_pplano, BYTE clr_s1,
  BYTE clr_s2, int modo, int borde)
{

vg_crea(&e->v,fil,col,ancho,alto,clr_fondo,clr_pplano,clr_s1,clr_s2,NULL);

e->texto=texto;
e->modo=modo;
e->borde=borde;

if(modo!=CG_TXTLINEA) vg_modo_texto(&e->v,VG_PASA_LINEA);

}

/****************************************************************************
	CG_CREA_CUADRO: crea un cuadro de di logo.
	  Entrada:      'cuad' puntero a estructura del cuadro a crear
			'titulo' texto de encabezamiento (NULL si ninguno)
			'fil', 'col' posici¢n del cuadro; si 'fil' es igual
			a CG_CENT se centra el cuadro verticalmente, si 'col'
			es igual a CG_CENT se centra horizontalmente
			'ancho', 'alto' tama¤o del cuadro
			'clr_fondo', 'clr_pplano' colores principales del
			cuadro
			'clr_s1', 'clr_s2' colores para sombra
			'clr_teclaf', 'clr_teclap' colores para teclas de
			activaci¢n
****************************************************************************/
void cg_crea_cuadro(STC_CUADROG *cuad, char *titulo, int fil, int col,
  int ancho, int alto, BYTE clr_fondo, BYTE clr_pplano, BYTE clr_s1,
  BYTE clr_s2, BYTE clr_teclaf, BYTE clr_teclap)
{

/* comprueba si hay que centrar el cuadro */
if(fil==CG_CENT) fil=(chr_maxfil()-alto)/2;
if(col==CG_CENT) col=(chr_maxcol()-ancho)/2;

vg_crea(&cuad->v,fil,col,ancho,alto,clr_fondo,clr_pplano,clr_s1,clr_s2,titulo);
cuad->clr_teclaf=clr_teclaf;
cuad->clr_teclap=clr_teclap;

/* inicializa elementos del cuadro */
cuad->elem1=NULL;
cuad->elemento=NULL;

}

/****************************************************************************
	CG_CREA_ELEMENTO: crea o modifica un elemento de un cuadro.
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
STC_ELEMG *cg_crea_elemento(STC_CUADROG *cuad, int tipo, int fil, int col,
  ...)
{
STC_ELEMG *e, *elem;
va_list par;
void *info;
char *texto, *buff;
int fil_elem, col_elem, ancho, alto, longitud, modo, orden, borde;

va_start(par,col);

/* calcula posici¢n del elemento en pantalla */
fil_elem=cuad->v.fil+fil+1;
col_elem=cuad->v.col+col+1;

e=(STC_ELEMG *)malloc(sizeof(STC_ELEMG));
if(e==NULL) return(NULL);

e->tipo=tipo;
e->fil=fil;
e->col=col;

/* crea elemento seg£n tipo */
switch(tipo) {
	case CG_ELEM_BOTON :
		info=(STC_ELEM_BOTONG *)malloc(sizeof(STC_ELEM_BOTONG));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		ancho=va_arg(par,int);
		inicializa_elem_boton((STC_ELEM_BOTONG *)info,ancho,texto);
		break;
	case CG_ELEM_INPUT :
		info=(STC_ELEM_INPUTG *)malloc(sizeof(STC_ELEM_INPUTG));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		ancho=va_arg(par,int);
		buff=va_arg(par,char *);
		longitud=va_arg(par,int);
		inicializa_elem_input((STC_ELEM_INPUTG *)info,ancho,texto,buff,
		  longitud);
		break;
	case CG_ELEM_LISTA :
		info=(STC_ELEM_LISTAG *)malloc(sizeof(STC_ELEM_LISTAG));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		ancho=va_arg(par,int);
		alto=va_arg(par,int);
		modo=va_arg(par,int);
		orden=va_arg(par,int);
		inicializa_elem_lista((STC_ELEM_LISTAG *)info,fil_elem,
		  col_elem,ancho,alto,texto,cuad->v.clr_fondo,
		  cuad->v.clr_pplano,cuad->v.clr_s2,cuad->v.clr_s1,modo,orden);
		break;
	case CG_ELEM_CHECK :
		info=(STC_ELEM_CHECKG *)malloc(sizeof(STC_ELEM_CHECKG));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		inicializa_elem_check((STC_ELEM_CHECKG *)info,texto);
		break;
	case CG_ELEM_TEXTO :
		info=(STC_ELEM_TEXTOG *)malloc(sizeof(STC_ELEM_TEXTOG));
		if(info==NULL) break;
		e->info=info;
		texto=va_arg(par,char *);
		ancho=va_arg(par,int);
		alto=va_arg(par,int);
		modo=va_arg(par,int);
		borde=va_arg(par,int);
		inicializa_elem_texto((STC_ELEM_TEXTOG *)info,texto,fil_elem,
		  col_elem,ancho,alto,cuad->v.clr_fondo,cuad->v.clr_pplano,
		  cuad->v.clr_s2,cuad->v.clr_s1,modo,borde);
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
	CG_DIBUJA_ELEMENTO: dibuja un elemento de un cuadro.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
			'e' puntero al elemento
	  Salida:       1 si pudo dibujarlo, 0 si no
****************************************************************************/
int cg_dibuja_elemento(STC_CUADROG *cuad, STC_ELEMG *e)
{
STC_ELEM_BOTONG *boton;
STC_ELEM_INPUTG *input;
STC_ELEM_LISTAG *lista;
STC_ELEM_CHECKG *check;
STC_ELEM_TEXTOG *texto;
char *txt;
int seleccionado, fil, col, x0, y0, x1, y1, chralt, inccol;

rg_puntero(RG_OCULTA);

/* posici¢n del elemento en pantalla */
fil=e->fil+cuad->v.fil+1;
col=e->col+cuad->v.col+1;
chralt=chr_altura();
x0=col*8;
y0=fil*chralt;

/* comprueba si es el elemento seleccionado */
if(e==cuad->elemento) seleccionado=1;
else seleccionado=0;

switch(e->tipo) {
	case CG_ELEM_NULO :
		break;
	case CG_ELEM_BOTON :
		boton=(STC_ELEM_BOTONG *)e->info;
		txt=boton->texto;
		x1=x0+(boton->ancho*8)-1;
		y0-=4;
		y1=y0+chralt+5;
		dibuja_rec_elem(x0+4,y0,x1-4,y1,cuad->v.clr_s1,cuad->v.clr_s2,
		  cuad->v.clr_fondo,1,seleccionado);
		imprime_txt_elem(fil,col+((boton->ancho-strlen(txt)+1)/2),txt,
		  strlen(txt),cuad->v.clr_fondo,cuad->v.clr_pplano,
		  cuad->clr_teclaf,cuad->clr_teclap,1);
		break;
	case CG_ELEM_INPUT :
		input=(STC_ELEM_INPUTG *)e->info;
		txt=input->texto;
		x1=x0+(input->ancho*8)-1;
		y0-=4;
		y1=y0+chralt+5;
		dibuja_rec_elem(x0,y0,x1,y1,cuad->v.clr_s2,cuad->v.clr_s1,
		  cuad->v.clr_fondo,1,seleccionado);
		imprime_txt_elem(fil,col-strlen(txt),txt,strlen(txt),
		  cuad->v.clr_fondo,cuad->v.clr_pplano,cuad->clr_teclaf,
		  cuad->clr_teclap,1);
		imprime_txt_elem(fil,col+1,input->cadena,input->ancho-2,
		  cuad->v.clr_fondo,cuad->v.clr_pplano,cuad->clr_teclaf,
		  cuad->clr_teclap,0);
		break;
	case CG_ELEM_LISTA :
		lista=(STC_ELEM_LISTAG *)e->info;
		txt=lista->texto;
		x1=x0+(lista->v.ancho*8)-1;
		y1=y0+(lista->v.alto*chralt)-1;
		dibuja_rec_elem(x0,y0+4,x1,y1-4,cuad->v.clr_s2,cuad->v.clr_s1,
		  cuad->v.clr_fondo,2,seleccionado);
		imprime_txt_elem(fil,col+((lista->v.ancho-strlen(txt))/2),txt,
		  strlen(txt),cuad->v.clr_fondo,cuad->v.clr_pplano,
		  cuad->clr_teclaf,cuad->clr_teclap,1);
		imprime_lista(&lista->v,lista->elemento,lista->elemento_pr,
		  lista->elemento_sel);
		dibuja_flechas_vert(&lista->v,lista->num_elementos,
		  lista->elemento_sel);
		break;
	case CG_ELEM_CHECK :
		check=(STC_ELEM_CHECKG *)e->info;
		txt=check->texto;
		x1=x0+chralt-1;
		y1=y0+chralt-1;
		if(chralt!=8) {
			x0++;
			y0++;
			x1-=2;
			y1-=2;
			inccol=3;
		}
		else inccol=2;
		dibuja_rec_elem(x0,y0,x1,y1,cuad->v.clr_s2,cuad->v.clr_s1,
		  cuad->v.clr_fondo,1,seleccionado);
		imprime_txt_elem(fil,col+inccol,txt,strlen(txt),
		  cuad->v.clr_fondo,cuad->v.clr_pplano,cuad->clr_teclaf,
		  cuad->clr_teclap,1);
		if(check->estado) {
			g_linea(x0+2,y0+2,x1-2,y1-2,cuad->v.clr_pplano,G_NORM);
			g_linea(x1-2,y0+2,x0+2,y1-2,cuad->v.clr_pplano,G_NORM);
		}
		else g_rectangulo(x0+2,y0+2,x1-2,y1-2,cuad->v.clr_fondo,
		  G_NORM,1);
		break;
	case CG_ELEM_TEXTO :
		texto=(STC_ELEM_TEXTOG *)e->info;
		txt=texto->texto;
		x1=x0+(texto->v.ancho*8)-1;
		y1=y0+(texto->v.alto*chralt)-1;
		if(texto->borde) dibuja_rec_elem(x0,y0,x1,y1,cuad->v.clr_s2,
		  cuad->v.clr_s1,cuad->v.clr_fondo,1,seleccionado);
		vg_pon_cursor(&texto->v,0,0);
		vg_impcad(&texto->v,txt,VG_NORELLENA);
		break;
}

rg_puntero(RG_MUESTRA);

return(1);
}

/****************************************************************************
	CG_ABRE: abre un cuadro de di logo.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
****************************************************************************/
void cg_abre(STC_CUADROG *cuad)
{
STC_ELEMG *elem;

vg_abre(&cuad->v);
for(elem=cuad->elem1; elem!=NULL; elem=elem->sgte) cg_dibuja_elemento(cuad,
  elem);

}

/****************************************************************************
	CG_CIERRA: cierra un cuadro de di logo.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
****************************************************************************/
void cg_cierra(STC_CUADROG *cuad)
{

vg_cierra(&cuad->v);

}

/****************************************************************************
	CG_ELIMINA: elimina un cuadro de di logo y todos sus elementos
 	  asociados liberando la memoria ocupada.
	  Entrada:      'cuad' puntero a la estructura con la informaci¢n
			del cuadro
****************************************************************************/
void cg_elimina(STC_CUADROG *cuad)
{
STC_ELEMG *e, *e1;

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
	CG_LEE_INPUT: lee una cadena de caracteres por teclado o edita una
	  cadena ya existente.
	  Entrada:      'fil', 'col' posici¢n de la zona de 'input'
			'ancho' anchura de la zona de 'input'
			'colorf', 'colorp' color para zona de 'input'
			'cadena' puntero a buffer d¢nde se guardar 
			la cadena tecleada; si el primer car cter no es '\0'
			se ajustar  la rutina para poder editar la cadena
			'longitud' longitud de la cadena (sin contar el
			\0 final)
	  Salida:       n£mero de caracteres tecleados
			-1 si se puls¢ TAB
			-2 si puls¢n con rat¢n fuera de la ventana
			-3 si se puls¢ ESCAPE
			-4 si se puls¢ ALT+tecla (variable global 'alt_tecla'
			   contendr  c¢digo ASCII de tecla pulsada)
****************************************************************************/
int cg_lee_input(int fil, int col, int ancho, BYTE colorf, BYTE colorp,
  char *cadena, int longitud)
{
STC_RATONG r;
char *cur, *fin, *ptr;
int num_car=0, ccur, tecla, shift, ascii, maxcol, chralt, cx, cy;

/* busca final de la cadena y n£mero de caracteres */
for(fin=cadena; *fin; fin++, num_car++);

/* inicializa posici¢n del cursor */
cur=fin;
if(num_car<ancho) ccur=num_car;
else ccur=ancho-1;

/* coge altura de los caracteres */
chralt=chr_altura();

do {
	cx=(col+ccur)*8;
	cy=fil*chralt;

	/* imprime l¡nea */
	imprime_txt_elem(fil,col,cur-ccur,ancho,colorf,colorp,colorf,colorp,0);

	/* imprime cursor */
	rg_puntero(RG_OCULTA);
	g_linea(cx,cy,cx,cy+chralt-1,colorp,G_NORM);
	rg_puntero(RG_MUESTRA);

	do {
		/* recoge posici¢n del rat¢n */
		rg_estado(&r);
		if(r.boton1) {
			/* comprueba si se puls¢ fuera de la ventana */
			/* y si es as¡ sale */
			maxcol=col+ancho-1;
			if((r.fil!=fil) || (r.col<col) || (r.col>maxcol)) {
				/* imprime l¡nea */
				imprime_txt_elem(fil,col,cadena,ancho,colorf,
				  colorp,colorf,colorp,0);
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
		imprime_txt_elem(fil,col,cadena,ancho,colorf,colorp,colorf,
		  colorp,0);
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
				/* borra cursor */
				rg_puntero(RG_OCULTA);
				g_linea(cx,cy,cx,cy+chralt-1,colorf,G_NORM);
				rg_puntero(RG_MUESTRA);

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
			imprime_txt_elem(fil,col,cadena,ancho,colorf,colorp,
			  colorf,colorp,0);
			return(-1);
		case 0x01 :             /* ESCAPE */
			/* imprime l¡nea */
			imprime_txt_elem(fil,col,cadena,ancho,colorf,colorp,
			  colorf,colorp,0);
			return(-3);
	}

} while(tecla!=0x1c0d);	/* hasta que pulse RETURN */

/* imprime l¡nea */
imprime_txt_elem(fil,col,cadena,ancho,colorf,colorp,colorf,colorp,0);

return(num_car);
}

/****************************************************************************
	CG_ACCION: env¡a una acci¢n a un cuadro de di logo.
	  Entrada:      'c' puntero a cuadro de di logo
			'acc' puntero a estructura con la acci¢n sobre el
			cuadro
	  Salida:       elemento seleccionado o c¢digo de acci¢n (si <0)
****************************************************************************/
int cg_accion(STC_CUADROG *c, STC_ACCION_CG *acc)
{
STC_RATONG r;
STC_ELEMG *e;
int i;
char tecla;

switch(acc->accion) {
	case CUADROG_NULA :             /* acci¢n nula */
		break;
	case CUADROG_SALIDA :           /* salir del cuadro */
		cg_cierra(c);
		cg_elimina(c);
		break;
	case CUADROG_SGTE_ELEM :        /* siguiente elemento */
		siguiente_elemento(c);
		resalta_elemento(c);
		return(CUADROG_NULA);
	case CUADROG_ANT_ELEM :         /* elemento anterior */
		anterior_elemento(c);
		resalta_elemento(c);
		return(CUADROG_NULA);
	case CUADROG_SELECC :
		for(i=0, e=c->elem1; (e!=NULL) && (e!=c->elemento);
		  e=e->sgte, i++);
		return(i);
	case CUADROG_TECLA :
		/* si estaba pulsado ALT convierte c¢digo 'scan' a ASCII */
		if(acc->shift & 0x0008) tecla=scan_ascii(acc->tecla);
		else tecla=mayuscula(acc->tecla & 0x00ff);

		/* busca elemento correspondiente a tecla pulsada */
		i=busca_elemento_tecla(c,tecla);
		if(i==-1) return(CUADROG_NULA);
		else return(i);
	case CUADROG_RATON :
		do {
			i=busca_elemento_raton(c,acc->fil,acc->col);
			rg_estado(&r);
			acc->fil=r.fil;
			acc->col=r.col;
		} while(r.boton1);
		if(i==CG_MAXELEM) return(CUADROG_NULA);
		else return(i);
}

return(acc->accion);
}

/****************************************************************************
	CG_GESTIONA: gestiona un cuadro de di logo.
	  Entrada:      'c' puntero a cuadro de di logo
	  Salida:       £ltimo elemento seleccionado, -1 si se sali¢ del
			cuadro
****************************************************************************/
int cg_gestiona(STC_CUADROG *c)
{
STC_ACCION_CG acc;
STC_RATONG r;
int i, accion, elemento, shift, tecla;

/* si rat¢n no est  inicializado, lo inicializa */
if(!rg_puntero(RG_MUESTRA)) rg_inicializa();

c->elemento=c->elem1;
resalta_elemento(c);
acc.accion=CUADROG_NULA;

while(1) {
	/* si elemento actual es de entrada o lista, lo selecciona */
	if(((c->elemento)->tipo==CG_ELEM_INPUT) ||
	  ((c->elemento)->tipo==CG_ELEM_LISTA)) {
		acc.accion=CUADROG_SELECC;
		accion=3;
	}
	else do {
		accion=0;
		rg_estado(&r);
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
		acc.accion=CUADROG_RATON;
		acc.fil=r.fil;
		acc.col=r.col;
	}
	else if(accion==2) switch((tecla >> 8) & 0x00ff) {
		case 0x01 :     /* ESCAPE */
			acc.accion=CUADROG_SALIDA;
			break;
		case 0x1c :     /* RETURN */
			acc.accion=CUADROG_SELECC;
			break;
		case 0x0f :     /* TAB */
			if(shift & 0x0003) acc.accion=CUADROG_ANT_ELEM;
			else acc.accion=CUADROG_SGTE_ELEM;
			break;
		default :
			acc.accion=CUADROG_TECLA;
			acc.tecla=tecla;
			acc.shift=shift;
			break;
	}

	elemento=cg_accion(c,&acc);

	/* ejecuta acciones seg£n el tipo de elemento */
	if(elemento>=0) {
		switch((c->elemento)->tipo) {
			case CG_ELEM_NULO :
				break;
			case CG_ELEM_BOTON :
				return(elemento);
			case CG_ELEM_INPUT :
				i=cg_elem_input(c->v.fil+(c->elemento)->fil+1,
				  c->v.col+(c->elemento)->col+1,c->v.clr_fondo,
				  c->v.clr_pplano,
				  (STC_ELEM_INPUTG *)(c->elemento)->info);
				/* si puls¢ TAB */
				if(i==-1) {
					shift=bioskey(2);
					if(shift & 0x0003) anterior_elemento(c);
					else siguiente_elemento(c);
					resalta_elemento(c);
				}
				/* si puls¢ con rat¢n fuera del elemento */
				else if(i==-2) {
					rg_estado(&r);
					busca_elemento_raton(c,r.fil,r.col);
				}
				/* si puls¢ ESC */
				else if(i==-3) return(-1);
				/* si puls¢ ALT+tecla */
				else if(i==-4) busca_elemento_tecla(c,
				  alt_tecla);
				break;
			case CG_ELEM_LISTA :
				i=cg_elem_lista(
				  (STC_ELEM_LISTAG *)(c->elemento)->info);
				/* si puls¢ TAB */
				if(i==-1) {
					shift=bioskey(2);
					if(shift & 0x0003) anterior_elemento(c);
					else siguiente_elemento(c);
					resalta_elemento(c);
				}
				/* si puls¢ con rat¢n fuera del elemento */
				else if(i==-2) {
					rg_estado(&r);
					busca_elemento_raton(c,r.fil,r.col);
				}
				/* si puls¢ ESC */
				else if(i==-3) return(-1);
				/* si lista est  vac¡a */
				else if(i==-4) {
					siguiente_elemento(c);
					resalta_elemento(c);
				}
				/* si se puls¢ ALT+tecla */
				else if(i==-5) busca_elemento_tecla(c,
				  alt_tecla);
				break;
			case CG_ELEM_CHECK :
				cg_elem_check(
				  (STC_ELEM_CHECKG *)(c->elemento)->info,c,
				  c->elemento);
				break;
			case CG_ELEM_TEXTO :
				break;
		}
	}
	else if(elemento==CUADROG_SALIDA) return(-1);

}

}

/****************************************************************************
	CG_ELEM_INPUT: gestiona un elemento de entrada.
	  Entrada:      'fil', 'col' posici¢n del elemento en pantalla
			'colorf', 'colorp' color de zona de entrada
			'e' puntero a informaci¢n de elemento de entrada
	  Salida:       n£mero de caracteres tecleados.
			-1 si se puls¢ TAB
			-2 si puls¢n con rat¢n fuera de la ventada
			-3 si se puls¢ ESCAPE
			-4 si se puls¢ ALT+tecla (variable global 'alt_tecla'
			   contendr  c¢digo de tecla pulsada)
****************************************************************************/
int cg_elem_input(int fil, int col, BYTE colorf, BYTE colorp,
  STC_ELEM_INPUTG *e)
{
int i;

i=cg_lee_input(fil,col+1,e->ancho-2,colorf,colorp,e->cadena,e->longitud);

return(i);
}

/****************************************************************************
	CG_ELEM_LISTA: gestiona un elemento de lista.
	  Entrada:      'e' puntero a informaci¢n de elemento de lista
	  Salida:       n£mero de elemento seleccionado
			-1 si se puls¢ TAB
			-2 si puls¢ con rat¢n fuera de la ventana
			-3 si se puls¢ ESCAPE
			-4 si la lista est  vac¡a
			-5 si se puls¢ ALT+tecla (variable global 'alt_tecla'
			   contendr  c¢digo de tecla pulsada)
****************************************************************************/
int cg_elem_lista(STC_ELEM_LISTAG *e)
{
STC_RATONG r;
STC_VENTANAG *v;
int elemento_ant=-1, tecla, shift, maxfil, maxcol, fil, col, xf, chralt;

/* si la lista est  vac¡a, sale */
if(!e->num_elementos) return(-4);

/* puntero a ventana de lista */
v=&e->v;
chralt=chr_altura();

/* busca elemento seleccionado */
busca_elem_selecc(e);

while(1) {
	if(e->elemento_sel!=elemento_ant) imprime_lista(v,e->elemento,
	  e->elemento_pr,e->elemento_sel);
	elemento_ant=e->elemento_sel;

	do {
		tecla=0;
		/* recoge posici¢n del rat¢n */
		rg_estado(&r);
		if(r.boton1) {
			/* comprueba si se puls¢ fuera de la ventana */
			/* y si es as¡ sale */
			maxfil=v->fil+v->alto-1;
			maxcol=v->col+v->ancho-1;
			if((r.fil<v->fil) || (r.fil>maxfil) ||
			  (r.col<v->col) || (r.col>(maxcol+1))) {
				imprime_lista(v,e->elemento,e->elemento_pr,
				  e->elemento_sel);
				dibuja_flechas_vert(v,e->num_elementos,
				  e->elemento_sel);
				busca_elem_selecc(e);
				return(-2);
			}
			else {
				/* calcula coordenadas relativas al */
				/* origen de la ventana */
				xf=(v->col+v->ancho-1)*8;
				fil=r.fil-v->fil;
				col=r.col-v->col;

				if((fil==0) && (r.x>=xf) &&
				  (r.x<=(xf+chralt-1))) tecla=0x4700;
				else if((fil==(v->alto-1)) && (r.x>=xf) &&
				  (r.x<=(xf+chralt-1))) tecla=0x4f00;
				else if((fil==1) && (r.x>=xf) &&
				  (r.x<=(xf+chralt-1))) tecla=0x4800;
				else if((fil==(v->alto-2)) && (r.x>=xf) &&
				  (r.x<=(xf+chralt-1))) tecla=0x5000;
				else if((fil==0) && (col!=0) &&
				  (col!=(v->ancho-1))) tecla=0x4800;
				else if((fil==(v->alto-1)) && (col!=0) &&
				  (r.x<xf)) tecla=0x5000;
				else if((fil<(v->alto/2)) && (r.x>=xf) &&
				  (r.x<=(xf+chralt-1))) tecla=0x4900;
				else if((fil>=(v->alto/2)) && (r.x>=xf) &&
				  (r.x<=(xf+chralt-1))) tecla=0x5100;
				else if((fil>0) && (fil<(v->alto-1)) &&
				  (col>0) && (r.x<xf)) {
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
						  e->elemento_sel);
						dibuja_flechas_vert(v,
						  e->num_elementos,
						  e->elemento_sel);
					}
					else {
						rg_estado(&r);
						if(r.boton1) break;
						imprime_lista(v,e->elemento,
						  e->elemento_pr,
						  e->elemento_sel);
						dibuja_flechas_vert(v,
						  e->num_elementos,
						  e->elemento_sel);
						busca_elem_selecc(e);
						return(e->elemento_sel);
					}
				}
			}

			delay(CG_PAUSA);
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
		imprime_lista(v,e->elemento,e->elemento_pr,e->elemento_sel);
		dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
		busca_elem_selecc(e);
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
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			break;
		case 0x50 :             /* cursor abajo */
			if(e->elemento_sel<(e->num_elementos-1))
			  e->elemento_sel++;
			if(e->modo==0) {
				if(e->elemento_sel>(e->elemento_pr+v->alto-3))
				  e->elemento_pr++;
			}
			else e->elemento_pr=e->elemento_sel;
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			break;
		case 0x49 :             /* p gina arriba */
			if(e->elemento_pr==0) break;
			e->elemento_pr-=v->alto-2;
			if(e->elemento_pr<0) e->elemento_pr=0;
			e->elemento_sel=e->elemento_pr;
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel);
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			break;
		case 0x51 :             /* p gina abajo */
			if(e->elemento_pr==(e->num_elementos-1)) break;
			e->elemento_pr+=v->alto-2;
			if(e->elemento_pr>(e->num_elementos-1))
			  e->elemento_pr=e->num_elementos-1;
			e->elemento_sel=e->elemento_pr;
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel);
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			break;
		case 0x47 :             /* cursor origen */
			e->elemento_pr=0;
			e->elemento_sel=0;
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			break;
		case 0x4f :             /* cursor fin */
			e->elemento_pr=e->num_elementos-(v->alto-2);
			if(e->elemento_pr<0) e->elemento_pr=0;
			e->elemento_sel=e->num_elementos-1;
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			break;
		case 0x1c :             /* RETURN */
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel);
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			busca_elem_selecc(e);
			return(e->elemento_sel);
		case 0x0f :             /* TAB */
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel);
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			busca_elem_selecc(e);
			return(-1);
		case 0x01 :             /* ESCAPE */
			imprime_lista(v,e->elemento,e->elemento_pr,
			  e->elemento_sel);
			dibuja_flechas_vert(v,e->num_elementos,e->elemento_sel);
			e->selecc=NULL;
			return(-3);
	}
}

}

/****************************************************************************
	CG_METE_EN_LISTA: inserta una cadena al final de la lista.
	  Entrada:      'e' puntero a la estructura con la informaci¢n de
			la lista
			'cadena' puntero a cadena de caracteres a a¤adir
			al final de la lista
	  Salida:       1 si se pudo crear y a¤adir elemento a lista, si
			no devuelve 0
****************************************************************************/
int cg_mete_en_lista(STC_ELEM_LISTAG *e, char *cadena)
{
STC_LISTAG *l_ultimo, *l_anterior, *l;

/* crea nuevo elemento */
l=(STC_LISTAG *)malloc(sizeof(STC_LISTAG));
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
if(e->orden==CG_LSTSINORDEN) {
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
	CG_BORRA_LISTA: elimina todos los elementos de una lista, liberando
	  la memoria ocupada.
	  Entrada:      'e' puntero a la estructura con la informaci¢n de
			la lista
****************************************************************************/
void cg_borra_lista(STC_ELEM_LISTAG *e)
{
STC_LISTAG *l, *l_sgte;

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
	CG_ELEM_CHECK: gestiona un elemento de caja de comprobaci¢n.
	  Entrada:      'e' puntero a informaci¢n de elemento de entrada
			'cuad' puntero a cuadro al que pertenece
			'elem' puntero elemento dentro del cuadro
	  Salida:       estado; 0 desactivada, 1 activada
****************************************************************************/
int cg_elem_check(STC_ELEM_CHECKG *e, STC_CUADROG *cuad, STC_ELEMG *elem)
{

if(e->estado==0) e->estado=1;
else e->estado=0;

cg_dibuja_elemento(cuad,elem);

return(e->estado);
}

/****************************************************************************
	CG_SELECC_FICHEROS: gestiona un cuadro de selecci¢n de ficheros.
	  Entrada:      'fil', 'col' posici¢n del cuadro; si 'fil' o 'col'
			son iguales a CG_CENT se centrar  el cuadro
			'titulo' t¡tulo del cuadro
			'clr_fondo', 'clr_pplano' colores principales
			'clr_s1', 'clr_s2' colores de sombreado
			'clr_teclaf', 'clr_teclap' colores de teclas de
			activaci¢n
			'ruta' ruta completa al directorio; si se da una
			cadena vac¡a se coge la ruta al directorio actual
			'mascara' m scara de b£squeda de ficheros
			'fichero' puntero a buffer donde se dejar  la
			ruta completa y el nombre del fichero seleccionado o
			una cadena vac¡a si no seleccion¢ ning£n fichero; el
			buffer debe ser de una longitud de MAXPATH bytes
****************************************************************************/
void cg_selecc_ficheros(int fil, int col, char *titulo, BYTE clr_fondo,
  BYTE clr_pplano, BYTE clr_s1, BYTE clr_s2, BYTE clr_teclaf, BYTE clr_teclap,
  char *ruta, char *mascara, char *fichero)
{
STC_CUADROG cfich;
STC_ACCION_CG acc;
STC_RATONG r;
STC_ELEMG *elista, *einput;
STC_ELEM_LISTAG *lista_fich;
char *c, nfich[MAXPATH], nfich2[MAXPATH], disq[MAXDRIVE], direct[MAXDIR],
  fich[MAXFILE], ext[MAXEXT], dir_orig[MAXPATH], dir[15];
int i, elemento, accion, unid_orig, shift, tecla;

/* guarda directorio y unidad actuales */
getcwd(dir_orig,MAXPATH);
unid_orig=getdisk();

/* si rat¢n no est  inicializado, lo inicializa */
if(!rg_puntero(RG_MUESTRA)) rg_inicializa();

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
cg_crea_cuadro(&cfich,titulo,fil,col,CG_FICH_ANCHO,CG_FICH_ALTO,clr_fondo,
  clr_pplano,clr_s1,clr_s2,clr_teclaf,clr_teclap);
einput=cg_crea_elemento(&cfich,CG_ELEM_INPUT,1,9,"^Fichero",CG_FICH_ANCHO-12,
  nfich,MAXPATH-1);
elista=cg_crea_elemento(&cfich,CG_ELEM_LISTA,3,0," ^Lista ",CG_FICH_ANCHO-19,
  CG_FICH_ALTO-5,CG_LSTNORMAL,CG_LSTORDENADA);
lista_fich=(STC_ELEM_LISTAG *)elista->info;
cg_crea_elemento(&cfich,CG_ELEM_BOTON,CG_FICH_ALTO-6,CG_FICH_ANCHO-15,
  "^Vale",11);
cg_crea_elemento(&cfich,CG_ELEM_BOTON,CG_FICH_ALTO-4,CG_FICH_ANCHO-15,
  "^Salir",11);

cfich.elemento=cfich.elem1;
lista_ficheros(lista_fich,nfich);
cg_abre(&cfich);
resalta_elemento(&cfich);
acc.accion=CUADROG_NULA;

while(1) {
	/* si elemento actual es de entrada o lista, lo selecciona */
	if(((cfich.elemento)->tipo==CG_ELEM_INPUT) ||
	  ((cfich.elemento)->tipo==CG_ELEM_LISTA)) {
		acc.accion=CUADROG_SELECC;
		accion=3;
	}
	else do {
		accion=0;
		rg_estado(&r);
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
		acc.accion=CUADROG_RATON;
		acc.fil=r.fil;
		acc.col=r.col;
	}
	else if(accion==2) switch((tecla >> 8) & 0x00ff) {
		case 0x01 :     /* ESCAPE */
			acc.accion=CUADROG_SALIDA;
			break;
		case 0x1c :     /* RETURN */
			acc.accion=CUADROG_SELECC;
			break;
		case 0x0f :     /* TAB */
			if(shift & 0x0003) acc.accion=CUADROG_ANT_ELEM;
			else acc.accion=CUADROG_SGTE_ELEM;
			break;
		default :
			acc.accion=CUADROG_TECLA;
			acc.tecla=tecla;
			acc.shift=shift;
			break;
	}

	elemento=cg_accion(&cfich,&acc);

	/* ejecuta acciones seg£n el elemento seleccionado */
	switch(elemento) {
		case 0 :        /* ventana introducci¢n nombre fichero */
			i=cg_elem_input(cfich.v.fil+(cfich.elemento)->fil+1,
			  cfich.v.col+(cfich.elemento)->col+1,
			  cfich.v.clr_fondo,cfich.v.clr_pplano,
			  (cfich.elemento)->info);
			lista_ficheros(lista_fich,nfich);
			cg_dibuja_elemento(&cfich,elista);
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
				rg_estado(&r);
				busca_elemento_raton(&cfich,r.fil,r.col);
			}
			/* si puls¢ ESC */
			else if(i==-3) {
				cg_cierra(&cfich);
				cg_elimina(&cfich);
				cg_borra_lista(lista_fich);
				*fichero='\0';
				/* restaura directorio y unidad de origen */
				setdisk(unid_orig);
				chdir(dir_orig);
				return;
			}
			/* si puls¢ ALT+tecla */
			else if(i==-4) busca_elemento_tecla(&cfich,alt_tecla);
			break;
		case 1 :        /* lista de ficheros */
			i=cg_elem_lista((STC_ELEM_LISTAG *)(cfich.elemento)->info);
			/* si puls¢ TAB */
			if(i==-1) {
				shift=bioskey(2);
				if(shift & 0x0003) anterior_elemento(&cfich);
				else siguiente_elemento(&cfich);
				resalta_elemento(&cfich);
			}
			/* si puls¢ con rat¢n fuera del elemento */
			else if(i==-2) {
				rg_estado(&r);
				busca_elemento_raton(&cfich,r.fil,r.col);
			}
			/* si puls¢ ESC */
			else if(i==-3) {
				cg_cierra(&cfich);
				cg_elimina(&cfich);
				cg_borra_lista(lista_fich);
				*fichero='\0';
				/* restaura directorio y unidad de origen */
				setdisk(unid_orig);
				chdir(dir_orig);
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
				if(*lista_fich->selecc==CG_CHRDIR1) {
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
					cg_dibuja_elemento(&cfich,einput);

					/* crea lista nueva */
					lista_ficheros(lista_fich,nfich);
					cg_dibuja_elemento(&cfich,elista);
				}
				else {
					strcpy(nfich,disq);
					strcat(nfich,direct);
					strcat(nfich,lista_fich->selecc);
					cg_cierra(&cfich);
					cg_elimina(&cfich);
					cg_borra_lista(lista_fich);
					strcpy(fichero,nfich);
					/* restaura directorio y unidad */
					/* de origen */
					setdisk(unid_orig);
					chdir(dir_orig);
					return;
				}
			}
			break;
		case 2 :        /* bot¢n 'Vale' */
			fnsplit(nfich,disq,direct,fich,ext);
			/* si hay elemento seleccionado de lista */
			/* y no es directorio */
			if((lista_fich->selecc!=NULL) &&
			  (*lista_fich->selecc!=CG_CHRDIR1)) {
				strcpy(nfich,disq);
				strcat(nfich,direct);
				strcat(nfich,lista_fich->selecc);
			}
			cg_cierra(&cfich);
			cg_elimina(&cfich);
			cg_borra_lista(lista_fich);
			strcpy(fichero,nfich);
			/* restaura directorio y unidad de origen */
			setdisk(unid_orig);
			chdir(dir_orig);
			return;
		case 3 :        /* bot¢n 'Salir' */
		case CUADROG_SALIDA :
			cg_cierra(&cfich);
			cg_elimina(&cfich);
			cg_borra_lista(lista_fich);
			*fichero='\0';
			/* restaura directorio y unidad de origen */
			setdisk(unid_orig);
			chdir(dir_orig);
			return;
	}
}

}
