/****************************************************************************
				   MENUG.C

	Biblioteca de funciones para gestionar men£s en los modos
	gr ficos de 16 y 256 colores.

			     (c)1995 JSJ Soft Ltd.

	Las siguientes funciones son p£blicas:
		- mg_crea: crea un men£ de opciones
		- mg_abre: dibuja un men£ en pantalla
		- mg_cierra: borra un men£ de pantalla
		- mg_elimina: elimina un men£ y libera memoria
		- mg_resalta_opcion: resalta la opci¢n actual de un men£
		- mg_accion: env¡a una acci¢n a un men£
		- mg_elige_opcion: elige una opci¢n de un men£

	Las siguientes estructuras est n definidas en MENUG.H:
		STC_OPCIONG: opci¢n de un men£
		STC_MENUG: men£ desplegable
		STC_ACCIONG: acci¢n para un men£
****************************************************************************/

#include <stddef.h>
#include <alloc.h>
#include <bios.h>
#include "graf.h"
#include "ventanag.h"
#include "menug.h"

/*** Variables globales internas ***/
static BYTE MenuG_Clr[]={
	7,	/* fondo */
	0,	/* primer plano */
	15,	/* sombra 1 */
	8,	/* sombra 2 */
	4,	/* tecla activaci¢n */
};

