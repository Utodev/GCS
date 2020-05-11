/****************************************************************************
                 EDITOR.C

    Biblioteca de funciones para gestionar un editor de textos.

                 (c)1995 JSJ Soft Ltd.

    Las siguientes funciones son p£blicas:
        - e_inicializa: inicializa el editor
        - e_elimina: libera memoria ocupada por el editor
        - e_cambia_color: modifica los colores del editor
        - e_borra_texto: borra texto del editor y libera memoria
        - e_editor: bucle principal del editor
        - e_dibuja_editor: dibuja ventana de edici¢n
        - e_carga_texto: carga un fichero en el editor
        - e_graba_texto: graba el texto del editor en un fichero
        - e_nombre_fichero: devuelve el nombre del fichero
            cargado en el editor
        - e_inicia_busqueda: busca una cadena en el texto
        - e_continua_busqueda: contin£a la b£squeda de una cadena
        - e_modificado: comprueba si el texto del editor ha sido
            modificado
        - e_cambia_modo: cambia modo de edici¢n
        - e_carga_textox: carga texto colocado en mitad de un
            fichero
        - e_vacia_bloque: libera memoria ocupada por bloque de
            texto
        - e_graba_bloque: graba bloque en un fichero
        - e_inserta_bloque: inserta bloque en la posici¢n actual
            del cursor
        - e_pon_cursor: cambia la posici¢n del cursor del editor
        - e_palabra_cursor: devuelve la palabra sobre la que se
            encontraba el cursor la £ltima vez que se puls¢ la tecla
            de ayuda
        - e_cursor_guarda: guarda la posici¢n del cursor del editor
        - e_cursor_recupera: recupera la posici¢n del cursor del
            editor
****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <bios.h>
#include "ventana.h"
#include "raton.h"
#include "cuadro.h"
#include "editor.h"

/*** Variables globales internas ***/
static char carcoment[E_CX];    /* caracteres de inicio de comentario */
static char caresp[E_CX];       /* caracteres especiales */
static char *palclv2[E_PX];     /* palabras clave de 2 letras */
static char *palclv3[E_PX];     /* palabras clave de 3 letras */
static char *palclv4[E_PX];     /* palabras clave de 4 letras */
static char *palclv5[E_PX];     /* palabras clave de 5 letras */
static char *palclv6[E_PX];     /* palabras clave de 6 letras */
static char *palclv7[E_PX];     /* palabras clave de 7 letras */
static char *palclv8[E_PX];     /* palabras clave de 8 letras */
static char *palclv9[E_PX];     /* palabras clave de 9 letras */

static STC_VENTANA ved;         /* ventana de edici¢n */
static STC_CUADRO cbusca;       /* cuadro para b£squedas */
static STC_ELEM_CHECK *chk_may; /* 'check' may£sculas/min£sculas */

static char cadena_busca[E_MAXLNGLIN];

static char *lin[E_MAXNUMLIN];  /* punteros a l¡neas de texto */
static STC_BLOQUE *bloque;      /* puntero a primera l¡nea bloque de texto */
static char palabra_cursor[E_MAXLNGLIN];

/* informaci¢n del estado del editor */
static int lin_ed=0, col_ed=0;  /* posici¢n del cursor en el texto */
static int filcur=0, colcur=0;  /* posici¢n del cursor en la ventana */
static int numlin=0;            /* n§ de l¡neas actual */
static int hdesplz=0;           /* desplazamiento horz. de la ventana */
static int modificado=0;        /* 1 si texto fue modificado, 0 si no */
static int modo_ed=E_EDITA;     /* modo de edici¢n, E_EDITA permite editar */
                                /* E_SOLOLECT s¢lo permite ver */
static char fich0[MAXPATH]="";      /* fichero por defecto */
static char fich_ed[MAXPATH]="";    /* fichero que se est  editando */
static int lin_ib=0, col_ib=0;  /* posici¢n de inicio de bloque */
static int lin_fb=0, col_fb=0;  /* posici¢n final de bloque */
static int mbloque=0;           /* 1 si en modo de bloque, 0 si no */
static BYTE clr_blq=0;          /* color para marcar bloque */
static BYTE clr_coment=0;       /* color de comentarios */
static BYTE clr_palclv=0;       /* color de palabra clave */
static BYTE clr_esp=0;          /* color de caracteres especiales */

/*** Prototipos de funciones internas ***/
static void pon_cursor(int fil, int col);
static void beep(void);
static char mayuscula(char c);
static void conv_mayuscula(char *cad);
static int esta_en_bloque(int lin, int col);
static int es_carcoment(char c);
static int es_caresp(char c);
static int es_palclv(char *pal, int pos);
static void imprime_linea_ed(int l);
static void imprime_texto_ed(void);
static void scroll_der_ed(void);
static void scroll_izq_ed(void);
static void scroll_arr_ed(void);
static void scroll_abj_ed(void);
static void cursor_izq(void);
static void cursor_der(void);
static void cursor_abj(void);
static void cursor_arr(void);
static void cursor_fin_lin(void);
static void cursor_inicio_lin(void);
static void cursor_fin(void);
static void cursor_inicio(void);
static void pagina_arr(void);
static void pagina_abj(void);
static void cursor_pos(int fil, int col);
static int inserta_caracter(char c);
static int borra_caracter(int nlin, int col);
static int inserta_linea(void);
static int borra_linea(int l);
static int copia_linea(int lin1, int lin2);
static void imprime_info(void);
static void ajusta_linea(char *lin);
static void suprime_espacios_fin(int l);
static int busca_cadena(void);
static void marca_bloque(void);
static int copia_bloque(int lini, int coli, int linf, int colf);
static int borra_bloque(int lini, int coli, int linf, int colf);
static int inserta_bloque(int nlin, int col);
static void coge_palabra_cursor(void);

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
    CONV_MAYUSCULA: convierte una cadena en may£scula.
      Entrada:      'cad' cadena a convertir
****************************************************************************/
void conv_mayuscula(char *cad)
{

while(*cad) {
    *cad=mayuscula(*cad);
    cad++;
}

}

/****************************************************************************
    ESTA_EN_BLOQUE: comprueba si un car cter pertenece a un bloque
      marcado.
      Entrada:      'lin', 'col' posici¢n del car cter dentro del
            texto
      Salida:       1 si el car cter est  en el bloque marcado, 0 si no
****************************************************************************/
int esta_en_bloque(int lin, int col)
{

if(lin_ib==lin_fb) {
    if(lin!=lin_ib) return(0);
    if((col_ib<=col_fb) && (col>=col_ib) && (col<=col_fb))
      return(1);
    if((col_ib>col_fb) && (col>=col_fb) && (col<=col_ib))
      return(1);
}
else if(lin_ib<lin_fb) {
    if((lin>lin_ib) && (lin<lin_fb)) return(1);
    else if((lin==lin_ib) && (col>=col_ib)) return(1);
    else if((lin==lin_fb) && (col<=col_fb)) return(1);
}
else {
    if((lin>lin_fb) && (lin<lin_ib)) return(1);
    else if((lin==lin_fb) && (col>=col_fb)) return(1);
    else if((lin==lin_ib) && (col<=col_ib)) return(1);
}

return(0);
}

/****************************************************************************
    ES_CARCOMENT: comprueba si un car cter es de inicio de comentario.
      Entrada:  'c' car cter a comprobar
      Salida:   1 si es inicio de comentario, 0 si no
****************************************************************************/
int es_carcoment(char c)
{
int i=0;

while(carcoment[i]!='\0') if(carcoment[i++]==c) return(1);

return(0);
}

/****************************************************************************
    ES_CARESP: comprueba si un car cter es especial.
      Entrada:  'c' car cter a comprobar
      Salida:   1 si es especial, 0 si no
****************************************************************************/
int es_caresp(char c)
{
int i=0;

while(caresp[i]!='\0') if(caresp[i++]==c) return(1);

return(0);
}

/****************************************************************************
    ES_PALCLV: comprueba si una palabra es una palabra clave.
      Entrada:  'pal' palabra a comparar
            'pos' posici¢n de palabra dentro de la l¡nea
            de texto
      Salida:   1 si es palabra clave, 0 si no
****************************************************************************/
int es_palclv(char *pal, int pos)
{
int i=0;
char palclv[256];

if(pos && !isspace(*(pal-1))) return(0);

while(isalnum(mayuscula(*pal))) palclv[i++]=*pal++;
palclv[i]='\0';

i=0;
switch(strlen(palclv)) {
    case 2 :
        while(palclv2[i]!=NULL) {
            if(!strnicmp(palclv,palclv2[i],2)) return(1);
            i++;
        }
        break;
    case 3 :
        while(palclv3[i]!=NULL) {
            if(!strnicmp(palclv,palclv3[i],3)) return(1);
            i++;
        }
        break;
    case 4 :
        while(palclv4[i]!=NULL) {
            if(!strnicmp(palclv,palclv4[i],4)) return(1);
            i++;
        }
        break;
    case 5 :
        while(palclv5[i]!=NULL) {
            if(!strnicmp(palclv,palclv5[i],5)) return(1);
            i++;
        }
        break;
    case 6 :
        while(palclv6[i]!=NULL) {
            if(!strnicmp(palclv,palclv6[i],6)) return(1);
            i++;
        }
        break;
    case 7 :
        while(palclv7[i]!=NULL) {
            if(!strnicmp(palclv,palclv7[i],7)) return(1);
            i++;
        }
    case 8 :
        while(palclv8[i]!=NULL) {
            if(!strnicmp(palclv,palclv8[i],8)) return(1);
            i++;
        }
    case 9 :
        while(palclv9[i]!=NULL) {
            if(!strnicmp(palclv,palclv9[i],9)) return(1);
            i++;
        }
}

return(0);
}

