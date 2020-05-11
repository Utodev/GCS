/****************************************************************************
				   MENU.C

	Biblioteca de funciones para gestionar men£s.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- m_crea: crea un men£ de opciones
		- m_abre: dibuja un men£ en pantalla
		- m_cierra: borra un men£ de pantalla
		- m_elimina: elimina un men£ y libera memoria
		- m_cambia_color: actualiza los colores de un men£
		- m_resalta_opcion: resalta la opci¢n actual de un men£
		- m_accion: env¡a una acci¢n a un men£
		- m_elige_opcion: elige una opci¢n de un men£
		- m_color: cambia paleta de colores

	Las siguientes estructuras est n definidas en MENU.H:
		STC_OPCION: opci¢n de un men£
		STC_MENU: men£ desplegable
		STC_ACCION: acci¢n para un men£
****************************************************************************/

#include <stddef.h>
#include <alloc.h>
#include <bios.h>
#include "ventana.h"
#include "raton.h"
#include "menu.h"

/*** Variables globales internas ***/
static BYTE Menu_Clr[]={
	0x70,	/* principal */
	0x7f,	/* sombra 1 */
	0x70,	/* sombra 2 */
	0x74,	/* tecla activaci¢n */
	0x07,	/* opci¢n seleccionada */
};

/*** Protipos de funciones internas ***/
static char mayuscula(char c);
static void coge_pos_cursor(int *fil, int *col);
static void pon_cursor(int fil, int col);
static char saca_tecla_opcion(char *(*menu), int *longitud);
static STC_MENU *crea_menu(char *opciones);
static void imprime_opcion(STC_MENU *menu, int opcion, int modo);

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
	SACA_TECLA_OPCION: extrae informaci¢n de la tecla de activaci¢n de
	  una opci¢n desde una cadena de definici¢n de men£.
	  Entrada:      'menu' puntero a cadena de definici¢n de las opciones
			del men£ con el formato:
			  "Opci¢n1^1:Opci¢n1^2:Opci¢n1^3|Opci¢n1^4"
			el car cter '^' indica la tecla de activaci¢n y
			el car cter '|' coloca una l¡nea de separaci¢n
			'longitud' puntero a variable donde se dejar  la
			longitud del texto de la opci¢n
	  Salida:       devuelve el c¢digo ASCII de la tecla de activaci¢n o
			0 si no la encontr¢; el puntero 'menu' queda
			actualizado para apuntar a la siguiente opci¢n y la
			variable 'columna' contiene la £ltima columna de la
			opci¢n actual
****************************************************************************/
char saca_tecla_opcion(char *(*menu), int *longitud)
{
char tecla=0;

*longitud=0;

do {
	switch(*(*menu)) {
		case CAR_SEPAR :
			(*menu)++;
			(*longitud)++;
			return(0);
		case CAR_TECLA :
			(*menu)++;
			tecla=mayuscula(*(*menu));
			break;
	}

	/* siguiente car cter */
	(*menu)++;
	(*longitud)++;
} while(*(*menu) && (*(*menu)!=CAR_FINOPC) && (*(*menu)!=CAR_SEPAR));

/* salta delimitador */
if(*(*menu)==CAR_FINOPC) (*menu)++;

return(tecla);
}

/****************************************************************************
	CREA_MENU: crea un men£.
	  Entrada:      'menu' cadena con la lista de opciones del men£
			de la forma:
			  "Opci¢n1^1:Opci¢n1^2:Opci¢n1^3|Opci¢n1^4"
			el car cter '^' indica la tecla de activaci¢n y
			el car cter '|' coloca una l¡nea de separaci¢n
	  Salida:       puntero a estructura con informaci¢n del men£ a usar
			por las otras funciones de manejo de men£s o NULL si
			hubo alg£n error
	  NOTA:         el m ximo n£mero de opciones de un men£ viene dado
			por la constante MAX_NUMOPCIONES
****************************************************************************/
STC_MENU *crea_menu(char *opciones)
{
STC_MENU *menu;
int i, longitud;

menu=(STC_MENU *)malloc(sizeof(STC_MENU));
if(menu==NULL) return(NULL);

i=0;
while((*opciones) && (i<MAX_NUMOPCIONES)) {
	/* puntero a texto de opci¢n */
	menu->opc[i].opcion=opciones;
	menu->opc[i].tecla=saca_tecla_opcion(&opciones,&longitud);
	menu->opc[i].lng_opcion=longitud;

	/* siguiente opci¢n */
	i++;
}

menu->num_opciones=i;

return(menu);
}