/*** Protipos de funciones internas ***/
static char mayuscula(char c);
static char saca_tecla_opcion(char *(*menu), int *longitud);
static STC_MENUG *crea_menu(char *opciones);
static void imprime_opcion(STC_MENUG *menu, int opcion, int modo);

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
STC_MENUG *crea_menu(char *opciones)
{
STC_MENUG *menu;
int i, longitud;

menu=(STC_MENUG *)malloc(sizeof(STC_MENUG));
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
	IMPRIME_OPCION: imprime una opci¢n en una ventana.
	  Entrada:      'menu' puntero a estructura con los datos del men£
			'opcion' n£mero de opci¢n a imprimir
			'modo' modo de impresi¢n, 0 normal o 1 resaltada
****************************************************************************/
void imprime_opcion(STC_MENUG *menu, int opcion, int modo)
{
int i, fil, col, chralt, x0, y0, x1, y1, lng;
char *txt_opcion;

rg_puntero(RG_OCULTA);

/* selecciona posici¢n de impresi¢n */
if((menu->tipo & MENUG_NFIJO)==MENUG_HORZ) {
	fil=menu->v.fil+menu->opc[opcion].fil;
	col=menu->v.col+menu->opc[opcion].col;
}
else {
	fil=menu->v.fil+menu->opc[opcion].fil+1;
	col=menu->v.col+menu->opc[opcion].col+1;
}

/* calcula recuadro para opci¢n */
chralt=chr_altura();
y0=fil*chralt;
y1=y0+chralt-1;
x0=col*8;

/* si es men£ vertical resalta todo el ancho de la ventana */
if((menu->tipo & MENUG_NFIJO)==MENUG_VERT) x1=x0+((menu->v.ancho-2)*8)-1;
/* si no, s¢lo hasta el primer espacio o fin de texto de opci¢n */
else {
	txt_opcion=menu->opc[opcion].opcion;
	lng=0;
	while(*txt_opcion && (*txt_opcion!=' ') && (*txt_opcion!=CAR_FINOPC) &&
	  (*txt_opcion!=CAR_SEPAR)) {
		lng++;
		txt_opcion++;
	}
	x1=x0+((lng-1)*8)-1;
}

/* si imprime resaltada dibuja s¢lo recuadro y sale */
if(modo!=0) {
	g_linea(x0,y0,x1,y0,menu->v.clr_s2,G_NORM);
	g_linea(x0,y0,x0,y1,menu->v.clr_s2,G_NORM);
	g_linea(x1,y0+1,x1,y1,menu->v.clr_s1,G_NORM);
	g_linea(x0+1,y1,x1,y1,menu->v.clr_s1,G_NORM);
	rg_puntero(RG_MUESTRA);
	return;
}
/* si no, borra posible recuadro que pudiera existir */
else g_rectangulo(x0,y0,x1,y1,menu->v.clr_fondo,G_NORM,0);

txt_opcion=menu->opc[opcion].opcion;

/* si es separador, lo imprime y sale */
if(*txt_opcion==CAR_SEPAR) {
	for(i=0; i<(menu->v.ancho-2); i++, col++) vg_impcar(fil,col,SEPARADOR,
	  MenuG_Clr[MGCLR_FONDO],MenuG_Clr[MGCLR_PPLANO]);
	rg_puntero(RG_MUESTRA);
	return;
}

while(*txt_opcion && (*txt_opcion!=CAR_FINOPC) && (*txt_opcion!=CAR_SEPAR)) {
	/* si es car cter de tecla de activaci¢n lo imprime en otro color */
	if(*txt_opcion==CAR_TECLA) {
		txt_opcion++;
		vg_impcar(fil,col,*txt_opcion,MenuG_Clr[MGCLR_FONDO],
		  MenuG_Clr[MGCLR_TECLA]);
	}
	else vg_impcar(fil,col,*txt_opcion,MenuG_Clr[MGCLR_FONDO],
	  MenuG_Clr[MGCLR_PPLANO]);

	txt_opcion++;
	col++;
}

rg_puntero(RG_MUESTRA);

}

/****************************************************************************
	MG_CREA: crea un men£ de opciones.
	  Entrada:      'tipo' tipo de men£; MENUG_HORZ barra de men£
			horizontal, MENUG_VERT men£ vertical; se puede a¤adir
			mediante OR l¢gico el valor MENUG_FIJO para que el
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
STC_MENUG *mg_crea(BYTE tipo, char *titulo, char *opciones, int fil,
  int col, int separ_opciones)
{
STC_MENUG *menu;
int i, ancho=0, alto, columna=0;

menu=crea_menu(opciones);
if(menu==NULL) return(NULL);

/* calcula dimensiones de la ventana del men£ y la posici¢n de */
/* los textos de cada opci¢n */
if((tipo & MENUG_NFIJO)==MENUG_HORZ) {
	for(i=0; i<menu->num_opciones; i++) {
		ancho+=menu->opc[i].lng_opcion;
		menu->opc[i].fil=0;
		menu->opc[i].col=columna;
		columna+=(menu->opc[i].lng_opcion+separ_opciones);
	}
	ancho+=((separ_opciones*(menu->num_opciones-1))+2);
	alto=3;
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

vg_crea(&menu->v,fil,col,ancho,alto,MenuG_Clr[MGCLR_FONDO],
  MenuG_Clr[MGCLR_PPLANO],MenuG_Clr[MGCLR_S1],MenuG_Clr[MGCLR_S2],titulo);
//vg_borde(&menu->v,VG_BORDE1);
menu->tipo=tipo;
if((tipo & MENUG_NFIJO)==MENUG_HORZ) menu->separ_opc=separ_opciones;
else menu->separ_opc=0;
menu->opcion=0;

return(menu);
}

/****************************************************************************
	MG_ABRE: dibuja un men£ en pantalla.
	  Entrada:      'menu' puntero a estructura de men£
****************************************************************************/
void mg_abre(STC_MENUG *menu)
{
int i;

if((menu->tipo & MENUG_NFIJO)==MENUG_HORZ) {
	rg_puntero(RG_OCULTA);
	for(i=menu->v.col; i<(menu->v.col+menu->v.ancho); i++)
	  vg_impcar(menu->v.fil,i,' ',MenuG_Clr[MGCLR_FONDO],
	  MenuG_Clr[MGCLR_PPLANO]);
	rg_puntero(RG_MUESTRA);
}
else if(!(menu->tipo & MENUG_FIJO)) vg_abre(&menu->v);
else vg_dibuja(&menu->v,1);

for(i=0; i<menu->num_opciones; i++) imprime_opcion(menu,i,0);

}

/****************************************************************************
	MG_CIERRA: borra un men£ de pantalla.
	  Entrada:      'menu' puntero a estructura de men£
****************************************************************************/
void mg_cierra(STC_MENUG *menu)
{

if(!(menu->tipo & MENUG_FIJO)) vg_cierra(&menu->v);

}

/****************************************************************************
	MG_ELIMINA: elimina la definici¢n de un men£ y libera la memoria
	  ocupada.
	  Entrada:      'menu' puntero a estructura de men£
****************************************************************************/
void mg_elimina(STC_MENUG *menu)
{

mg_cierra(menu);
free(menu);

}

/****************************************************************************
	MG_RESALTA_OPCION: resalta la opci¢n actual de un men£.
	  Entrada:      'menu' puntero a estructura con los datos del men£
			'resalta' 0 quita resalte, 1 pone resalte
****************************************************************************/
void mg_resalta_opcion(STC_MENUG *menu, int resalta)
{

if(resalta==0) imprime_opcion(menu,menu->opcion,0);
else imprime_opcion(menu,menu->opcion,1);

}

/****************************************************************************
	MG_ACCION: env¡a una acci¢n a un men£.
	  Entrada:      'menu' puntero a estructura de men£
			'acc' puntero a estructura con la acci¢n a enviar
	  Salida:       respuesta del men£ que puede ser el n£mero de una
			opci¢n o un c¢digo de acci¢n (si es <0)
****************************************************************************/
int mg_accion(STC_MENUG *menu, STC_ACCIONG *acc)
{
int i, minfil, mincol, maxfil, maxcol;
char tecla;

switch(acc->accion) {
	case MENUG_NULA :      	/* acci¢n nula */
		return(MENUG_NULA);
	case MENUG_SALIDA :     /* salir del men£ (ESCAPE) */
		mg_resalta_opcion(menu,0);
		break;
	case MENUG_SELECCIONA : /* selecciona opci¢n */
		mg_resalta_opcion(menu,0);
		return(menu->opcion);
	case MENUG_ARRIBA :     /* cursor arriba */
		if((menu->tipo & MENUG_NFIJO)!=MENUG_HORZ) {
			mg_resalta_opcion(menu,0);
			menu->opcion--;
			if(menu->opcion<0) menu->opcion=menu->num_opciones-1;
			if(*menu->opc[menu->opcion].opcion==CAR_SEPAR)
			  menu->opcion--;
			mg_resalta_opcion(menu,1);
			return(MENUG_NULA);
		}
		break;
	case MENUG_ABAJO :      /* cursor abajo */
		if((menu->tipo & MENUG_NFIJO)!=MENUG_HORZ) {
			mg_resalta_opcion(menu,0);
			menu->opcion++;
			if(menu->opcion>(menu->num_opciones-1)) menu->opcion=0;
			if(*menu->opc[menu->opcion].opcion==CAR_SEPAR)
			  menu->opcion++;
			mg_resalta_opcion(menu,1);
			return(MENUG_NULA);
		}
		break;
	case MENUG_IZQUIERDA :  /* cursor izquierda */
		if((menu->tipo & MENUG_NFIJO)==MENUG_HORZ) {
			mg_resalta_opcion(menu,0);
			menu->opcion--;
			if(menu->opcion<0) menu->opcion=menu->num_opciones-1;
			if(*menu->opc[menu->opcion].opcion==CAR_SEPAR)
			  menu->opcion--;
			mg_resalta_opcion(menu,1);
			return(MENUG_NULA);
		}
		else return(MENUG_SALIDA);
	case MENUG_DERECHA :	/* cursor derecha */
		if((menu->tipo & MENUG_NFIJO)==MENUG_HORZ) {
			mg_resalta_opcion(menu,0);
			menu->opcion++;
			if(menu->opcion>(menu->num_opciones-1)) menu->opcion=0;
			if(*menu->opc[menu->opcion].opcion==CAR_SEPAR)
			  menu->opcion++;
			mg_resalta_opcion(menu,1);
			return(MENUG_NULA);
		}
		else return(MENUG_SALIDA);
	case MENUG_TECLA :      /* tecla de opci¢n */
		if(!acc->tecla) return(MENUG_NULA);
		/* comprueba si es una tecla de activaci¢n v lida */
		for(i=0; i<menu->num_opciones; i++) {
			tecla=mayuscula(acc->tecla);
			if(tecla==menu->opc[i].tecla) {
				mg_resalta_opcion(menu,0);
				menu->opcion=i;
				return(i);
			}
		}
		return(MENUG_NULA);
	case MENUG_RATON :      /* acci¢n del rat¢n */
		/* comprueba si coordenadas de rat¢n est n dentro de */
		/* ventana de men£ */
		if((menu->tipo & MENUG_NFIJO)==MENUG_HORZ) {
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
			if((menu->tipo & MENUG_NFIJO)==MENUG_VERT)
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
					mg_resalta_opcion(menu,0);
					menu->opcion=i;
					mg_resalta_opcion(menu,1);
				}
			}
		return(MENUG_NULA);
		}
		/* si coordenadas del cursor est n fuera, y el bot¢n no */
		/* est  pulsado, cierra men£ */
		else {
			mg_resalta_opcion(menu,0);
			return(MENUG_SALIDA);
		}
}