/****************************************************************************
    IMPRIME_LINEA_ED: imprime una l¡nea en la ventana del editor.
      Entrada:      'l' n£mero de l¡nea a imprimir
****************************************************************************/
void imprime_linea_ed(int l)
{
BYTE color;
int i, fil, col, salta;
char *txt;

if(l>(numlin-1)) return;

r_puntero(R_OCULTA);

/* calcula fila de la l¡nea en la ventana */
if(l<lin_ed) fil=filcur-(lin_ed-l);
else fil=filcur+(l-lin_ed);

/* calcula desplazamiento horizontal de ventana de edici¢n */
hdesplz=col_ed-colcur;

/* imprime s¢lo si desplazamiento no hace que sobrepase el final de l¡nea */
color=ved.clr_princ;
fil+=ved.fil+1;
col=0;
if(hdesplz<(int)strlen(lin[l])) {
    salta=hdesplz;
    txt=lin[l];
    while(*txt && (col<ved.ancho-2)) {
        /* si hay bloque activo comprueba si car cter est  */
            /* dentro del bloque y lo imprime resaltado */
        if(mbloque && esta_en_bloque(l,col+hdesplz)) {
            if(!salta) v_impcar(fil,col+ved.col+1,*txt,clr_blq);
        }
        else {
                /* resalte de sintaxis */
                if(es_carcoment(*txt)) {
                    color=clr_coment;
                    if(!salta) v_impcar(fil,col+ved.col+1,*txt,
                  color);
                }
                else if((color!=clr_coment) && es_caresp(*txt)) {
                    if(!salta) v_impcar(fil,col+ved.col+1,*txt,
                  clr_esp);
                }
                else if((color!=clr_coment) && es_palclv(txt,
                  hdesplz+col)) {
                    color=clr_palclv;
                    while(isalnum(mayuscula(*txt))) {
                        if(!salta) {
                              v_impcar(fil,col+ved.col+1,*txt,
                        color);
                              col++;
                            }
                        else salta--;
                        txt++;
                    }
                    txt--;
                    if(col) col--;
                else salta++;
                    color=ved.clr_princ;
                }
                else if(!salta) v_impcar(fil,col+ved.col+1,*txt,color) ;
        }
        if(!salta) col++;
        else salta--;
        txt++;
    }
}

/* rellena con espacios hasta el final de la ventana */
for(i=col; i<(ved.ancho-2); i++) v_impcar(fil,i+ved.col+1,' ',ved.clr_princ);

r_puntero(R_MUESTRA);

}

/****************************************************************************
    IMPRIME_TEXTO_ED: imprime el texto en la ventana del editor.
****************************************************************************/
void imprime_texto_ed(void)
{
int i, j, lin_pr, lin_ult;

lin_pr=lin_ed-filcur;
lin_ult=lin_pr+ved.alto-2;
if(lin_ult>numlin) lin_ult=numlin;

for(i=lin_pr; i<lin_ult; i++) imprime_linea_ed(i);

/* rellena hasta fin de ventana */
for(i=lin_ult-lin_pr; i<(ved.alto-2); i++) {
    for(j=0; j<(ved.ancho-2); j++) v_impcar(i+ved.fil+1,j+ved.col+1,' ',
      ved.clr_princ);
}

}

/****************************************************************************
    SCROLL_DER_ED: desplaza hacia la derecha la ventana de edici¢n.
****************************************************************************/
void scroll_der_ed(void)
{
int i, lin_pr, lin_ult;

v_scroll_der(&ved,1);

hdesplz=col_ed-colcur;

lin_pr=lin_ed-filcur;
lin_ult=lin_pr+ved.alto-2;
if(lin_ult>numlin) lin_ult=numlin;

/* imprime los primeros caracteres de todas las l¡neas */
v_pon_cursor(&ved,0,0);
for(i=lin_pr; i<lin_ult; i++) {
    if(hdesplz<(int)strlen(lin[i])) v_impc(&ved,*(lin[i]+hdesplz));
    ved.filc++;
    ved.colc=0;
}

}

/****************************************************************************
    SCROLL_IZQ_ED: desplaza hacia la izquierda la ventana de edici¢n.
****************************************************************************/
void scroll_izq_ed(void)
{
int i, lin_pr, lin_ult, d;

v_scroll_izq(&ved,1);

hdesplz=col_ed-colcur;

lin_pr=lin_ed-filcur;
lin_ult=lin_pr+ved.alto-2;
if(lin_ult>numlin) lin_ult=numlin;

/* imprime los £ltimos caracteres de todas las l¡neas */
v_pon_cursor(&ved,0,ved.ancho-3);
for(i=lin_pr; i<lin_ult; i++) {
    d=hdesplz+ved.ancho-3;
    if(d<(int)strlen(lin[i])) v_impc(&ved,*(lin[i]+d));
    ved.filc++;
    ved.colc=ved.ancho-3;
}

}

/****************************************************************************
    SCROLL_ARR_ED: desplaza hacia arriba la ventana de edici¢n.
****************************************************************************/
void scroll_arr_ed(void)
{
int lin_ult;

v_scroll_arr(&ved,1);

/* imprime £tima l¡nea */
lin_ult=lin_ed-filcur+ved.alto-3;
if(lin_ult>numlin) lin_ult=numlin;
imprime_linea_ed(lin_ult);

}

/****************************************************************************
    SCROLL_ABJ_ED: desplaza hacia abajo la ventana de edici¢n.
****************************************************************************/
void scroll_abj_ed(void)
{
int lin_pr;

v_scroll_abj(&ved,1);

/* imprime primera l¡nea */
lin_pr=lin_ed-filcur;
imprime_linea_ed(lin_pr);

}

/****************************************************************************
    CURSOR_IZQ: mueve el cursor un car cter hacia la izquierda.
****************************************************************************/
void cursor_izq(void)
{

if(col_ed>0) {
    col_ed--;
    if(colcur>0) colcur--;
    else scroll_der_ed();
}

}

/****************************************************************************
    CURSOR_DER: mueve el cursor un car cter hacia la derecha.
****************************************************************************/
void cursor_der(void)
{
int i;

i=strlen(lin[lin_ed]);
if(col_ed<i) {
    col_ed++;
    if(colcur<(col_ed-hdesplz)) {
        if(colcur<(ved.ancho-3)) colcur++;
        else scroll_izq_ed();
    }
}

}

/****************************************************************************
    CURSOR_ABJ: mueve el cursor una l¡nea hacia abajo.
****************************************************************************/
void cursor_abj(void)
{
int i;

if(lin_ed<(numlin-1)) {
    lin_ed++;
    if(filcur<lin_ed) {
        if(filcur<(ved.alto-3)) filcur++;
        else scroll_arr_ed();
    }
}

i=strlen(lin[lin_ed]);
if(col_ed>i) col_ed=i;
colcur=col_ed-hdesplz;
if((colcur<0) || (colcur>(ved.ancho-3))) cursor_fin_lin();

}

/****************************************************************************
    CURSOR_ARR: mueve el cursor una l¡nea hacia arriba.
****************************************************************************/
void cursor_arr(void)
{
int i;

if(lin_ed>0) {
    lin_ed--;
    if(filcur>0) filcur--;
    else scroll_abj_ed();
}

i=strlen(lin[lin_ed]);
if(col_ed>i) col_ed=i;
colcur=col_ed-hdesplz;
if((colcur<0) || (colcur>(ved.ancho-3))) cursor_fin_lin();

}