/****************************************************************************
	IMPRIME_OPCION: imprime una opci¢n de un men£.
	  Entrada:      'menu' puntero a estructura con los datos del men£
			'opcion' n£mero de opci¢n a imprimir
			'modo' modo de impresi¢n, 0 normal o 1 resaltada
****************************************************************************/
void imprime_opcion(STC_MENU *menu, int opcion, int modo)
{
int i, fil, col;
BYTE clr;
char *txt_opcion;

r_puntero(R_OCULTA);

/* selecciona posici¢n de impresi¢n */
if((menu->tipo & MENU_NFIJO)==MENU_HORZ) {
	fil=menu->v.fil+menu->opc[opcion].fil;
	col=menu->v.col+menu->opc[opcion].col;
}
else {
	fil=menu->v.fil+menu->opc[opcion].fil+1;
	col=menu->v.col+menu->opc[opcion].col+1;
}

/* selecciona color correspondiente */
if(modo==0) clr=Menu_Clr[MCLR_PRINCIPAL];
else clr=Menu_Clr[MCLR_SELECC];

txt_opcion=menu->opc[opcion].opcion;

/* si es separador, lo imprime y sale */
if(*txt_opcion==CAR_SEPAR) {
	for(i=0; i<menu->v.ancho-2; i++) v_impcar(fil,col+i,SEPARADOR,clr);
	r_puntero(R_MUESTRA);
	return;
}

while(*txt_opcion && (*txt_opcion!=CAR_FINOPC) && (*txt_opcion!=CAR_SEPAR)) {
	/* si es car cter de tecla de activaci¢n lo imprime en otro color */
	if(*txt_opcion==CAR_TECLA) {
		txt_opcion++;
		/* lo imprime en otro color s¢lo si no resaltada */
		if(modo==0) v_impcar(fil,col,*txt_opcion,Menu_Clr[MCLR_TECLA]);
		else v_impcar(fil,col,*txt_opcion,clr);
	}
	else {
		/* si es men£ horizontal, no resalta los espacios */
		if(((menu->tipo & MENU_NFIJO)==MENU_HORZ) &&
		  (*txt_opcion==' ') && (modo!=0)) v_impcar(fil,col,
		  *txt_opcion,Menu_Clr[MCLR_PRINCIPAL]);
		else v_impcar(fil,col,*txt_opcion,clr);
	}

	txt_opcion++;
	col++;
}

/* si es men£ vertical resalta todo el ancho de la ventana */
if((menu->tipo & MENU_NFIJO)==MENU_VERT) {
	for(i=0; i<(menu->v.ancho-menu->opc[opcion].lng_opcion-2); i++)
	  v_impcar(fil,col++,' ',clr);
}

r_puntero(R_MUESTRA);

}

/****************************************************************************
	M_CREA: crea un men£ de opciones.
	  Entrada:      'tipo' tipo de men£; MENU_HORZ barra de men£
			horizontal, MENU_VERT men£ vertical; se puede a¤adir
			mediante OR l¢gico el valor MENU_FIJO para que el
			men£ no se pueda cerrar
			'titulo' texto de encabezamiento del men£ (NULL si
			ninguno)
			'opciones' cadena con las opciones del men£
			de la forma:
			  "Opci¢n1^1:Opci¢n1^2:Opci¢n1^3|Opci¢n1^4"
			el car cter '^' indica la tecla de activaci¢n y
			el car cter '|' coloca una l¡nea de separaci¢n, la
			£ltima cadena ser  nula ("")
			'fil', 'col' posici¢n donde se colocar  el men£
			'separ_opciones' espacio de separaci¢n entre las
			opciones de men£ de barra
	  Salida:       puntero a estructura con informaci¢n del men£ a usar
			por las otras funciones de manejo de men£s o NULL si
			hubo alg£n error
	  NOTA:         el m ximo n£mero de opciones de un men£ viene dado
			por la constante MAX_NUMOPCIONES
****************************************************************************/
STC_MENU *m_crea(BYTE tipo, char *titulo, char *opciones, int fil,
  int col, int separ_opciones)
{
STC_MENU *menu;
int i, ancho=0, alto, columna=0;

menu=crea_menu(opciones);
if(menu==NULL) return(NULL);

/* calcula dimensiones de la ventana del men£ y la posici¢n de */
/* los textos de cada opci¢n */
if((tipo & MENU_NFIJO)==MENU_HORZ) {
	for(i=0; i<menu->num_opciones; i++) {
		ancho+=menu->opc[i].lng_opcion;
		menu->opc[i].fil=0;
		menu->opc[i].col=columna;
		columna+=(menu->opc[i].lng_opcion+separ_opciones);
	}
	ancho+=(separ_opciones*(menu->num_opciones-1));
	alto=1;
}
else {
	for(i=0; i<menu->num_opciones; i++) {
		if(menu->opc[i].lng_opcion>ancho) {
			ancho=menu->opc[i].lng_opcion;
		}
		menu->opc[i].col=0;
		menu->opc[i].fil=i;
	}
	ancho+=2;
	alto=menu->num_opciones+2;
}

v_crea(&menu->v,fil,col,ancho,alto,Menu_Clr[MCLR_PRINCIPAL],
  Menu_Clr[MCLR_S1],Menu_Clr[MCLR_S2],titulo,1);
menu->tipo=tipo;
if((tipo & MENU_NFIJO)==MENU_HORZ) menu->separ_opc=separ_opciones;
else menu->separ_opc=0;
menu->opcion=0;

return(menu);
}

