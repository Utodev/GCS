/**************************************
	Fichero de cabecera de
	las funciones de manejo
	de men£s de MENUG.C
**************************************/

#if !defined (MENUG_H)
#define MENUG_H

/*** Constantes ***/
#define MAX_NUMOPCIONES	20      /* m ximo n§ de opciones de un men£ */
#define MENUG_HORZ      0       /* tipo de men£ horizontal */
#define MENUG_VERT      1       /* tipo de men£ vertical */
#define MENUG_FIJO      0x80    /* activa men£ fijo, que no se cierra */
#define MENUG_NFIJO     0x7f    /* para comprobaciones de tipo de men£ */
#define CAR_FINOPC      ':'     /* fin de texto de opci¢n */
#define CAR_SEPAR       '|'     /* separador de opciones */
#define CAR_TECLA       '^'     /* marca tecla de activaci¢n */
#define SEPARADOR       'Ä'     /* car cter separador */

/* contenido de matriz de colores */
#define MGCLR_FONDO	0
#define MGCLR_PPLANO	1
#define MGCLR_S1	2
#define MGCLR_S2	3
#define MGCLR_TECLA	4

/* c¢digos de acciones para men£s */
#define MENUG_NULA     		-1	/* acci¢n nula */
#define MENUG_SALIDA    	-2      /* salida de un men£ (ESCAPE) */
#define MENUG_IZQUIERDA 	-3      /* movimiento de cursor izquierda */
#define MENUG_DERECHA   	-4      /* movimiento de cursor derecha */
#define MENUG_ARRIBA    	-5      /* movimiento de cursor arriba */
#define MENUG_ABAJO     	-6      /* movimiento de cursor abajo */
#define MENUG_SELECCIONA 	-7      /* selecci¢n de opci¢n */
#define MENUG_TECLA      	-8      /* env¡a tecla */
#define MENUG_RATON      	-9      /* env¡a acci¢n de rat¢n */

/*** Tipos de datos y estructuras ***/
#if !defined (BYTE)
#define BYTE unsigned char
#endif

typedef struct {
	char *opcion;           /* puntero a texto de la opci¢n */
				/* "" si £ltima opci¢n */
	char tecla;             /* c¢digo ASCII de la tecla de activaci¢n */
	int fil, col;         	/* posici¢n de la opci¢n en la ventana */
	int lng_opcion;         /* longitud del texto de la opci¢n */
} STC_OPCIONG;

typedef struct {
	STC_VENTANAG v;        	/* ventana del men£ */
	STC_OPCIONG opc[MAX_NUMOPCIONES];    	/* opciones del men£ */
	BYTE tipo;              /* tipo de men£: MENUG_HORZ o MENUG_VERT */
	int num_opciones;       /* n£mero de opciones */
	int separ_opc;        	/* separaci¢n entre opciones */
				/* s¢lo MENU_HORZ */
	int opcion;             /* £ltima opci¢n seleccionada */
} STC_MENUG;

typedef struct {
	int accion;             /* c¢digo de acci¢n */
	char tecla;             /* tecla pulsada (MENUG_TECLA) */
	int fil, col;         	/* posici¢n del rat¢n (MENUG_RATON) */
} STC_ACCIONG;

/*** Prototipos ***/
STC_MENUG *mg_crea(BYTE tipo, char *titulo, char *opciones, int fil,
  int col, int separ_opciones);
void mg_abre(STC_MENUG *menu);
void mg_cierra(STC_MENUG *menu);
void mg_elimina(STC_MENUG *menu);
void mg_resalta_opcion(STC_MENUG *menu, int resalta);
int mg_accion(STC_MENUG *menu, STC_ACCIONG *acc);
int mg_elige_opcion(STC_MENUG *menu);
void mg_color(BYTE clr_fondo, BYTE clr_pplano, BYTE clr_s1, BYTE clr_s2,
  BYTE clr_tecla);

#endif  /* MENUG_H */