/****************************************************************************
    CURSOR_FIN_LIN: mueve el cursor al final de la l¡nea actual.
****************************************************************************/
void cursor_fin_lin(void)
{

col_ed=strlen(lin[lin_ed]);
colcur=ved.ancho-3;
if(colcur>col_ed) colcur=col_ed;

pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
    CURSOR_INICIO_LIN: mueve el cursor al inicio de la l¡nea actual.
****************************************************************************/
void cursor_inicio_lin(void)
{

col_ed=0;
colcur=0;

pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
    CURSOR_FIN: mueve el cursor al final del texto.
****************************************************************************/
void cursor_fin(void)
{

lin_ed=numlin-1;
col_ed=strlen(lin[lin_ed]);
filcur=lin_ed;
if(filcur>(ved.alto-3)) filcur=ved.alto-3;
colcur=ved.ancho-3;
if(colcur>col_ed) colcur=col_ed;

pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
    CURSOR_INICIO: mueve el cursor al inicio del texto.
****************************************************************************/
void cursor_inicio(void)
{

lin_ed=0;
col_ed=0;
filcur=0;
colcur=0;

pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
    PAGINA_ARR: desplaza una p gina hacia arriba.
****************************************************************************/
void pagina_arr(void)
{
int i;

lin_ed-=ved.alto-3;
if(lin_ed<0) lin_ed=0;

filcur=0;

i=strlen(lin[lin_ed]);
if(col_ed>i) col_ed=i;
colcur=col_ed-hdesplz;
if((colcur<0) || (colcur>(ved.ancho-3))) cursor_fin_lin();

pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
    PAGINA_ABJ: desplaza una p gina hacia abajo.
****************************************************************************/
void pagina_abj(void)
{
int i;

lin_ed+=ved.alto-3;
if(lin_ed>(numlin-1)) lin_ed=numlin-1;

filcur=ved.alto-3;
if(filcur>lin_ed) filcur=lin_ed;

i=strlen(lin[lin_ed]);
if(col_ed>i) col_ed=i;
colcur=col_ed-hdesplz;
if((colcur<0) || (colcur>(ved.ancho-3))) cursor_fin_lin();

pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
imprime_texto_ed();

}

/****************************************************************************
    CURSOR_POS: coloca el cursor en una posici¢n dentro de la
      ventana de edici¢n.
      Entrada:      'fil', 'col' posici¢n del cursor dentro de la
            ventana de edici¢n
****************************************************************************/
void cursor_pos(int fil, int col)
{
int lin_pr, col_pr, lng;

/* ajusta posici¢n si se sale de la ventana */
if(fil>(ved.alto-3)) fil=ved.alto-3;
if(col>(ved.ancho-3)) col=ved.ancho-3;

/* calcula l¡nea del cursor */
lin_pr=lin_ed-filcur;
lin_ed=lin_pr+fil;
if(lin_ed>(numlin-1)) lin_ed=numlin-1;
filcur=fil;
if(filcur>lin_ed) filcur=lin_ed;

/* calcula columna del cursor */
col_pr=col_ed-colcur;
col_ed=col_pr+col;
lng=strlen(lin[lin_ed]);
if(col_ed>lng) col_ed=lng;
colcur=col;
if((colcur+hdesplz)>col_ed) cursor_fin_lin();

}

/****************************************************************************
    INSERTA_CARACTER: inserta un car cter en la posici¢n actual del
      cursor.
      Entrada:      'c' car cter a insertar
      Salida:       1 si se pudo insertar, 0 si no
****************************************************************************/
int inserta_caracter(char c)
{
char *l, *ll;
int lng, i;

l=lin[lin_ed];

/* longitud actual de la l¡nea contando '\0' final */
lng=strlen(l)+1;

/* si la l¡nea ha alcanzado la m xima longitud permitida, sale */
if(lng>=E_MAXLNGLIN) return(0);

/* indica texto modificado */
modificado=1;

/* a¤ade un byte m s a la l¡nea */
if((l=(char *)realloc(l,lng+1))==NULL) return(0);
lin[lin_ed]=l;

/* inserta el car cter en la posici¢n correspondiente desplazando el */
/* resto de la l¡nea si es necesario */
for(i=0; i<col_ed; i++) l++;

/* puntero auxiliar al final de la l¡nea */
for(ll=l; *ll; ll++);

/* desplaza caracteres */
for(; ll>=l; ll--) *(ll+1)=*ll;
*l=c;

return(1);
}

/****************************************************************************
    BORRA_CARACTER: borra un car cter de una l¡nea.
      Entrada:      'nlin' l¡nea
            'col' posici¢n del car cter dentro de la l¡nea
      Salida:       1 si se pudo borrar, 0 si no
****************************************************************************/
int borra_caracter(int nlin, int col)
{
char *l;
int lng, i;

/* puntero a l¡nea, y longitud */
l=lin[nlin];
lng=strlen(l);

/* si sobrepasa la l¡nea, sale */
if(col>=lng) return(0);

/* indica texto modificado */
modificado=1;

/* puntero a car cter a borrar */
l+=col;

/* borra car cter */
for(i=0; i<(lng-col); i++, l++) *l=*(l+1);

/* reasigna memoria */
if((l=(char *)realloc(lin[nlin],lng))!=NULL) lin[nlin]=l;

return(1);
}

/****************************************************************************
    INSERTA_LINEA: inserta una l¡nea debajo de la actual, moviendo el
      texto desde el car cter actual hasta el final a la nueva l¡nea.
      Salida:       1 si pudo insertar, 0 si no
****************************************************************************/
int inserta_linea(void)
{
char *l1, *l2;
int lng1, lng2, i;

/* si ya est n todas las l¡neas ocupadas, sale */
if(numlin>=E_MAXNUMLIN) return(0);

/* indica texto modificado */
modificado=1;

/* puntero a car cter actual de la l¡nea */
l1=lin[lin_ed];
lng1=strlen(l1);
l1+=col_ed;

/* reserva memoria para la nueva l¡nea */
lng2=lng1-col_ed;
l2=(char *)malloc(lng2+1);
if(l2==NULL) return(0);

/* desplaza l¡neas */
for(i=numlin; i>(lin_ed+1); i--) lin[i]=lin[i-1];

/* traslada texto a nueva l¡nea */
lin[lin_ed+1]=l2;
for(i=0; i<lng2; i++) *l2++=*l1++;
*l2='\0';

/* ajusta tama¤o de l¡nea anterior */
if((l1=(char *)realloc(lin[lin_ed],col_ed+1))==NULL) return(0);
lin[lin_ed]=l1;
*(l1+col_ed)='\0';

/* incrementa n£mero de l¡neas */
numlin++;

return(1);
}

/****************************************************************************
    BORRA_LINEA: borra una l¡nea de texto y desplaza el resto.
      Entrada:      'l' n£mero de l¡nea a borrar
      Salida:       1 si pudo borrar, 0 si no
****************************************************************************/
int borra_linea(int l)
{
int i;

/* si no es n£mero de l¡nea v lido, sale */
if(l>=numlin) return(0);

/* indica texto modificado */
modificado=1;

/* libera memoria */
free(lin[l]);

/* desplaza el resto de las l¡neas */
for(i=l; i<(numlin-1); i++) lin[i]=lin[i+1];
lin[i]=NULL;

numlin--;
if(numlin==0) {
    lin[0]=(char *)malloc(1);
    *lin[0]='\0';
    numlin=1;
    cursor_inicio_lin();
    return(1);
}
else if(lin_ed>=numlin) cursor_arr();

return(1);
}

/****************************************************************************
    COPIA_LINEA: copia una l¡nea de texto al final de otra.
      Entrada:      'lin1' n£mero de l¡nea de destino
            'lin2' n£mero de l¡nea a copiar
      Salida:       1 si pudo copiar, 0 si no
****************************************************************************/
int copia_linea(int lin1, int lin2)
{
char *l1, *l2;
int i, lng1, lng2;

/* si n£mero de l¡nea no v lido, sale */
if((lin1>=numlin) || (lin2>=numlin)) return(0);

/* indica texto modificado */
modificado=1;

l1=lin[lin1];
l2=lin[lin2];
lng1=strlen(l1);
lng2=strlen(l2);

/* si la segunda l¡nea est  vac¡a, sale sin hacer nada */
if(lng2==0) return(1);

/* si la l¡nea resultante sobrepasa la longitud m xima, sale */
if((lng1+lng2)>E_MAXLNGLIN) return(0);

/* reasigna memoria para la l¡nea destino */
if((l1=(char *)realloc(lin[lin1],lng1+lng2+1))==NULL) return(0);
lin[lin1]=l1;

/* copia l¡nea al final de la l¡nea de destino */
for(i=0; i<lng1; i++) l1++;
while(*l2) *l1++=*l2++;
*l1='\0';

return(1);
}

/****************************************************************************
    IMPRIME_INFO: imprime informaci¢n sobre estado del editor.
****************************************************************************/
void imprime_info(void)
{
int i, lng, fil, col, pos_ed;
char info[81], *txt;

r_puntero(R_OCULTA);

/* imprime marca de texto modificado y fila y columna del cursor */
fil=ved.fil+ved.alto-1;
col=ved.col+2;
if(modificado) v_impcar(fil,col-1,E_CHRMOD,ved.clr_s2);
else v_impcar(fil,col-1,E_CHRNOMOD,ved.clr_s2);
sprintf(info," %04u:%03u ",lin_ed+1,col_ed+1);
for(txt=info; *txt; txt++, col++) v_impcar(fil,col,*txt,ved.clr_s2);

/* nombre de fichero */
fil=ved.fil;
lng=strlen(fich_ed);
if(lng>(ved.ancho-4)) lng=ved.ancho-4;
col=ved.col+((ved.ancho-lng-2)/2);
v_impcar(fil,col,E_CHR1F,ved.clr_s1);
col++;
for(txt=fich_ed, i=0; (*txt && (i<lng)); txt++, i++, col++) v_impcar(fil,col,
  *txt,ved.clr_s1);
v_impcar(fil,col,E_CHR2F,ved.clr_s1);

/* barra vertical de desplazamiento */
fil=ved.fil+1;
col=ved.col+ved.ancho-1;
v_impcar(fil,col,E_FLECHARR,ved.clr_s2);
fil++;
lng=ved.alto-4;
for(i=0; i<lng; i++) {
    v_impcar(fil,col,E_CARRELL1,ved.clr_s2);
    fil++;
}
v_impcar(fil,col,E_FLECHABJ,ved.clr_s2);
if(numlin==1) pos_ed=0;
else pos_ed=(int)(((unsigned long)(lng-1)*lin_ed)/(numlin-1));
pos_ed+=ved.fil+2;
v_impcar(pos_ed,col,E_CARRELL2,ved.clr_s2);

/* barra horizontal de desplazamiento */
fil=ved.fil+ved.alto-1;
col=ved.col+12;
v_impcar(fil,col,E_FLECHIZQ,ved.clr_s2);
col++;
lng=ved.ancho-15;
for(i=0; i<lng; i++) {
    v_impcar(fil,col,E_CARRELL1,ved.clr_s2);
    col++;
}
v_impcar(fil,col,E_FLECHDER,ved.clr_s2);
pos_ed=((lng*col_ed)/E_MAXLNGLIN)+ved.col+13;
v_impcar(fil,pos_ed,E_CARRELL2,ved.clr_s2);

r_puntero(R_MUESTRA);

}

/****************************************************************************
    AJUSTA_LINEA: expande las tabulaciones de una l¡nea de texto y
      suprime el '\n' final si lo hay.
      Entrada:      'lin' puntero a la l¡nea con el texto
****************************************************************************/
void ajusta_linea(char *lin)
{
int i;
char temp[E_MAXLNGLIN], *p, *pt;

/* puntero a la l¡nea */
p=lin;
/* puntero a buffer temporal */
pt=temp;

for(i=0; ((i<(E_MAXLNGLIN-1)) && *p && (*p!='\n')); i++, p++) {
    /* si es tabulaci¢n, la expande */
    if(*p=='\t') {
        do {
            *pt++=' ';
            i++;
        } while(((i % 8)!=0) && (i<(E_MAXLNGLIN-1)));
        i--;
    }
    else *pt++=*p;
}

/* coloca '\0' final */
*pt='\0';

/* devuelve cadena expandida */
strcpy(lin,temp);

}

/****************************************************************************
    SUPRIME_ESPACIOS_FIN: suprime los espacios al final de una l¡nea.
      Entrada:      'l' n£mero de l¡nea
****************************************************************************/
void suprime_espacios_fin(int l)
{
char *pl;
int lng, i=0;

/* puntero al final de la l¡nea */
lng=strlen(lin[l]);
pl=lin[l]+lng-1;

/* busca £ltimo car cter que no sea espacio y cuenta espacios finales */
while((*pl==' ') && (pl>lin[l])) {
    pl--;
    i++;
}

if(i) {
    /* indica texto modificado */
    modificado=1;

    /* marca fin de la l¡nea */
    *(pl+1)='\0';

    /* reasigna memoria */
    if((pl=(char *)realloc(lin[l],lng-i+1))!=NULL) lin[l]=pl;
}

}

/****************************************************************************
    BUSCA_CADENA: busca una cadena desde la posici¢n actual del cursor.
      Salida:       1 cadena encontrada, 0 no
****************************************************************************/
int busca_cadena(void)
{
char linea[E_MAXLNGLIN], cadena[E_MAXLNGLIN], *l, *p;
int i;

/* si cadena a buscar vac¡a, sale */
if(*cadena_busca=='\0') return(0);

/* posici¢n actual del cursor */
i=lin_ed;
l=lin[i]+col_ed;
while(i<numlin) {
    strcpy(linea,l);
    strcpy(cadena,cadena_busca);
    /* si no distingue may£sculas y min£sculas, convierte a may£sculas */
    if(chk_may->estado==0) {
        conv_mayuscula(linea);
        conv_mayuscula(cadena);
    }
    p=strstr(linea,cadena);
    if(p!=NULL) break;
    i++;
    l=lin[i];
}

/* si cadena no encontrada, sale */
if(i==numlin) return(0);

/* coloca cursor al inicio de la cadena encontrada */
lin_ed=i;
col_ed=(int)(p-linea)+(int)(l-lin[i]);
if(col_ed<(ved.ancho-3)) colcur=col_ed;

return(1);
}

/****************************************************************************
    MARCA_BLOQUE: marca un bloque de texto.
****************************************************************************/
void marca_bloque(void)
{
STC_RATON r;
int fin_bloque=0, accion, boton=0, ccursor, tecla, shift, minfil, mincol,
  maxfil, maxcol, posv;

mbloque=1;
lin_ib=lin_ed;
col_ib=col_ed;

/* guarda posici¢n actual del cursor */
ccursor=colcur;

while(!fin_bloque) {
    lin_fb=lin_ed;
    col_fb=col_ed;

    pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
    imprime_texto_ed();
    imprime_info();

    do {
        accion=0;
        r_estado(&r);
        if(r.boton1) {
            accion=1;
            if(boton<2) boton++;
        }
        else if(r.boton2) {
            boton=0;
            accion=2;
        }
        else {
            boton=0;
            tecla=bioskey(1);
            if(tecla) {
                bioskey(0);
                accion=3;
            }
        }
    } while(!accion);

    if(accion==1) {
        /* calcula l¡mites de la ventana de edici¢n posici¢n */
        /* del cuadro de desplazamiento */
        minfil=ved.fil;
        maxfil=ved.fil+ved.alto-1;
        mincol=ved.col;
        maxcol=ved.col+ved.ancho-1;

        if(numlin==1) posv=0;
        else posv=(int)(((unsigned long)(ved.alto-5)*lin_ed)/
          (numlin-1));
        posv+=ved.fil+2;

        /* si est  fuera de la ventana de edici¢n, sale */
        if((r.fil<minfil) || (r.fil>maxfil) || (r.col<mincol) ||
          (r.col>maxcol)) fin_bloque=1;
        /* comprueba si est  en barra vertical de desplazamiento */
        else if(r.col==maxcol) {
            if(r.fil==(minfil+1)) cursor_arr();
            else if(r.fil==(maxfil-1)) cursor_abj();
            else if(r.fil==minfil) cursor_inicio();
            else if(r.fil==maxfil) cursor_fin();
            else if(r.fil<posv) pagina_arr();
            else if(r.fil>posv) pagina_abj();
            else if(r.fil==posv) {
                if(posv==(ved.fil+2)) cursor_inicio();
                else if(posv==(ved.fil+ved.alto-3))
                  cursor_fin();
            }
        }
        /* comprueba si est  en barra horizontal de desplazamiento */
        else if(r.fil==maxfil) {
            if(r.col==(mincol+12)) cursor_izq();
            else if(r.col==(maxcol-1)) cursor_der();
        }
        /* comprueba si est  dentro de la ventana de edici¢n */
        else if((r.fil>minfil) && (r.fil<maxfil) &&
          (r.col>mincol) && (r.col<maxcol)) cursor_pos(r.fil-minfil-1,
          r.col-mincol-1);

        if(boton==1) delay(E_PAUSA1);
        else delay(E_PAUSA2);
    }
    else if(accion==2) fin_bloque=1;
    else {
        shift=bioskey(2);

        /* comprueba si est  pulsado alg£n SHIFT */
        if(shift & 0x0003) {
            switch((tecla >> 8) & 0x00ff) {
                case TCUR_IZQ :
                    cursor_izq();
                    break;
                case TCUR_DER :
                    cursor_der();
                    break;
                case TCUR_ABJ :
                    cursor_abj();
                    break;
                case TCUR_ARR :
                    cursor_arr();
                    break;
                case TPAG_ABJ :
                    pagina_abj();
                    break;
                case TPAG_ARR :
                    pagina_arr();
                    break;
                case TFIN :
                    cursor_fin_lin();
                    break;
                case TORG :
                    cursor_inicio_lin();
                    break;
                case TCTR_FIN :
                    cursor_fin();
                    break;
                case TCTR_ORG :
                    cursor_inicio();
                    break;
                case TSUP :             /* BORRAR BLOQUE */
                    if(!borra_bloque(lin_ib,col_ib,
                    lin_fb,col_fb)) beep();
                    if(lin_ib<=lin_fb) lin_ed=lin_ib;
                    else lin_ed=lin_fb;
                    if(col_ib<=col_fb) col_ed=col_ib;
                    else col_ed=col_fb;
                    cursor_pos(filcur,ccursor);
                    fin_bloque=1;
                    break;
            }
        }
        else switch((tecla >> 8) & 0x00ff) {
            case TCUR_IZQ :
            case TCUR_DER :
            case TCUR_ABJ :
            case TCUR_ARR :
            case TPAG_ABJ :
            case TPAG_ARR :
            case TFIN :
            case TORG :
            case TCTR_FIN :
            case TCTR_ORG :
                fin_bloque=1;
                break;
            /* COPIAR BLOQUE */
            /* case TCTR_INS : */
            case TCTR_C :
                if(!copia_bloque(lin_ib,col_ib,
                  lin_fb,col_fb)) beep();
                fin_bloque=1;
                break;
            /* CORTAR BLOQUE */
            /* case TCTR_SUP : */
            case TCTR_X :
                if(!copia_bloque(lin_ib,col_ib,
                  lin_fb,col_fb)) beep();
                else if(!borra_bloque(lin_ib,col_ib,
                  lin_fb,col_fb)) beep();
                if(lin_ib<=lin_fb) lin_ed=lin_ib;
                else lin_ed=lin_fb;
                if(col_ib<=col_fb) col_ed=col_ib;
                else col_ed=col_fb;
                cursor_pos(filcur,ccursor);
                fin_bloque=1;
                break;
        }
    }
}

mbloque=0;
imprime_texto_ed();

/* espera a que suelte bot¢n 2 */
do {
    r_estado(&r);
} while(r.boton2);

}

/****************************************************************************
    COPIA_BLOQUE: copia un bloque de texto en un buffer de memoria.
      Entrada:      'lini', 'coli' inicio del bloque
            'linf', 'colf' final del bloque
      Salida:       1 si se pudo copiar, 0 si no
****************************************************************************/
int copia_bloque(int lini, int coli, int linf, int colf)
{
STC_BLOQUE *b;
char temp[E_MAXLNGLIN], *txt;
int i, nlin, ncol;

if((lini==linf) && (coli>colf)) {
    i=colf;
    colf=coli;
    coli=i;
}
else if(lini>linf) {
    i=linf;
    linf=lini;
    lini=i;
    i=colf;
    colf=coli;
    coli=i;
}

/* si hay un bloque lo borra */
if(bloque!=NULL) e_vacia_bloque();

for(nlin=lini; nlin<=linf; nlin++) {
    /* si es primera l¡nea coge texto a partir de columna de inicio */
    /* si no coger texto a partir de primera columna */
    if(nlin==lini) {
        ncol=coli;
        b=(STC_BLOQUE *)malloc(sizeof(STC_BLOQUE));
        if(b==NULL) return(0);
        b->lin=NULL;
        b->sgte=NULL;
        bloque=b;
    }
    else {
        ncol=0;
        b->sgte=(STC_BLOQUE *)malloc(sizeof(STC_BLOQUE));
        if(b->sgte==NULL) {
            e_vacia_bloque();
            return(0);
        }
        b=b->sgte;
        b->lin=NULL;
        b->sgte=NULL;
    }
    i=0;
    txt=lin[nlin]+ncol;
    while(*txt && (i<(E_MAXLNGLIN-1))) {
        /* si es £ltima l¡nea de bloque comprueba que */
        /* no sobrepase columna final */
        if((nlin==linf) && (ncol>colf)) break;
        temp[i]=*txt;
        i++;
        ncol++;
        txt++;
    }
    temp[i]='\0';

    /* guarda texto en buffer */
    b->lin=(char *)malloc(strlen(temp)+1);
    if(b->lin==NULL) {
        e_vacia_bloque();
        return(0);
    }
    strcpy(b->lin,temp);
}

return(1);
}

/****************************************************************************
    BORRA_BLOQUE: borra un bloque de texto.
      Entrada:      'lini', 'coli' inicio del bloque
            'linf', 'colf' final del bloque
      Salida:       1 si se pudo borrar, 0 si no
****************************************************************************/
int borra_bloque(int lini, int coli, int linf, int colf)
{
int i, nlin, lng;

if((lini==linf) && (coli>colf)) {
    i=colf;
    colf=coli;
    coli=i;
}
else if(lini>linf) {
    i=linf;
    linf=lini;
    lini=i;
    i=colf;
    colf=coli;
    coli=i;
}

/* indica texto modificado */
modificado=1;

/* si l¡nea de inicio es igual a l¡nea final */
if(lini==linf) {
    lng=strlen(lin[lini]);
    if(colf>(lng-1)) colf=lng-1;
    for(i=coli; i<=colf; i++) {
        if(!borra_caracter(lini,coli)) return(0);
    }
}
else {
    for(nlin=lini; nlin<=linf; nlin++) {
        /* si es primera l¡nea borra texto desde de columna inicial */
        if(nlin==lini) {
            lng=strlen(lin[nlin]);
            for(i=coli; i<lng; i++) {
                if(!borra_caracter(nlin,coli)) return(0);
            }
        }
        /* si es la £ltima borra texto hasta columna final */
        else if(nlin==linf) {
            lng=strlen(lin[linf]);
            if(colf>(lng-1)) colf=lng-1;
            for(i=0; i<=colf; i++) {
                if(!borra_caracter(nlin,0)) return(0);
            }
        }
        /* en cualquier otro caso borra l¡nea entera */
        else {
            free(lin[nlin]);
            lin[nlin]=NULL;
        }
    }

    /* si borr¢ l¡neas intermedias, desplaza texto */
    nlin=linf-lini-1;

    /* si borr¢ l¡nea inicial completa la elimina tambi‚n */
    if(!*lin[lini]) {
        free(lin[lini]);
        lin[lini]=NULL;
        nlin++;
    }
    /* ¡dem l¡nea final */
    if(!*lin[linf]) {
        free(lin[linf]);
        lin[linf]=NULL;
        linf++;
        nlin++;
    }

    if(nlin) {
        for(i=linf; i<numlin; i++) lin[i-nlin]=lin[i];
        numlin-=nlin;
    }

    /* si se han borrado todas la l¡neas crea una */
    if(!numlin) {
        lin[0]=(char *)malloc(1);
        *lin[0]='\0';
        numlin++;
    }
}

return(1);
}

/****************************************************************************
    INSERTA_BLOQUE: inserta un bloque en el texto.
      Entrada:      'nlin', 'col' posici¢n d¢nde insertar bloque
      Salida:       1 si se pudo borrar, 0 si no
****************************************************************************/
int inserta_bloque(int nlin, int col)
{
STC_BLOQUE *b;
int nlin_bloque=0, lng, i, j;
char *txtblq, *txt, temp[E_MAXLNGLIN];

/* si no hay bloque, sale */
if(bloque==NULL) return(0);

/* calcula n£mero de l¡neas de bloque (sin contar primera) */
b=bloque;
while(b->sgte!=NULL) {
    b=b->sgte;
    nlin_bloque++;
}

/* si n£mero de l¡neas de bloque sobrepasa m ximo permitido, sale */
if((numlin+nlin_bloque)>E_MAXNUMLIN) return(0);

/* indica texto modificado */
modificado=1;

/* inserta primera l¡nea de bloque */
txtblq=bloque->lin;

/* calcula n£mero de caracteres de bloque que se pueden insertar */
lng=strlen(lin[nlin])+strlen(txtblq);
if(lng>(E_MAXLNGLIN-1)) lng=(E_MAXLNGLIN-1)-strlen(lin[nlin]);
else lng=strlen(txtblq);

strcpy(temp,lin[nlin]);
txt=lin[nlin]+col;
for(i=col; ((lng!=0) && *txtblq); i++, txtblq++, lng--) temp[i]=*txtblq;
for(; ((i<(E_MAXLNGLIN-1)) && *txt); i++, txt++) temp[i]=*txt;
temp[i]='\0';
if((txt=realloc(lin[nlin],strlen(temp)+1))!=NULL) {
    lin[nlin]=txt;
    strcpy(lin[nlin],temp);
}
nlin++;

/* inserta siguientes l¡neas de bloque */
b=bloque->sgte;
for(i=0; i<nlin_bloque; i++) {
    if((txt=(char *)malloc(strlen(b->lin)+1))==NULL) return(0);
    strcpy(txt,b->lin);
    /* desplaza l¡neas hacia abajo */
    for(j=numlin; j>nlin; j--) lin[j]=lin[j-1];
    lin[nlin]=txt;
    nlin++;
    numlin++;
    b=b->sgte;
}

return(1);
}

/****************************************************************************
    COGE_PALABRA_CURSOR: recoge la palabra sobre la que se encuentra
      el cursor y la guarda en buffer 'palabra_cursor'.
****************************************************************************/
void coge_palabra_cursor(void)
{
char *ltxt;
int i;

/* puntero a posici¢n actual de cursor */
ltxt=lin[lin_ed]+col_ed;

/* busca hacia atr s hasta encontrar el l¡mite izquierdo de */
/* la palabra (car cter no alfanum‚rico) */
for(; (ltxt>lin[lin_ed]) && isalnum(*ltxt);  ltxt--);
ltxt++;

/* almacena palabra en buffer */
for(i=0; (i<(E_MAXLNGLIN-1)) && *ltxt && isalnum(*ltxt); i++)
  palabra_cursor[i]=*ltxt++;

palabra_cursor[i]='\0';

}

/****************************************************************************
    E_INICIALIZA: inicializa el editor.
      Entrada:      'fich' nombre de fichero por defecto
            'fil', 'col' origen de la ventana de edici¢n
            'ancho', 'alto' tama¤o de la ventana de edici¢n
            'clr_princ' color principal de la ventana
            'clr_s1', 'clr_s2' colores de sombreado
            'clr_bloque' color para marcar bloque
            'clr_comentario' color para comentarios
            'clr_palabraclv' color para palabras clave
            'clr_especial' color para caracteres especiales
            'clr_princc', 'clr_s1c', 'clr_s2c', 'clr_botonc',
            'clr_inputc', 'clr_teclac', 'clr_tecla_botonc'
            colores para cuadro de di logo
            'fsintx' nombre de fichero con datos para resalte
            de sintaxis
            'modo' modo de edici¢n inicial:
                E_EDITA permite editar
                E_SOLOLECT no permite edici¢n
****************************************************************************/
void e_inicializa(char *fich, int fil, int col, int ancho, int alto,
  BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_bloque,
  BYTE clr_comentario, BYTE clr_palabraclv, BYTE clr_especial,
  BYTE clr_princc, BYTE clr_s1c, BYTE clr_s2c, BYTE clr_botonc,
  BYTE clr_inputc, BYTE clr_teclac, BYTE clr_tecla_botonc, char *fsintx,
  int modo)
{
STC_ELEM *elem;
FILE *fstx;
char l0[256], *l;
int i, icom=0, iesp=0, i2=0, i3=0, i4=0, i5=0, i6=0, i7=0, i8=0, i9=0;

/* crea la ventana de edici¢n */
v_crea(&ved,fil,col,ancho,alto,clr_princ,clr_s1,clr_s2,NULL,1);

/* inicializa punteros a l¡neas de texto */
for(i=0; i<E_MAXNUMLIN; i++) lin[i]=NULL;

/* crea la primera l¡nea de texto */
lin[0]=(char *)malloc(1);
*lin[0]='\0';

/* inicializa estado del editor */
lin_ed=0;
col_ed=0;
filcur=0;
colcur=0;
numlin=1;
modificado=0;
modo_ed=modo;
strcpy(fich0,fich);
strupr(fich0);
strcpy(fich_ed,fich0);
lin_ib=0;
col_ib=0;
lin_fb=0;
col_fb=0;
mbloque=0;
clr_blq=clr_bloque;
clr_coment=clr_comentario;
clr_palclv=clr_palabraclv;
clr_esp=clr_especial;

/* crea cuadro de di logo para b£squedas */
c_crea_cuadro(&cbusca," Buscar ",CB_FIL,CB_COL,CB_ANCHO,CB_ALTO,clr_princc,
  clr_s1c,clr_s2c,clr_botonc,clr_inputc,clr_princc,clr_teclac,
  clr_tecla_botonc);
cadena_busca[0]='\0';
c_crea_elemento(&cbusca,C_ELEM_INPUT,0,7,"^Texto",CB_ANCHO-10,cadena_busca,
  E_MAXLNGLIN-1);
c_crea_elemento(&cbusca,C_ELEM_BOTON,4,(CB_ANCHO-20)/2,"^Vale",8);
c_crea_elemento(&cbusca,C_ELEM_BOTON,4,((CB_ANCHO-20)/2)+11,"^Salir",9);
elem=c_crea_elemento(&cbusca,C_ELEM_CHECK,2,1,"^May£sculas/min£sculas");
chk_may=(STC_ELEM_CHECK *)elem->info;

/* inicializa puntero a buffer para bloque de texto */
bloque=NULL;

/* inicializa buffer para palabra sobre la que est  cursor */
*palabra_cursor='\0';

/* carga datos para resalte de sintaxis */
carcoment[0]='\0';
caresp[0]='\0';
for(i=0; i<E_PX; i++) {
    palclv2[i]=NULL;
    palclv3[i]=NULL;
    palclv4[i]=NULL;
    palclv5[i]=NULL;
    palclv6[i]=NULL;
    palclv7[i]=NULL;
    palclv8[i]=NULL;
    palclv9[i]=NULL;
}

/* abre fichero con datos de resalte de sintaxis */
if((fstx=fopen(fsintx,"rt"))==NULL) return;

/* comentario */
if(fgets(l0,256,fstx)==NULL) {
    fclose(fstx);
    return;
}

/* caracteres de inicio de comentario */
if(fgets(l0,256,fstx)==NULL) {
    fclose(fstx);
    return;
}
for(l=l0; *l && (icom<(E_CX-1)); l++) if(*l!=' ') carcoment[icom++]=*l;
carcoment[icom]='\0';

/* caracteres especiales */
if(fgets(l0,256,fstx)==NULL) {
    fclose(fstx);
    return;
}
for(l=l0; *l && (iesp<(E_CX-1)); l++) if(*l!=' ') caresp[iesp++]=*l;
caresp[iesp]='\0';

/* palabras clave */
while(1) {
    if(fgets(l0,256,fstx)==NULL) break;

    /* pone marca de fin de cadena a palabra */
    l=l0;
    while(isalnum(*l)) l++;
    *l='\0';

    /* reserva memoria y guarda palabra */
    l=(char *)malloc(strlen(l0));
    if(l==NULL) break;
    strcpy(l,l0);

    /* coloca palabra en el grupo correspondiente */
    switch(strlen(l0)) {
        case 2 :
            if(i2<(E_PX-1)) palclv2[i2++]=l;
            else free(l);
            break;
        case 3 :
            if(i3<(E_PX-1)) palclv3[i3++]=l;
            else free(l);
            break;
        case 4 :
            if(i4<(E_PX-1)) palclv4[i4++]=l;
            else free(l);
            break;
        case 5 :
            if(i5<(E_PX-1)) palclv5[i5++]=l;
            else free(l);
            break;
        case 6 :
            if(i6<(E_PX-1)) palclv6[i6++]=l;
            else free(l);
            break;
        case 7 :
            if(i7<(E_PX-1)) palclv7[i7++]=l;
            else free(l);
            break;
        case 8 :
            if(i8<(E_PX-1)) palclv8[i8++]=l;
            else free(l);
            break;
        case 9 :
            if(i9<(E_PX-1)) palclv9[i9++]=l;
            else free(l);
            break;
        default :
            free(l);
    }
}

fclose(fstx);

palclv2[i2]=NULL;
palclv3[i3]=NULL;
palclv4[i4]=NULL;
palclv5[i5]=NULL;
palclv6[i6]=NULL;
palclv7[i7]=NULL;
palclv8[i8]=NULL;
palclv9[i9]=NULL;

}

/****************************************************************************
    E_ELIMINA: libera memoria ocupada por el editor.
****************************************************************************/
void e_elimina(void)
{
int i;

e_borra_texto();
e_vacia_bloque();
c_elimina(&cbusca);
for(i=0; i<E_PX; i++) {
    if(palclv2[i]!=NULL) {
        free(palclv2[i]);
        palclv2[i]=NULL;
    }
    if(palclv3[i]!=NULL) {
        free(palclv3[i]);
        palclv3[i]=NULL;
    }
    if(palclv4[i]!=NULL) {
        free(palclv4[i]);
        palclv4[i]=NULL;
    }
    if(palclv5[i]!=NULL) {
        free(palclv5[i]);
        palclv5[i]=NULL;
    }
    if(palclv6[i]!=NULL) {
        free(palclv6[i]);
        palclv6[i]=NULL;
    }
    if(palclv7[i]!=NULL) {
        free(palclv7[i]);
        palclv7[i]=NULL;
    }
    if(palclv8[i]!=NULL) {
        free(palclv8[i]);
        palclv8[i]=NULL;
    }
    if(palclv9[i]!=NULL) {
        free(palclv9[i]);
        palclv9[i]=NULL;
    }
}

}

/****************************************************************************
    E_CAMBIA_COLOR: modifica los colores del editor.
      Entrada:      'clr_princ' color principal de la ventana
            'clr_s1', 'clr_s2' colores de sombreado
            'clr_bloque' color para marcar bloque
            'clr_comentario' color para comentarios
            'clr_palabraclv' color para palabras clave
            'clr_especial' color para caracteres especiales
            'clr_princc', 'clr_s1c', 'clr_s2c', 'clr_botonc',
            'clr_inputc', 'clr_teclac', 'clr_tecla_botonc'
            colores para cuadro de di logo
            'modo' 1 redibuja el editor, 0 no lo redibuja
****************************************************************************/
void e_cambia_color(BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_bloque,
  BYTE clr_comentario, BYTE clr_palabraclv, BYTE clr_especial,BYTE clr_princc, BYTE clr_s1c, BYTE clr_s2c, BYTE clr_botonc,
  BYTE clr_inputc, BYTE clr_teclac, BYTE clr_tecla_botonc, int modo)
{

ved.clr_princ=clr_princ;
ved.clr_s1=clr_s1;
ved.clr_s2=clr_s2;
ved.clr_texto=clr_princ;

clr_blq=clr_bloque;
clr_coment=clr_comentario;
clr_palclv=clr_palabraclv;
clr_esp=clr_especial;

c_cambia_color(&cbusca,clr_princc,clr_s1c,clr_s2c,clr_botonc,clr_inputc,
  clr_princc,clr_teclac,clr_tecla_botonc,0);

if(modo) {
    v_dibuja(&ved,1);
    imprime_texto_ed();
    imprime_info();
}

}

/****************************************************************************
    E_BORRA_TEXTO: borra todo el texto del editor, liberando la
      memoria ocupada e inicializa variables internas.
****************************************************************************/
void e_borra_texto(void)
{
int i;

i=0;
while((lin[i]!=NULL) && (i<numlin)) {
    free(lin[i]);
    lin[i]=NULL;
    i++;
}

/* inicializa variables internas */
lin_ed=0;
col_ed=0;
filcur=0;
colcur=0;
numlin=0;
modificado=0;
strcpy(fich_ed,fich0);

}

/****************************************************************************
    E_EDITOR: rutina de control principal del editor.
      Salida:       0 si puls¢ ESCAPE
            1 si puls¢ F1 (ayuda)
            2 si puls¢ Shift+F1 (ayuda palabra del cursor)
****************************************************************************/
int e_editor(void)
{
STC_RATON r;
int fin_edicion=0, col, lin_ant, lin_act, accion, boton=0, tecla, shift,
  ascii, minfil, mincol, maxfil, maxcol, posv;

/* si rat¢n no est  inicializado, lo inicializa */
if(!r_puntero(R_MUESTRA)) r_inicializa();

v_borra(&ved);

/* si no hay l¡neas, crea la primera */
if(!numlin) {
    lin[0]=(char *)malloc(1);
    *lin[0]='\0';
    numlin++;
}

pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
imprime_texto_ed();

lin_ant=lin_ed;
lin_act=lin_ed;

while(!fin_edicion) {
    /* comprueba si ha cambiado de l¡nea y est  editando */
    if((lin_act!=lin_ant) && (modo_ed==E_EDITA)) {
        if(lin_ant<numlin) suprime_espacios_fin(lin_ant);
    }
    lin_ant=lin_act;

    pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
    imprime_linea_ed(lin_ed);

    imprime_info();

    do {
        accion=0;
        r_estado(&r);
        if(r.boton1) {
            accion=1;
            if(boton<2) boton++;
        }
        else if(r.boton2) {
            boton=0;
            accion=2;
        }
        else {
            boton=0;
            tecla=bioskey(1);
            if(tecla) {
                bioskey(0);
                accion=3;
            }
        }
    } while(!accion);

    if(accion==1) {
        /* calcula l¡mites de la ventana de edici¢n posici¢n */
        /* del cuadro de desplazamiento */
        minfil=ved.fil;
        maxfil=ved.fil+ved.alto-1;
        mincol=ved.col;
        maxcol=ved.col+ved.ancho-1;

        if(numlin==1) posv=0;
        else posv=(int)(((unsigned long)(ved.alto-5)*lin_ed)/
          (numlin-1));
        posv+=ved.fil+2;

        /* si est  fuera de la ventana de edici¢n, sale */
        if((r.fil<minfil) || (r.fil>maxfil) || (r.col<mincol) ||
          (r.col>maxcol)) fin_edicion=1;
        /* comprueba si est  en barra vertical de desplazamiento */
        else if(r.col==maxcol) {
            if(r.fil==(minfil+1)) cursor_arr();
            else if(r.fil==(maxfil-1)) cursor_abj();
            else if(r.fil==minfil) cursor_inicio();
            else if(r.fil==maxfil) cursor_fin();
            else if(r.fil<posv) pagina_arr();
            else if(r.fil>posv) pagina_abj();
            else if(r.fil==posv) {
                if(posv==(ved.fil+2)) cursor_inicio();
                else if(posv==(ved.fil+ved.alto-3))
                  cursor_fin();
            }
        }
        /* comprueba si est  en barra horizontal de desplazamiento */
        else if(r.fil==maxfil) {
            if(r.col==(mincol+12)) cursor_izq();
            else if(r.col==(maxcol-1)) cursor_der();
        }
        /* comprueba si est  dentro de la ventana de edici¢n */
        else if((r.fil>minfil) && (r.fil<maxfil) && (r.col>mincol) &&
          (r.col<maxcol)) cursor_pos(r.fil-minfil-1,r.col-mincol-1);

        if(boton==1) delay(E_PAUSA1);
        else delay(E_PAUSA2);
    }
    else if(accion==2) {
        /* calcula l¡mites de la ventana de edici¢n posici¢n */
        /* del cuadro de desplazamiento */
        minfil=ved.fil;
        maxfil=ved.fil+ved.alto-1;
        mincol=ved.col;
        maxcol=ved.col+ved.ancho-1;

        /* comprueba si est  dentro de la ventana de edici¢n */
        if((r.fil>minfil) && (r.fil<maxfil) && (r.col>mincol) &&
          (r.col<maxcol)) {
            cursor_pos(r.fil-minfil-1,r.col-mincol-1);
            pon_cursor(filcur+ved.fil+1,colcur+ved.col+1);
            /* espera a que suelte bot¢n 2 */
            do {
                r_estado(&r);
            } while(r.boton2);
            if(modo_ed==E_SOLOLECT) beep();
            else marca_bloque();
        }
    }
    else {
        /* coge estado de teclas SHIFT */
        shift=bioskey(2);

        /* mira si tecle¢ un car cter v lido */
        ascii=tecla & 0x00ff;

        if(ascii==RETURN) {
            if(modo_ed==E_SOLOLECT) beep();
            else if(inserta_linea()) {
                cursor_abj();
                cursor_inicio_lin();
            }
            else beep();
        }
        else if(ascii==TAB) {
            if(modo_ed==E_SOLOLECT) beep();
            else do {
                if(inserta_caracter(' ')) cursor_der();
            } while((col_ed % 8)!=0);
        }
        else if(ascii>31) {
            if(modo_ed==E_SOLOLECT) beep();
            else if(inserta_caracter((char)ascii)) cursor_der();
            else beep();
        }
        /* comprueba si est  pulsado alg£n SHIFT */
        else if(shift & 0x0003) {
            switch((tecla >> 8) & 0x00ff) {
                case TINS :     /* INSERTAR BLOQUE */
                    if(!inserta_bloque(lin_ed,col_ed))
                      beep();
                    imprime_texto_ed();
                    break;
                case TSF1 :     /* AYUDA PALABRA CURSOR */
                    coge_palabra_cursor();
                    return(2);
                case TDEL :
                    if(modo_ed==E_SOLOLECT) {
                        beep();
                        break;
                    }
                    /* si no est  al inicio de la l¡nea */
                    if(col_ed>0) {
                        if(borra_caracter(lin_ed,
                          col_ed-1)) cursor_izq();
                    }
                    else if(lin_ed>0) {
                        col=strlen(lin[lin_ed-1]);
                        if(copia_linea(lin_ed-1,
                          lin_ed)) {
                            if(lin_ed==(numlin-1))
                              borra_linea(lin_ed);
                            else {
                                borra_linea(lin_ed);
                                cursor_arr();
                            }
                            col_ed=col;
                            colcur=col;
                            if(colcur>(ved.ancho-3))
                              colcur=ved.ancho-3;
                            pon_cursor(filcur+
                              ved.fil+1,colcur+
                              ved.col+1);
                            imprime_texto_ed();
                        }
                        else beep();
                    }
                    /* elimina c¢digo ASCII */
                    tecla &= 0xff00;
                    break;
                case TCUR_IZQ :
                case TCUR_DER :
                case TCUR_ABJ :
                case TCUR_ARR :
                case TPAG_ABJ :
                case TPAG_ARR :
                case TFIN :
                case TORG :
                case TCTR_FIN :
                case TCTR_ORG :
                    if(modo_ed==E_SOLOLECT) beep();
                    else marca_bloque();
                    break;
            }
        }
        /* comprueba si est  pulsado CTRL */
        else if(shift & 0x0004) {
            switch((tecla >> 8) & 0x00ff) {
                case TCTR_FIN :
                    cursor_fin();
                    break;
                case TCTR_ORG :
                    cursor_inicio();
                    break;
                case TCTR_Y :   /* BORRAR LINEA */
                    if(modo_ed==E_SOLOLECT) beep();
                    else {
                        borra_linea(lin_ed);
                        cursor_inicio_lin();
                    }
                    /* elimina c¢digo ASCII */
                    tecla=0;
                    break;
                case TCTR_L :   /* CONTINUAR BUSQUEDA */
                    e_continua_busqueda();
                    /* elimina c¢digo ASCII */
                    tecla=0;
                    break;
                case TCTR_V :   /* INSERTAR BLOQUE */
                    if(!inserta_bloque(lin_ed,col_ed))
                      beep();
                    imprime_texto_ed();
                    break;
            }
        }
        else switch((tecla >> 8) & 0x00ff) {
            case TCUR_IZQ :
                cursor_izq();
                break;
            case TCUR_DER :
                cursor_der();
                break;
            case TCUR_ABJ :
                cursor_abj();
                break;
            case TCUR_ARR :
                cursor_arr();
                break;
            case TPAG_ABJ :
                pagina_abj();
                break;
            case TPAG_ARR :
                pagina_arr();
                break;
            case TFIN :
                cursor_fin_lin();
                break;
            case TORG :
                cursor_inicio_lin();
                break;
            case TDEL :
                if(modo_ed==E_SOLOLECT) {
                    beep();
                    break;
                }
                /* si no est  al inicio de la l¡nea */
                if(col_ed>0) {
                    if(borra_caracter(lin_ed,col_ed-1))
                      cursor_izq();
                }
                else if(lin_ed>0) {
                    col=strlen(lin[lin_ed-1]);
                    if(copia_linea(lin_ed-1,lin_ed)) {
                        if(lin_ed==(numlin-1))
                          borra_linea(lin_ed);
                        else {
                            borra_linea(lin_ed);
                            cursor_arr();
                        }
                        col_ed=col;
                        colcur=col;
                        if(colcur>(ved.ancho-3))
                          colcur=ved.ancho-3;
                        pon_cursor(filcur+ved.fil+1,
                          colcur+ved.col+1);
                        imprime_texto_ed();
                    }
                    else beep();
                }
                /* elimina c¢digo ASCII */
                tecla &= 0xff00;
                break;
            case TSUP :
                if(modo_ed==E_SOLOLECT) {
                    beep();
                    break;
                }
                col=strlen(lin[lin_ed]);
                if(col_ed>=col) {
                    if(copia_linea(lin_ed,lin_ed+1)) {
                        borra_linea(lin_ed+1);
                        pon_cursor(filcur+ved.fil+1,
                          colcur+ved.col+1);
                        imprime_texto_ed();
                    }
                    else beep();
                }
                else borra_caracter(lin_ed,col_ed);
                break;
            case TESC :
                fin_edicion=1;
                /* elimina c¢digo ASCII */
                tecla &= 0xff00;
                break;
            case TF1 :              /* AYUDA */
                return(1);
            case TF2 :              /* BUSCAR */
                e_inicia_busqueda();
                break;
            case TF3 :              /* CONTINUAR BUSQUEDA */
                e_continua_busqueda();
                break;
            case TF4 :              /* BORRAR LINEA */
                if(modo_ed==E_SOLOLECT) beep();
                else {
                    borra_linea(lin_ed);
                    cursor_inicio_lin();
                }
                break;
        }
    }

    lin_act=lin_ed;

}

return(0);
}

/****************************************************************************
    E_DIBUJA_EDITOR: dibuja la ventana de edici¢n.
****************************************************************************/
void e_dibuja_editor(void)
{

v_borra(&ved);
imprime_texto_ed();
imprime_info();

}

/****************************************************************************
    E_CARGA_TEXTO: carga un fichero de texto en el editor.
      Entrada:      'fich' nombre del fichero a cargar
      Salida:       1 si se carg¢ correctamente, 0 si hubo errores
****************************************************************************/
int e_carga_texto(char *fich)
{
FILE *f_texto;
char temp[E_MAXLNGLIN], *txt;

if((f_texto=fopen(fich,"rt"))==NULL) return(0);

/* borra texto del editor */
e_borra_texto();

/* indica texto nuevo */
modificado=0;

/* copia nombre de fichero */
strcpy(fich_ed,fich);
strupr(fich_ed);

/* lee l¡neas del fichero */
while(numlin<E_MAXNUMLIN) {
    if(fgets(temp,E_MAXLNGLIN,f_texto)==NULL) {
        if(feof(f_texto)) break;
        else {
            fclose(f_texto);
            return(0);
        }
    }

    /* expande tabulaciones y suprime '\n' final */
    ajusta_linea(temp);

    /* inserta l¡nea en el editor */
    txt=(char *)malloc(strlen(temp)+1);
    if(txt==NULL) {
        e_borra_texto();
        return(0);
    }
    strcpy(txt,temp);
    lin[numlin]=txt;

    numlin++;
}

fclose(f_texto);

/* si fichero estaba totalmente vac¡o, crea una l¡nea al menos */
if(!numlin) {
    lin[0]=(char *)malloc(1);
    *lin[0]='\0';
    numlin=1;
}

return(1);
}

/****************************************************************************
    E_GRABA_TEXTO: graba el texto del editor en un fichero.
      Entrada:      'fich' nombre del fichero, NULL si se quiere usar el
            mismo con el que se carg¢.
      Salida:       1 si se grab¢ correctamente, 0 si hubo errores
****************************************************************************/
int e_graba_texto(char *fich)
{
FILE *f_texto;
int i, lng;
char temp[E_MAXLNGLIN+1];

if(fich!=NULL) {
    strcpy(fich_ed,fich);
    strupr(fich_ed);
}

if((f_texto=fopen(fich_ed,"wt"))==NULL) return(0);

for(i=0; i<numlin; i++) {
    /* a¤ade '\n' al final */
    strcpy(temp,lin[i]);
    lng=strlen(temp);
    temp[lng]='\n';
    temp[lng+1]='\0';

    /* graba l¡nea */
    if(fputs(temp,f_texto)==EOF) {
        fclose(f_texto);
        return(0);
    }
}

fclose(f_texto);

/* indica que fichero ya ha sido grabado */
modificado=0;

return(1);
}

/****************************************************************************
    E_NOMBRE_FICHERO: devuelve el nombre del fichero cargado en el
      editor.
      Entrada:      'nf' puntero a buffer donde se dejar  el
            nombre del fichero (el buffer debe tener capacidad
            para MAXPATH caracteres)
****************************************************************************/
void e_nombre_fichero(char *nf)
{

strcpy(nf,fich_ed);

}

/****************************************************************************
    E_INICIA_BUSQUEDA: inicia la b£squeda de una cadena.
      Salida:       1 si encontr¢ cadena, 0 si no
****************************************************************************/
int e_inicia_busqueda(void)
{
int op;

c_abre(&cbusca);
op=c_gestiona(&cbusca);
c_cierra(&cbusca);

/* si sali¢ del cuadro */
if((op==-1) || (op==2)) return(0);

if(busca_cadena()==0) {
    beep();
    imprime_texto_ed();
    return(0);
}

imprime_texto_ed();

return(1);
}

/****************************************************************************
    E_CONTINUA_BUSQUEDA: continua la b£squeda de una cadena.
      Salida:       1 si encontr¢ cadena, 0 si no
****************************************************************************/
int e_continua_busqueda(void)
{
int op, lng, linc, colc;

/* si cadena vac¡a, la pide */
if(*cadena_busca=='\0') {
    c_abre(&cbusca);
    op=c_gestiona(&cbusca);
    c_cierra(&cbusca);

    /* si sali¢ del cuadro */
    if((op==-1) || (op==2)) return(0);
}

/* guarda posici¢n del cursor */
linc=lin_ed;
colc=col_ed;

/* pasa al siguiente car cter; si es el £ltimo, sale */
lng=strlen(lin[lin_ed]);
if(col_ed<(lng-1)) col_ed++;
else if(lin_ed<(numlin-1)) lin_ed++;
else {
    beep();
    return(0);
}

if(busca_cadena()==0) {
    /* restaura posici¢n del cursor */
    lin_ed=linc;
    col_ed=colc;
    beep();
    imprime_texto_ed();
    return(0);
}

imprime_texto_ed();

return(1);
}

/****************************************************************************
    E_MODIFICADO: comprueba si el texto del editor ha sido modificado.
      Salida:       1 si se modific¢ el texto del editor, 0 si no
****************************************************************************/
int e_modificado(void)
{

return(modificado);
}

/****************************************************************************
    E_CAMBIA_MODO: cambia modo de edici¢n.
      Entrada:      'modo' modo de edici¢n, E_EDITA permite editar
            texto, E_SOLOLECT s¢lo permite ver
****************************************************************************/
void e_cambia_modo(int modo)
{

modo_ed=modo;

}

/****************************************************************************
    E_CARGA_TEXTOX: carga texto colocado en mitad de un fichero.
      Entrada:      'fich' nombre del fichero
            'desplz' posici¢n de inicio del texto respecto al
            origen del fichero
      Salida:       1 si se carg¢ correctamente, 0 si hubo errores
****************************************************************************/
int e_carga_textox(char *fich, long desplz)
{
FILE *f_texto;
char c, temp[E_MAXLNGLIN], *txt;
int i;

if((f_texto=fopen(fich,"rb"))==NULL) return(0);

/* borra texto del editor */
e_borra_texto();

/* indica texto nuevo */
modificado=0;

/* copia nombre de fichero */
strcpy(fich_ed,fich);
strupr(fich_ed);

fseek(f_texto,desplz,SEEK_SET);

/* lee l¡neas del fichero */
while(numlin<E_MAXNUMLIN) {
    i=0;
    do {
        c=(char)fgetc(f_texto);
        if(feof(f_texto) || ferror(f_texto)) break;
        temp[i++]=c;
        if(i>(E_MAXLNGLIN-2)) break;
    } while(c!='\n');
    /* debe haber cogido al menos dos caracteres, si no, l¡nea de */
    /* texto no es v lida */
    if(i>=2) {
        temp[i]='\0';
        if(temp[i-1]=='\n') {
            if(temp[i-2]=='\r') {
                temp[i-2]='\n';
                temp[i-1]='\0';
            }
        }
    }
    else temp[0]='\0';
    if(feof(f_texto)) break;
    else if(ferror(f_texto)) {
        fclose(f_texto);
        return(0);
    }

    /* expande tabulaciones y suprime '\n' final */
    ajusta_linea(temp);

    /* inserta l¡nea en el editor */
    txt=(char *)malloc(strlen(temp)+1);
    if(txt==NULL) {
        e_borra_texto();
        return(0);
    }
    strcpy(txt,temp);
    lin[numlin]=txt;

    numlin++;
}

fclose(f_texto);

/* si fichero estaba totalmente vac¡o, crea una l¡nea al menos */
if(!numlin) {
    lin[0]=(char *)malloc(1);
    *lin[0]='\0';
    numlin=1;
}

return(1);
}

/****************************************************************************
    E_VACIA_BLOQUE: libera memoria ocupada por bloque de texto.
****************************************************************************/
void e_vacia_bloque(void)
{
STC_BLOQUE *b0, *b1;

if(bloque==NULL) return;

b0=bloque;

do {
    b1=b0->sgte;
    free(b0->lin);
    free(b0);
    b0=b1;
} while(b0!=NULL);

bloque=NULL;

}

/****************************************************************************
    E_GRABA_BLOQUE: graba el bloque en un fichero.
      Entrada:      'nfich' nombre del fichero
      Salida:       1 si se pudo grabar, 0 si no
****************************************************************************/
int e_graba_bloque(char *nfich)
{
FILE *f_bloque;
STC_BLOQUE *b;
char temp[E_MAXLNGLIN+1];
int i;

if((f_bloque=fopen(nfich,"wt"))==NULL) return(0);

b=bloque;
while(b!=NULL) {
    /* copia l¡nea de bloque a buffer y a¤ade '\n' final */
    strcpy(temp,b->lin);
    i=strlen(temp);
    temp[i]='\n';
    temp[i+1]='\0';

    /* graba l¡nea */
    if(fputs(temp,f_bloque)==EOF) {
        fclose(f_bloque);
        return(0);
    }

    /* siguiente l¡nea de bloque */
    b=b->sgte;
}

fclose(f_bloque);

return(1);
}

/****************************************************************************
    E_INSERTA_BLOQUE: inserta bloque en posici¢n actual del cursor.
      Salida:       1 si se pudo insertar, 0 si no
****************************************************************************/
int e_inserta_bloque(void)
{

if(!inserta_bloque(lin_ed,col_ed)) {
    beep();
    imprime_texto_ed();
    return(0);
}

imprime_texto_ed();

return(1);
}

/****************************************************************************
    E_PON_CURSOR: cambia la posici¢n del cursor del editor.
      Entrada:      'lin', 'col' nueva posici¢n del cursor
****************************************************************************/
void e_pon_cursor(int lin, int col)
{

lin_ed=lin-1;
col_ed=col;
filcur=0;
colcur=0;

}

/****************************************************************************
    E_PALABRA_CURSOR: devuelve la palabra sobre la que se encontraba
      el cursor la £ltima vez que se puls¢ la tecla de ayuda.
      Salida:       puntero a buffer que contiene la palabra
****************************************************************************/
char *e_palabra_cursor(void)
{

return(palabra_cursor);
}

/****************************************************************************
    E_CURSOR_GUARDA: guarda la posici¢n del cursor del editor.
      Entrada:  'cur' puntero a estructura donde guardar datos
****************************************************************************/
void e_cursor_guarda(CURSOR_ED *cur)
{

cur->lin=lin_ed;
cur->col=col_ed;
cur->filcur=filcur;
cur->colcur=colcur;

}

/****************************************************************************
    E_CURSOR_RECUPERA: recupera la posici¢n del cursor del editor.
      Entrada:  'cur' estructura con datos cursor
****************************************************************************/
void e_cursor_recupera(CURSOR_ED *cur)
{

lin_ed=cur->lin;
col_ed=cur->col;
filcur=cur->filcur;
colcur=cur->colcur;

}