/****************************************************************************
	M_ABRE: dibuja un men£ en pantalla.
	  Entrada:      'menu' puntero a estructura de men£
****************************************************************************/
void m_abre(STC_MENU *menu)
{
int i;

if((menu->tipo & MENU_NFIJO)==MENU_HORZ) {
	r_puntero(R_OCULTA);
	for(i=menu->v.col; i<(menu->v.col+menu->v.ancho); i++)
	  v_impcar(menu->v.fil,i,' ',Menu_Clr[MCLR_PRINCIPAL]);
	r_puntero(R_MUESTRA);
}
else v_abre(&menu->v);

for(i=0; i<menu->num_opciones; i++) imprime_opcion(menu,i,0);

}

/****************************************************************************
	M_CIERRA: borra un men£ de pantalla.
	  Entrada:      'menu' puntero a estructura de men£
****************************************************************************/
void m_cierra(STC_MENU *menu)
{

if((menu->tipo & MENU_NFIJO)!=MENU_HORZ) v_cierra(&menu->v);

}

/****************************************************************************
	M_ELIMINA: elimina la definici¢n de un men£ y libera la memoria
	  ocupada.
	  Entrada:      'menu' puntero a estructura de men£
****************************************************************************/
void m_elimina(STC_MENU *menu)
{

m_cierra(menu);
free(menu);

}

/****************************************************************************
	M_CAMBIA_COLOR: actualiza los colores de un men£ de opciones.
	  Entrada:      'menu' puntero a estructura de men£
****************************************************************************/
void m_cambia_color(STC_MENU *menu)
{

menu->v.clr_princ=Menu_Clr[MCLR_PRINCIPAL];
menu->v.clr_s1=Menu_Clr[MCLR_S1];
menu->v.clr_s2=Menu_Clr[MCLR_S2];
menu->v.clr_texto=Menu_Clr[MCLR_PRINCIPAL];

}

/****************************************************************************
	M_RESALTA_OPCION: resalta la opci¢n actual de un men£.
	  Entrada:      'menu' puntero a estructura con los datos del men£
			'resalta' 0 quita resalte, 1 pone resalte
****************************************************************************/
void m_resalta_opcion(STC_MENU *menu, int resalta)
{

if(resalta==0) imprime_opcion(menu,menu->opcion,0);
else imprime_opcion(menu,menu->opcion,1);

}