return(acc->accion);
}

/****************************************************************************
	MG_ELIGE_OPCION: elige una opci¢n de un men£.
	  Entrada:      'menu' puntero a estructura de men£
	  Salida:       n£mero de la opci¢n seleccionada, -1 si se sali¢
			del men£ sin elegir ninguna opci¢n
****************************************************************************/
int mg_elige_opcion(STC_MENUG *menu)
{
static boton1=0;
STC_ACCIONG acc;
STC_RATONG r;
int accion, opcion, tecla, minfil, mincol, maxfil, maxcol;

/* si rat¢n no est  inicializado, lo inicializa */
if(!rg_puntero(RG_MUESTRA)) rg_inicializa();

mg_resalta_opcion(menu,1);

acc.accion=MENUG_NULA;

while(1) {
	do {
		accion=0;
		rg_estado(&r);
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
			if((menu->tipo & MENUG_NFIJO)==MENUG_HORZ) {
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
		acc.accion=MENUG_RATON;
		acc.fil=r.fil;
		acc.col=r.col;
	}
	/* selecci¢n con el rat¢n */
	else if(accion==2) acc.accion=MENUG_SELECCIONA;
	else switch((tecla >> 8) & 0x00ff) {
		case 0x01 :     /* ESCAPE */
			acc.accion=MENUG_SALIDA;
			break;
		case 0x1c :     /* RETURN */
			acc.accion=MENUG_SELECCIONA;
			break;
		case 0x48 :     /* Cursor arriba */
			acc.accion=MENUG_ARRIBA;
			break;
		case 0x50 :     /* Cursor abajo */
			acc.accion=MENUG_ABAJO;
			break;
		case 0x4b :     /* Cursor izquierda */
			acc.accion=MENUG_IZQUIERDA;
			break;
		case 0x4d :     /* Cursor derecha */
			acc.accion=MENUG_DERECHA;
			break;
		default :
			acc.accion=MENUG_TECLA;
			acc.tecla=(char)(tecla & 0x00ff);
			break;
	}

	opcion=mg_accion(menu,&acc);

	if(opcion>=0) return(opcion);
	else if(opcion==MENUG_SALIDA) return(-1);
}

}

/****************************************************************************
	MG_COLOR: cambia paleta de colores para men£s.
	  Entrada:      'clr_fondo' color de fondo
			'clr_pplano'  "   primer plano
			'clr_s1'      "   sombra 1
			'clr_s2'      "   sombra 2
			'clr_tecla'   "   tecla activaci¢n
****************************************************************************/
void mg_color(BYTE clr_fondo, BYTE clr_pplano, BYTE clr_s1, BYTE clr_s2,
  BYTE clr_tecla)
{

MenuG_Clr[MGCLR_FONDO]=clr_fondo;
MenuG_Clr[MGCLR_PPLANO]=clr_pplano;
MenuG_Clr[MGCLR_S1]=clr_s1;
MenuG_Clr[MGCLR_S2]=clr_s2;
MenuG_Clr[MGCLR_TECLA]=clr_tecla;

}