/****************************************************************************
	M_ACCION: env¡a una acci¢n a un men£.
	  Entrada:      'menu' puntero a estructura de men£
			'acc' puntero a estructura con la acci¢n a enviar
	  Salida:       respuesta del men£ que puede ser el n£mero de una
			opci¢n o un c¢digo de acci¢n (si es <0)
****************************************************************************/
int m_accion(STC_MENU *menu, STC_ACCION *acc)
{
int i, minfil, mincol, maxfil, maxcol;
char tecla;

switch(acc->accion) {
	case MENU_NULA :        /* acci¢n nula */
		return(MENU_NULA);
	case MENU_SALIDA :      /* salir del men£ (ESCAPE) */
		m_resalta_opcion(menu,0);
		if(!(menu->tipo & MENU_FIJO)) m_cierra(menu);
		break;
	case MENU_SELECCIONA :  /* selecciona opci¢n */
		m_resalta_opcion(menu,0);
		return(menu->opcion);
	case MENU_ARRIBA :      /* cursor arriba */
		if((menu->tipo & MENU_NFIJO)!=MENU_HORZ) {
			m_resalta_opcion(menu,0);
			menu->opcion--;
			if(menu->opcion<0) menu->opcion=menu->num_opciones-1;
			if(*menu->opc[menu->opcion].opcion==CAR_SEPAR)
			  menu->opcion--;
			m_resalta_opcion(menu,1);
			return(MENU_NULA);
		}
		break;
	case MENU_ABAJO :       /* cursor abajo */
		if((menu->tipo & MENU_NFIJO)!=MENU_HORZ) {
			m_resalta_opcion(menu,0);
			menu->opcion++;
			if(menu->opcion>(menu->num_opciones-1)) menu->opcion=0;
			if(*menu->opc[menu->opcion].opcion==CAR_SEPAR)
			  menu->opcion++;
			m_resalta_opcion(menu,1);
			return(MENU_NULA);
		}
		break;
	case MENU_IZQUIERDA :   /* cursor izquierda */
		if((menu->tipo & MENU_NFIJO)==MENU_HORZ) {
			m_resalta_opcion(menu,0);
			menu->opcion--;
			if(menu->opcion<0) menu->opcion=menu->num_opciones-1;
			if(*menu->opc[menu->opcion].opcion==CAR_SEPAR)
			  menu->opcion--;
			m_resalta_opcion(menu,1);
			return(MENU_NULA);
		}
		else {
			if(!(menu->tipo & MENU_FIJO)) m_cierra(menu);
			return(MENU_SALIDA);
		}
	case MENU_DERECHA :     /* cursor derecha */
		if((menu->tipo & MENU_NFIJO)==MENU_HORZ) {
			m_resalta_opcion(menu,0);
			menu->opcion++;
			if(menu->opcion>(menu->num_opciones-1)) menu->opcion=0;
			if(*menu->opc[menu->opcion].opcion==CAR_SEPAR)
			  menu->opcion++;
			m_resalta_opcion(menu,1);
			return(MENU_NULA);
		}
		else {
			if(!(menu->tipo & MENU_FIJO)) m_cierra(menu);
			return(MENU_SALIDA);
		}
	case MENU_TECLA :       /* tecla de opci¢n */
		if(!acc->tecla) return(MENU_NULA);
		/* comprueba si es una tecla de activaci¢n v lida */
		for(i=0; i<menu->num_opciones; i++) {
			tecla=mayuscula(acc->tecla);
			if(tecla==menu->opc[i].tecla) {
				m_resalta_opcion(menu,0);
				menu->opcion=i;
				return(i);
			}
		}
		return(MENU_NULA);
	case MENU_RATON :       /* acci¢n del rat¢n */
		/* comprueba si coordenadas de rat¢n est n dentro de */
		/* ventana de men£ */
		if((menu->tipo & MENU_NFIJO)==MENU_HORZ) {
			minfil=menu->v.fil;
			maxfil=minfil;
		}
		else {
			minfil=menu->v.fil+1;
			maxfil=minfil+menu->v.alto-3;
		}
		mincol=menu->v.col;
		maxcol=mincol+menu->v.ancho-1;
		if((acc->fil>=minfil) && (acc->fil<=maxfil) &&
		  (acc->col>=mincol) && (acc->col<=maxcol)) {
			/* comprueba a que opci¢n corresponden las */
			/* coordenadas del puntero del rat¢n */
			if((menu->tipo & MENU_NFIJO)==MENU_VERT)
			  i=acc->fil-minfil;
			else {
				for(i=0; i<menu->num_opciones; i++) {
					mincol=menu->opc[i].col;
					maxcol=menu->opc[i].col+
					  menu->opc[i].lng_opcion+
					  menu->separ_opc-1;
					if((acc->col>=mincol) &&
					  (acc->col<=maxcol)) break;
				}
			}
			/* selecciona opci¢n si no es separador */
			if(*menu->opc[i].opcion!=CAR_SEPAR) {
				if(menu->opcion!=i) {
					m_resalta_opcion(menu,0);
					menu->opcion=i;
					m_resalta_opcion(menu,1);
				}
			}
		return(MENU_NULA);
		}
		/* si coordenadas del cursor est n fuera, y el bot¢n no */
		/* est  pulsado, cierra men£ */
		else {
			m_resalta_opcion(menu,0);
			if(!(menu->tipo & MENU_FIJO)) m_cierra(menu);
			return(MENU_SALIDA);
		}
}

return(acc->accion);
}

/****************************************************************************
	M_ELIGE_OPCION: elige una opci¢n de un men£.
	  Entrada:      'menu' puntero a estructura de men£
	  Salida:       n£mero de la opci¢n seleccionada, -1 si se sali¢
			del men£ sin elegir ninguna opci¢n
****************************************************************************/
int m_elige_opcion(STC_MENU *menu)
{
static int boton1=0;
STC_ACCION acc;
STC_RATON r;
int accion, opcion, tecla, minfil, mincol, maxfil, maxcol, filcur, colcur;

/* si rat¢n no est  inicializado, lo inicializa */
if(!r_puntero(R_MUESTRA)) r_inicializa();

/* oculta cursor */
coge_pos_cursor(&filcur,&colcur);
pon_cursor(25,0);

m_resalta_opcion(menu,1);

acc.accion=MENU_NULA;

while(1) {
	do {
		accion=0;
		r_estado(&r);
		/* si pulsado el bot¢n 1 del rat¢n, indica acci¢n del rat¢n */
		if(r.boton1) {
			accion=1;
			boton1=1;
		}
		/* si anteriormente estaba pulsado el bot¢n 1 y ahora est  */
		/* suelto, y dentro del men£, indica selecci¢n con rat¢n */
		else if(boton1) {
			/* comprueba si puntero de rat¢n est  dentro de */
			/* ventana de men£ */
			if((menu->tipo & MENU_NFIJO)==MENU_HORZ) {
				minfil=menu->v.fil;
				maxfil=minfil;
			}
			else  {
				minfil=menu->v.fil+1;
				maxfil=minfil+menu->v.alto-3;
			}
			mincol=menu->v.col;
			maxcol=mincol+menu->v.ancho-1;
			if((r.fil>=minfil) && (r.fil<=maxfil) &&
			  (r.col>=mincol) && (r.col<=maxcol)) accion=2;
			boton1=0;
		}
		else {
			/* si hay tecla esperando, indica acci¢n de teclado */
			tecla=bioskey(1);
			if(tecla) {
				bioskey(0);
				accion=3;
			}
		}
	} while(!accion);

	/* si es acci¢n de rat¢n */
	if(accion==1) {
		acc.accion=MENU_RATON;
		acc.fil=r.fil;
		acc.col=r.col;
	}
	/* selecci¢n con el rat¢n */
	else if(accion==2) acc.accion=MENU_SELECCIONA;
	else switch((tecla >> 8) & 0x00ff) {
		case 0x01 :     /* ESCAPE */
			acc.accion=MENU_SALIDA;
			break;
		case 0x1c :     /* RETURN */
			acc.accion=MENU_SELECCIONA;
			break;
		case 0x48 :     /* Cursor arriba */
			acc.accion=MENU_ARRIBA;
			break;
		case 0x50 :     /* Cursor abajo */
			acc.accion=MENU_ABAJO;
			break;
		case 0x4b :     /* Cursor izquierda */
			acc.accion=MENU_IZQUIERDA;
			break;
		case 0x4d :     /* Cursor derecha */
			acc.accion=MENU_DERECHA;
			break;
		default :
			acc.accion=MENU_TECLA;
			acc.tecla=(char)(tecla & 0x00ff);
			break;
	}

	opcion=m_accion(menu,&acc);

	if(opcion>=0) {
		pon_cursor(filcur,colcur);
		return(opcion);
	}
	else if(opcion==MENU_SALIDA) {
		pon_cursor(filcur,colcur);
		return(-1);
	}
}

}

/****************************************************************************
	M_COLOR: cambia paleta de colores para men£s.
	  Entrada:      'clr_princ' color principal
			'clr_s1'      "    sombra 1
			'clr_s2'      "    sombra 2
			'clr_tecla'   "    tecla activaci¢n
			'clr_sel'     "    opci¢n seleccionada
****************************************************************************/
void m_color(BYTE clr_princ, BYTE clr_s1, BYTE clr_s2, BYTE clr_tecla,
  BYTE clr_sel)
{

Menu_Clr[MCLR_PRINCIPAL]=clr_princ;
Menu_Clr[MCLR_S1]=clr_s1;
Menu_Clr[MCLR_S2]=clr_s2;
Menu_Clr[MCLR_TECLA]=clr_tecla;
Menu_Clr[MCLR_SELECC]=clr_sel;

}
